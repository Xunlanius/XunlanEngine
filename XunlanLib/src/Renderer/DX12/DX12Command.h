#pragma once

#include "Renderer/DX12/DX12Common.h"
#include "Renderer/DX12/DX12Surface.h"

namespace Xunlan::Graphics::DX12
{
    class DX12Command final
    {
    public:

        explicit DX12Command(Device& device, D3D12_COMMAND_LIST_TYPE type);
        DX12Command(const DX12Command& rhs) = delete;
        DX12Command& operator =(const DX12Command& rhs) = delete;
        DX12Command(DX12Command&& rhs) = delete;
        DX12Command& operator =(DX12Command&& rhs) = delete;
        ~DX12Command() = default;

    public:

        void OnFrameBegin();
        void OnFrameEnd(DX12Surface& surface);

        void Release();
        void Flush();

        ID3D12CommandQueue* GetCommandQueue() const { return m_cmdQueue.Get(); }
        GraphicsCommandList* GetCommandList() const { return m_cmdList.Get(); }
        uint32 GetCurrFrameIndex() const { return m_currFrameIndex; }

    private:

        struct CommandFrame
        {
            void Wait(HANDLE fenceEvent, ID3D12Fence* fence);
            void Release();

            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAllocator = nullptr;
            uint64 fenceValue = 0;
        };

        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cmdQueue = nullptr;
        Microsoft::WRL::ComPtr<GraphicsCommandList> m_cmdList = nullptr;
        CommandFrame m_cmdFrames[NUM_FRAME_BUFFERS] = {};

        uint32 m_currFrameIndex = 0;

        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence = nullptr;
        HANDLE m_fenceEvent = nullptr;
        uint64 m_fenceValue = 0;
    };
}