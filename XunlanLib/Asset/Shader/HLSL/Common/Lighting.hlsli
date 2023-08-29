#ifndef LIGHTING
#define LIGHTING

#include "Data/CBuffer.hlsli"
#include "Data/Sampler.hlsli"
#include "Transform.hlsli"

float PercentageCloserFilteringSoftShadow(Texture2D shadowMap, SamplerState sample, float4 lightPos)
{
    const float bias = 0.01;
    
    float width, height, numLevels;
    shadowMap.GetDimensions(0, width, height, numLevels);
    const float2 texelSize = 1.0f / float2(width, height);
    
    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            const float2 uv = lightPos.xy + float2(x, y) * texelSize;
            const float depth = shadowMap.Sample(sample, uv).r;

            shadow += lightPos.z - bias > depth ? 0.0f : 1.0f;
        }
    }
    
    return shadow /= 9.0;
}

float HardShadow(Texture2D shadowMap, SamplerState sample, float4 lightPos)
{
    const float bias = 0.01;
    const float depth = shadowMap.Sample(sample, lightPos.xy).r;
    
    return lightPos.z - bias > depth ? 0.0f : 1.0f;
}

float ComputeShadow(Texture2D shadowMap, SamplerState sample, float4 lightPos)
{
    lightPos.xyz /= lightPos.w;

    if (lightPos.x < -1.0f || lightPos.x > 1.0f ||
        lightPos.y < -1.0f || lightPos.y > 1.0f ||
        lightPos.z < 0.0f || lightPos.z > 1.0f)
    {
        return 0.0f;
    }
    
    lightPos.x = lightPos.x / 2 + 0.5;
    lightPos.y = lightPos.y / -2 + 0.5;

    // 0.0 -> in shadow
    // 1.0 -> in light
    // const float shadow = HardShadow(shadowMap, sample, lightPos);
    const float shadow = PercentageCloserFilteringSoftShadow(shadowMap, sample, lightPos);
    return shadow;
}

float3 ComputeDirectionalLight(float3 worldPos, float3 worldNormal)
{
    const float3 viewDir = GetViewDir(worldPos);

    const DirectionalLight light = g_perFrame.directionalLight;
    const float3 lightDir = light.direction;
        
    const float diffuse = max(0.0f, dot(worldNormal, -lightDir));
    const float3 halfDir = normalize(lightDir + viewDir);
    const float specular = pow(max(0.0f, dot(halfDir, worldNormal)), 16) * 0.5f;
        
    const float3 lightColor = light.color * light.intensity;
        
    Texture2D shadowMap = ResourceDescriptorHeap[g_shadowMaps.maps[0].depthIndex];
    const float shadow = ComputeShadow(shadowMap, LinearWarp, mul(light.viewProj, float4(worldPos, 1.0f)));
        
    float3 color = (diffuse + specular) * lightColor * shadow;
    color = saturate(color + g_perFrame.ambientLight);
    
    return color;
}

#endif