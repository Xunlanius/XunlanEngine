#include "DX12API.h"
#include "Renderer/GraphicsAPI.h"
#include "Renderer/DX12/DX12Core.h"

namespace Xunlan::Graphics::DX12
{
    void GetGraphicsAPI(GraphicsAPI& platformAPI)
    {
        platformAPI.platform = Platform::DX12;

        platformAPI.initialize = Core::Initialize;
        platformAPI.shutdown = Core::Shutdown;

        platformAPI.surface.create = Core::CreateSurface;
        platformAPI.surface.remove = Core::RemoveSurface;
        platformAPI.surface.render = Core::RenderSurface;

        platformAPI.surface.getWidth = Core::GetSurfaceWidth;
        platformAPI.surface.getHeight = Core::GetSurfaceHeight;
        platformAPI.surface.resize = Core::ResizeSurface;
    }
}