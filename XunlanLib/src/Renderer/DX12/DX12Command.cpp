#include "DX12Command.h"

namespace Xunlan::Graphics::DX12
{
    using namespace Microsoft::WRL;

    DX12Command::DX12Command(Device& device, D3D12_COMMAND_LIST_TYPE type)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = type;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;

        Check(device.CreateCommandQueue(&desc, IID_PPV_ARGS(&m_cmdQueue)));

        NAME_OBJECT(
            m_cmdQueue,
            type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
            L"Direct Command Queue" :
            type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
            L"Compute Command Queue" : L"Command Queue");

        for (uint32 i = 0; i < NUM_FRAME_BUFFERS; ++i)
        {
            Check(device.CreateCommandAllocator(type, IID_PPV_ARGS(&m_cmdFrames[i].cmdAllocator)));

            NAME_OBJECT_INDEX(
                m_cmdFrames[i].cmdAllocator,
                type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
                L"Direct Command Allocator" :
                type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
                L"Compute Command Allocator" : L"Command Allocator",
                i);
        }

        Check(device.CreateCommandList(0, type, m_cmdFrames[0].cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&m_cmdList)));
        Check(m_cmdList->Close());

        NAME_OBJECT(
            m_cmdList,
            type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
            L"Direct Command List" :
            (type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command List" : L"Command List"));

        Check(device.CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

        NAME_OBJECT(m_fence, L"Fence");

        m_fenceEvent = ::CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        assert(m_fenceEvent);
    }

    void DX12Command::OnFrameBegin()
    {
        CommandFrame& frame = m_cmdFrames[m_currFrameIndex];
        frame.Wait(m_fenceEvent, m_fence.Get());

        Check(frame.cmdAllocator->Reset());
        Check(m_cmdList->Reset(frame.cmdAllocator.Get(), nullptr));
    }
    void DX12Command::OnFrameEnd(DX12Surface& surface)
    {
        Check(m_cmdList->Close());
        ID3D12CommandList* const cmdLists[] = { m_cmdList.Get() };
        m_cmdQueue->ExecuteCommandLists(1, cmdLists);

        surface.Present();

        ++m_fenceValue;
        m_cmdFrames[m_currFrameIndex].fenceValue = m_fenceValue;
        m_cmdQueue->Signal(m_fence.Get(), m_fenceValue);

        m_currFrameIndex = (m_currFrameIndex + 1) % NUM_FRAME_BUFFERS;
    }

    void DX12Command::Release()
    {
        Flush();

        m_cmdQueue.Reset();
        m_cmdList.Reset();
        m_fence.Reset();

        m_fenceValue = 0;
        ::CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;

        for (uint32 i = 0; i < NUM_FRAME_BUFFERS; ++i)
        {
            m_cmdFrames[i].Release();
        }
    }
    void DX12Command::Flush()
    {
        for (uint32 i = 0; i < NUM_FRAME_BUFFERS; ++i)
        {
            m_cmdFrames[i].Wait(m_fenceEvent, m_fence.Get());
        }

        m_currFrameIndex = 0;
    }

    void DX12Command::CommandFrame::Wait(HANDLE fenceEvent, ID3D12Fence* fence)
    {
        assert(fenceEvent && fence);

        if (fence->GetCompletedValue() < fenceValue)
        {
            Check(fence->SetEventOnCompletion(fenceValue, fenceEvent));
            ::WaitForSingleObject(fenceEvent, INFINITE);
        }
    }
    void DX12Command::CommandFrame::Release()
    {
        cmdAllocator.Reset();
        fenceValue = 0;
    }
}