#version 460 core

layout(location = 0) in vec3 aCrntPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;
layout(location = 3) in vec3 aTangent;

layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 0) uniform CameraBuffer
{
    mat4 proj;
    mat4 view;
    vec3 camPos;
}
cameraData;

layout(set = 1, binding = 0) uniform sampler2D albedo0;
layout(set = 1, binding = 1) uniform sampler2D ao0;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughness0;
layout(set = 1, binding = 3) uniform sampler2D normal0;
layout(set = 1, binding = 4) uniform sampler2D displacement0;

#define MAX_DIR_LIGHTS 8
layout(set = 2, binding = 0) uniform DirLightsBlock
{
    vec4  dirLightDirection[MAX_DIR_LIGHTS];
    vec4  dirLightColor[MAX_DIR_LIGHTS];
    mat4  dirShadowMatrix[MAX_DIR_LIGHTS];
    ivec4 dirLayerIndex[MAX_DIR_LIGHTS];
    int   numDirLights;
};
layout(set = 2, binding = 1) uniform sampler2DArray dirShadowMaps;
// usage: dirLightDirection[i].xyz, dirLayerIndex[i].x

#define PI 3.14159265359

mat3 getTBN()
{
    vec3 N = normalize(aNormal);
    vec3 T = normalize(aTangent - N * dot(aTangent, N));
    vec3 B = cross(T, N);
    return mat3(T, B, N);
}

vec3 getNormal(vec2 UVs)
{
    mat3 TBN    = getTBN();
    vec3 mapped = texture(normal0, UVs).rgb * 2.0f - 1.0f;
    return normalize(TBN * mapped);
}

vec2 getUVs()
{
    vec2  UVs                  = aTex;
    float currentDepthMapValue = texture(displacement0, UVs).r;

    if (currentDepthMapValue == 0.0f)
        return UVs;

    mat3 TBN            = transpose(getTBN());
    vec3 viewDirTangent = normalize(TBN * (cameraData.camPos - aCrntPos));

    float       heightScale       = 0.07f;
    const float minLayers         = 8.0f;
    const float maxLayers         = 64.0f;
    float       numLayers         = mix(maxLayers, minLayers, abs(dot(vec3(0.0f, 0.0f, 1.0f), viewDirTangent)));
    float       layerDepth        = 1.0f / numLayers;
    float       currentLayerDepth = 0.0f;

    vec2 S        = viewDirTangent.xy / viewDirTangent.z * heightScale;
    vec2 deltaUVs = S / numLayers;

    while (currentLayerDepth < currentDepthMapValue)
    {
        UVs += deltaUVs;
        currentDepthMapValue = texture(displacement0, UVs).r;
        currentLayerDepth += layerDepth;
    }

    vec2  prevTexCoords = UVs - deltaUVs;
    float afterDepth    = currentDepthMapValue - currentLayerDepth;
    float beforeDepth   = texture(displacement0, prevTexCoords).r - currentLayerDepth + layerDepth;
    float weight        = afterDepth / (afterDepth - beforeDepth);
    UVs                 = prevTexCoords * weight + UVs * (1.0f - weight);

    if (UVs.x > 1.0 || UVs.y > 1.0 || UVs.x < 0.0 || UVs.y < 0.0)
        discard;

    return UVs;
}

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom       = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1  = GeometrySchlickGGX(NdotV, k);
    float ggx2  = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.001 - cosTheta, 5.0);
}

vec3 Fr(vec3 crntAlbedoColor, float crntRoughness, float crntMetalic, vec3 F0, vec3 Wo, vec3 Wi, vec3 N, vec3 HalfWay)
{
    vec3  Flambert = crntAlbedoColor / PI;
    float cosTheta = max(dot(HalfWay, Wo), 0.0);
    vec3  Fresnel  = fresnelSchlick(cosTheta, F0);
    vec3  kd       = (vec3(1.0f) - Fresnel) * (1.0 - crntMetalic);

    float alpha             = max(crntRoughness * crntRoughness, 0.02);
    float k                 = (crntRoughness + 1.0) * (crntRoughness + 1.0) / 8.0;
    vec3  cookTorranceNum   = DistributionGGX(N, HalfWay, alpha) * GeometrySmith(N, Wo, Wi, k) * Fresnel;
    float cookTorranceDenum = 4 * max(dot(Wo, N), 0.02) * max(dot(Wi, N), 0.02);
    vec3  cookTorrance      = cookTorranceNum / cookTorranceDenum;

    return kd * Flambert + cookTorrance;
}

vec3 direcLight(int i, vec3 lightDir, vec3 N)
{
    vec4  fragPosLight = dirShadowMatrix[i] * vec4(aCrntPos, 1.0);
    float shadow       = 0.0f;
    vec3  lightCoords  = fragPosLight.xyz / fragPosLight.w;
    if (lightCoords.z <= 1.0f)
    {
        lightCoords        = (lightCoords + 1.0f) / 2.0f;  // [-1, 1] range to [0, 1]
        float currentDepth = lightCoords.z;
        float bias         = max(0.0025f * (1.0f - dot(N, normalize(lightDir))), 0.0005f);

        int  sampleRadius = 2;
        vec2 pixelSize    = 1.0 / textureSize(dirShadowMaps, 0).xy;
        for (int y = -sampleRadius; y <= sampleRadius; y++)
        {
            for (int x = -sampleRadius; x <= sampleRadius; x++)
            {
                float closestDepth = texture(dirShadowMaps, vec3(lightCoords.xy + vec2(x, y) * pixelSize, float(dirLayerIndex[i].x))).r;
                if (currentDepth > closestDepth + bias)
                    shadow += 1.0f;
            }
        }
        shadow /= pow((sampleRadius * 2 + 1), 2);
    }

    return (1.0f - shadow) * dirLightColor[i].xyz;
    // return vec4(vec3(shadow), 1.0f);  // for debugging shadows (shows shadow regions in white)
}

void main()
{
    vec3        sum           = vec3(0.0f);  // PBR sum
    const vec2  UVs           = getUVs();    // world space UVs
    const vec4  crntAlbedo    = texture(albedo0, UVs);
    const float crntMetalic   = texture(metallicRoughness0, UVs).b;
    const float crntRoughness = texture(metallicRoughness0, UVs).g;

    const vec3 N        = getNormal(UVs);                           // normal
    const vec3 Wo       = normalize(cameraData.camPos - aCrntPos);  // view dir
    vec3       F0       = vec3(0.04);
    F0                  = mix(F0, crntAlbedo.xyz, crntMetalic);
    const float ao      = texture(ao0, UVs).r;  // ambient occlusion
    vec3        ambient = vec3(ao);             // for now no IBL

    for (int i = 0; i < numDirLights; i++)
    {
        const vec3 Wi      = normalize(-dirLightDirection[i].xyz);  // light dir
        const vec3 HalfWay = normalize(Wi + Wo);
        sum += Fr(crntAlbedo.xyz, crntRoughness, crntMetalic, F0, Wo, Wi, N, HalfWay) * direcLight(i, Wi, N) * max(dot(N, Wi), 0.0);
    }

    FragColor = vec4(sum, 1.0f);
}