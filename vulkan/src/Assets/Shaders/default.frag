#version 460 core

layout(location = 0) in vec3 aCrntPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in float aTangentSign;

layout(location = 0) out vec4 FragColor;

layout(set = 1, binding = 0) uniform sampler2D albedo0;
layout(set = 1, binding = 1) uniform sampler2D ao0;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughness0;
layout(set = 1, binding = 3) uniform sampler2D normal0;
layout(set = 1, binding = 4) uniform sampler2D displacement0;

layout(set = 0, binding = 0) uniform CameraBuffer
{
    mat4 proj;
    mat4 view;
    vec3 camPos;
}
cameraData;

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

void main()
{
    vec3 N      = normalize(aNormal);
    vec3 T      = normalize(aTangent - N * dot(aTangent, N));
    vec3 B      = cross(T, N);
    mat3 TBN    = mat3(T, B, N);
    vec3 mapped = texture(normal0, aTex).rgb * 2.0f - 1.0f;

    const vec3 Norm = normalize(TBN * mapped);
    // FragColor      = vec4(aTangent * 0.5 + 0.5, 1.0);
    FragColor = vec4(Norm, 1.0f);

    // FragColor = vec4(texture(albedo0, UVs));
}