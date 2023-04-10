#pragma once

#include "DX12Common.h"
#include <array>
#include <mutex>

namespace Xunlan::DX12
{
    class UploadContext final
    {
    public:

        explicit UploadContext(Device& device);
        ~UploadContext();

        void Upload(ID3D12Resource* defaultResource, const void* data, uint64 byteSize);

    private:

        /// <summary>
        /// This method should be locked before being called
        /// </summary>
        uint32 GetAvailableFrameIndex();

    private:

        struct UploadFrame
        {
            bool IsReady() const { return m_uploadBuffer == nullptr; }
            void Wait(ID3D12Fence1* fence, HANDLE fenceEvent);
            void Release();

            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_cmdAllocator;
            Microsoft::WRL::ComPtr<GraphicsCommandList> m_cmdList;
            Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
            uint64 m_fenceValue = 0;
        };

        static constexpr uint32 UPLOAD_FRAME_COUNT = 4;

        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cmdQueue;
        std::array<UploadFrame, UPLOAD_FRAME_COUNT> m_uploadFrames;
        Microsoft::WRL::ComPtr<ID3D12Fence1> m_fence;
        HANDLE m_fenceEvent = {};
        uint64 m_fenceValue = 0;

        std::mutex m_mutex = {};
    };
}