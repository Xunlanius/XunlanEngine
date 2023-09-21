#ifndef LIGHTING
#define LIGHTING

#include "Data/CBuffer.hlsli"
#include "Data/Sampler.hlsli"
#include "Transform.hlsli"

#define BIAS 0.01

float PercentageCloserFilteringSoftShadow(Texture2D shadowMap, SamplerState sample, float3 texSpacePos)
{
    float width;
    float height;
    shadowMap.GetDimensions(width, height);
    
    const float2 texelSize = 1.0f / float2(width, height);
    
    float shadow = 0.0;
    
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            const float2 uv = texSpacePos.xy + float2(x, y) * texelSize;
            const float depth = shadowMap.Sample(sample, uv).r;

            shadow += (texSpacePos.z - BIAS > depth) ? 0.0f : 1.0f;
        }
    }
    
    return shadow /= 9.0;
}

float HardShadow(Texture2D shadowMap, SamplerState sample, float3 texSpacePos)
{
    const float depth = shadowMap.Sample(sample, texSpacePos.xy).r;
    
    return (texSpacePos.z - BIAS > depth) ? 0.0f : 1.0f;
}

float ComputeShadow(Texture2D shadowMap, SamplerState sample, float4 lightViewPos)
{
    if (lightViewPos.x < -1.0f || lightViewPos.x > 1.0f ||
        lightViewPos.y < -1.0f || lightViewPos.y > 1.0f ||
        lightViewPos.z < 0.0f || lightViewPos.z > 1.0f)
    {
        return 0.0f;
    }
    
    float3 texSpacePos = float3(lightViewPos.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f), lightViewPos.z);

    // 0.0 -> in shadow
    // 1.0 -> in light
    
    return PercentageCloserFilteringSoftShadow(shadowMap, sample, texSpacePos);
}

float3 ComputeDirectionalLight(Texture2D shadowMap, float3 posWS, float3 normalWS)
{
    const float3 viewDir = GetViewDir(posWS);

    const DirectionalLight light = g_lights.directionalLight;
    const float3 lightDir = light.direction;

    const float diffuse = max(0.0f, dot(normalWS, -lightDir));
    const float3 halfDir = normalize(lightDir + viewDir);
    const float specular = pow(max(0.0f, dot(halfDir, normalWS)), 16) * 0.5f;

    const float3 lightColor = light.color * light.intensity;

    float4 lightViewPos = mul(light.viewProj, float4(posWS, 1.0f));
    lightViewPos.xyz /= lightViewPos.w;
    
    const float shadow = ComputeShadow(shadowMap, LinearWarp, lightViewPos);

    float3 color = (diffuse + specular) * lightColor * shadow;
    color = saturate(color + g_lights.ambientLight);

    return color;
}

#endif