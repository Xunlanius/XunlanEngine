#pragma once

#include "DX12Common.h"
#include "DX12Surface.h"

namespace Xunlan::DX12
{
    class DX12Command final
    {
    public:

        explicit DX12Command(Device& device, D3D12_COMMAND_LIST_TYPE type);
        DISABLE_COPY(DX12Command)
        DISABLE_MOVE(DX12Command)
        ~DX12Command();

    public:

        void BeginRecord();
        void EndRecord();

        void Flush();

        ID3D12CommandQueue* GetCommandQueue() const { return m_cmdQueue.Get(); }
        GraphicsCommandList* GetCommandList() const { return m_cmdList.Get(); }
        uint32 GetCurrFrameIndex() const { return m_currFrameIndex; }

    private:

        struct CommandFrame
        {
            void Wait(HANDLE fenceEvent, ID3D12Fence* fence);
            void Release();

            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_cmdAllocator;
            uint64 m_fenceValue = 0;
        };

        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cmdQueue;
        Microsoft::WRL::ComPtr<GraphicsCommandList> m_cmdList;

        CommandFrame m_cmdFrames[NUM_FRAME_BUFFERS];
        uint32 m_currFrameIndex = 0;

        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence = nullptr;
        HANDLE m_fenceEvent = nullptr;
        uint64 m_fenceValue = 0;
    };
}