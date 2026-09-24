#version 460 core

layout(location = 0) in vec3 aCrntPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;
layout(location = 3) in vec3 aTangent;

layout(location = 0) out vec4 FragColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

// mat3 getTBN()
// {
//     vec3 N = normalize(aNormal);
//     vec3 T = normalize(Tangent - N * dot(Tangent, N));
//     vec3 B = cross(T, N);
//     return mat3(T, B, N);
// }

// vec3 getNormal(vec2 UVs)
// {
//     if (!useNormal)
//         return normalize(aNormal);

//     mat3 TBN    = getTBN();
//     vec3 mapped = texture(normal0, UVs).rgb * 2.0f - 1.0f;
//     return normalize(TBN * mapped);
// }

// vec2 getUVs()
// {
//     if (!useDisplacement)
//         return texCoord;

//     mat3 TBN            = transpose(getTBN());
//     vec3 viewDirTangent = normalize(TBN * (camPos - crntPos));

//     float       heightScale       = 0.07f;
//     const float minLayers         = 8.0f;
//     const float maxLayers         = 64.0f;
//     float       numLayers         = mix(maxLayers, minLayers, abs(dot(vec3(0.0f, 0.0f, 1.0f), viewDirTangent)));
//     float       layerDepth        = 1.0f / numLayers;
//     float       currentLayerDepth = 0.0f;

//     vec2 S        = viewDirTangent.xy / viewDirTangent.z * heightScale;
//     vec2 deltaUVs = S / numLayers;

//     vec2  UVs                  = texCoord;
//     float currentDepthMapValue = texture(displacement0, UVs).r;

//     while (currentLayerDepth < currentDepthMapValue)
//     {
//         UVs += deltaUVs;
//         currentDepthMapValue = texture(displacement0, UVs).r;
//         currentLayerDepth += layerDepth;
//     }

//     vec2  prevTexCoords = UVs - deltaUVs;
//     float afterDepth    = currentDepthMapValue - currentLayerDepth;
//     float beforeDepth   = texture(displacement0, prevTexCoords).r - currentLayerDepth + layerDepth;
//     float weight        = afterDepth / (afterDepth - beforeDepth);
//     UVs                 = prevTexCoords * weight + UVs * (1.0f - weight);

//     if (UVs.x > 1.0 || UVs.y > 1.0 || UVs.x < 0.0 || UVs.y < 0.0)
//         discard;

//     return UVs;
// }

void main()
{
    FragColor = vec4(texture(texSampler, aTex));
}