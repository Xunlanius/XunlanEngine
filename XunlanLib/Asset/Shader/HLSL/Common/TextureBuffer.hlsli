#ifndef TEXTURE
#define TEXTURE

SamplerState PointWrap : register(s0);
SamplerState PointClamp : register(s1);
SamplerState LinearWarp : register(s2);
SamplerState LinearClamp : register(s3);
SamplerState AnisotropicWarp : register(s4);
SamplerState AnisotropicClamp : register(s5);

#endif