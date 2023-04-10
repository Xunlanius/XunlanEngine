#include "DX12Command.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    DX12Command::DX12Command(Device& device, D3D12_COMMAND_LIST_TYPE type)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = type;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;
        desc.NodeMask = 0;

        Check(device.CreateCommandQueue(&desc, IID_PPV_ARGS(&m_cmdQueue)));

        NAME_OBJECT(
            m_cmdQueue,
            type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
            L"Direct Command Queue" :
            type == D3D12_COMMAND_LIST_TYPE_COPY ?
            L"Copy Command Queue" :
            type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
            L"Compute Command Queue" : L"Unknown Command Queue"
        );

        for (uint32 i = 0; i < NUM_FRAME_BUFFERS; ++i)
        {
            Check(device.CreateCommandAllocator(type, IID_PPV_ARGS(&m_cmdFrames[i].m_cmdAllocator)));

            NAME_OBJECT_INDEX(
                m_cmdFrames[i].m_cmdAllocator,
                type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
                L"Direct Command Allocator" :
                type == D3D12_COMMAND_LIST_TYPE_COPY ?
                L"Copy Command Allocator" :
                type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
                L"Compute Command Allocator" : L"Unknown Command Allocator",
                i
            );
        }

        Check(device.CreateCommandList(0, type, m_cmdFrames[0].m_cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&m_cmdList)));
        Check(m_cmdList->Close());

        NAME_OBJECT(
            m_cmdList,
            type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
            L"Direct Command List" :
            type == D3D12_COMMAND_LIST_TYPE_COPY ?
            L"Copy Command List" :
            type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
            L"Compute Command List" : L"Unknown Command List"
        );

        Check(device.CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

        NAME_OBJECT(m_fence, L"Fence");

        m_fenceEvent = ::CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        assert(m_fenceEvent);
    }
    DX12Command::~DX12Command()
    {
        Flush();

        for (CommandFrame& frame : m_cmdFrames) frame.Release();

        m_cmdQueue.Reset();
        m_cmdList.Reset();
        m_fence.Reset();

        m_fenceValue = 0;
        ::CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }

    void DX12Command::BeginRecord()
    {
        CommandFrame& frame = m_cmdFrames[m_currFrameIndex];
        frame.Wait(m_fenceEvent, m_fence.Get());

        Check(frame.m_cmdAllocator->Reset());
        Check(m_cmdList->Reset(frame.m_cmdAllocator.Get(), nullptr));
    }
    void DX12Command::EndRecord()
    {
        Check(m_cmdList->Close());
        ID3D12CommandList* const cmdLists[] = { m_cmdList.Get() };
        m_cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

        // Update fence value
        m_cmdFrames[m_currFrameIndex].m_fenceValue = ++m_fenceValue;
        Check(m_cmdQueue->Signal(m_fence.Get(), m_fenceValue));

        // Update frame index
        m_currFrameIndex = (m_currFrameIndex + 1) % NUM_FRAME_BUFFERS;
    }

    void DX12Command::Flush()
    {
        Check(m_cmdQueue->Signal(m_fence.Get(), ++m_fenceValue));

        for (CommandFrame& frame : m_cmdFrames)
        {
            frame.Wait(m_fenceEvent, m_fence.Get());
        }

        m_currFrameIndex = 0;
    }

    void DX12Command::CommandFrame::Wait(HANDLE fenceEvent, ID3D12Fence* fence)
    {
        assert(fenceEvent && fence);

        if (fence->GetCompletedValue() < m_fenceValue)
        {
            Check(fence->SetEventOnCompletion(m_fenceValue, fenceEvent));
            ::WaitForSingleObject(fenceEvent, INFINITE);
        }
    }
    void DX12Command::CommandFrame::Release()
    {
        m_cmdAllocator.Reset();
        m_fenceValue = 0;
    }
}