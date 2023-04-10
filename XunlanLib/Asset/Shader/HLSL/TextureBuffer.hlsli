cbuffer ShadowMapIndices : register(b3)
{
    uint ShadowMapIndices[4];
}

cbuffer TextureIndices : register(b4)
{
    uint BaseColorIndex;
    uint RoughnessIndex;
    uint MetallicIndex;
    uint NormalMapIndex;
    uint HeightMapIndex;
    uint AOMapIndex;
}

SamplerState PointWrap : register(s0);
SamplerState PointClamp : register(s1);
SamplerState LinearWarp : register(s2);
SamplerState LinearClamp : register(s3);
SamplerState AnisotropicWarp : register(s4);
SamplerState AnisotropicClamp : register(s5);