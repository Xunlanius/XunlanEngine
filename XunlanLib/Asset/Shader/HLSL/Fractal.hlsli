#define MANDEL_RE_START -2.8f
#define MANDEL_RE_END 1.0f
#define MANDEL_IM_START -1.5f
#define MANDEL_IM_END 1.5f

#define JULIA_RE_START -2.0f
#define JULIA_RE_END 2.0f
#define JULIA_IM_START -1.5f
#define JULIA_IM_END 1.5f

#define MAX_ITERATION 1000

float2 ComplexSquare(float2 z)
{
    return float2(z.x * z.x - z.y * z.y, 2.0f * z.x * z.y);
}

float3 MapColor(float i)
{
    float3 ambient = float3(0.009f, 0.012f, 0.016f);
    return float3(2.0f * i, 4.0f * i, 8.0f * i) + ambient;
}

float3 DrawMandelBrot(float2 uv)
{
    const float real = MANDEL_RE_START + uv.x * (MANDEL_RE_END - MANDEL_RE_START);
    const float imaginary = MANDEL_IM_START + uv.y * (MANDEL_IM_END - MANDEL_IM_START);
    const float2 c = float2(real, imaginary);
    
    float2 z = 0.0f;
    for (uint i = 0; i < MAX_ITERATION; ++i)
    {
        z = ComplexSquare(z) + c;
        
        const float distance = dot(z, z);
        if (distance > 10.0f)
        {
            return MapColor((float)i * 5 / MAX_ITERATION);
        }
    }

    return 1.0f;
}

float3 DrawJuliaSet(float2 uv, uint frame)
{
    const float real = JULIA_RE_START + uv.x * (JULIA_RE_END - JULIA_RE_START);
    const float imaginary = JULIA_IM_START + uv.y * (JULIA_IM_END - JULIA_IM_START);
    float2 z = float2(real, imaginary);
    
    const float f = frame * 0.0001f;
    const float2 w = float2(cos(f), sin(f));
    const float2 c = (2.0f * w - ComplexSquare(w)) * 0.26f;
    
    for (uint i = 0; i < MAX_ITERATION; ++i)
    {
        z = ComplexSquare(z) + c;
        
        const float distance = dot(z, z);
        if (distance > 10.0f)
        {
            return MapColor((float) i / MAX_ITERATION);
        }
    }

    return 1.0f;
}