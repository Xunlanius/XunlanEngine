#if !defined(XUNLAN_COMMON) && !defined(__cplusplus)
#error
#endif

static const uint MAX_NUM_DIRECTIONAL_LIGHTS = 4;
static const uint MAX_NUM_POINT_LIGHTS = 16;

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float2 uv;
};

struct DirectionalLight
{
    float3 direction;
    float pad0;
    float3 color;
    float pad1;
    float4x4 directionLightViewProj;
};

struct PointLight
{
    float3 position;
    float radius;
    float3 intensity;
    float pad0;
};

struct PerFrameBuffer
{
    float4x4 view;
    float4x4 proj;
    float4x4 invProj;
    float4x4 viewProj;
    float4x4 invViewProj;
    
    float3 cameraPos;
    float width;
    float3 cameraDir;
    float height;
    
    float3 ambientLight;
    uint numDirectionalLights;
    DirectionalLight directionLights[MAX_NUM_DIRECTIONAL_LIGHTS];
    float3 pad0;
    uint numPointLights;
    PointLight pointLights[MAX_NUM_POINT_LIGHTS];
    
    float deltaTime;
    float3 pad1;
};

struct PerObjectBuffer
{
    float4x4 world;
    float4x4 invWorld;
    float4x4 worldViewProj;
};

#ifdef __cplusplus
static_assert(sizeof(PerObjectBuffer) % 16 == 0, "PerObjectBuffer should be aligned to 16-bytes.");
#endif