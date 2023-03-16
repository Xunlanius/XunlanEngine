#pragma once

#include "Renderer/DX12/DX12Common.h"

namespace Xunlan::Graphics::DX12::Upload
{
    bool Initialize();
    void Shutdown();

    /// <summary>
    /// Once called method EndUpload(), the object can't be used again
    /// </summary>
    class UploadContext final
    {
    public:

        explicit UploadContext(uint64 alignedSize);
        UploadContext(const UploadContext& rhs) = delete;
        UploadContext(UploadContext&& rhs) = delete;
        UploadContext& operator=(const UploadContext& rhs) = delete;
        UploadContext& operator=(UploadContext&& rhs) = delete;

    public:

        void EndUpload();

        GraphicsCommandList* GetCommandList() const { return m_cmdList; }
        ID3D12Resource* GetUploadBuffer() const { return m_uploadBuffer; }
        void* GetCPUAddress() const { return m_cpuAddress; }

    private:

        // This class doesn't hold the control of the following 2 members
        GraphicsCommandList* m_cmdList = nullptr;
        ID3D12Resource* m_uploadBuffer = nullptr;

        void* m_cpuAddress = nullptr;
        uint32 m_frameIndex = UINT32_MAX;
    };
}