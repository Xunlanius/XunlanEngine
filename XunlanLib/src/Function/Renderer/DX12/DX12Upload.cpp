#include "DX12Upload.h"
#include "DX12RHI.h"
#include "DX12Helper.h"

using namespace Microsoft::WRL;

namespace Xunlan::DX12
{
    UploadContext::UploadContext(Device& device)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;
        desc.NodeMask = 0;

        Check(device.CreateCommandQueue(&desc, IID_PPV_ARGS(&m_cmdQueue)));
        NAME_OBJECT(m_cmdQueue, L"Upload Command Queue");

        Check(device.CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        NAME_OBJECT(m_fence, L"Upload Fence");

        m_fenceEvent = ::CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        assert(m_fenceEvent);

        for (uint32 i = 0; i < UPLOAD_FRAME_COUNT; ++i)
        {
            UploadFrame& frame = m_uploadFrames[i];

            Check(device.CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame.m_cmdAllocator)));
            Check(device.CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame.m_cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&frame.m_cmdList)));
            frame.m_cmdList->Close();
            NAME_OBJECT_INDEX(frame.m_cmdAllocator, L"Upload Command Allocator", i);
            NAME_OBJECT_INDEX(frame.m_cmdList, L"Upload Command List", i);
        }
    }
    UploadContext::~UploadContext()
    {
        for (UploadFrame& frame : m_uploadFrames)
        {
            frame.Wait(m_fence.Get(), m_fenceEvent);
            frame.Release();
        }

        ::CloseHandle(m_fenceEvent);
        m_fence.Reset();
        m_cmdQueue.Reset();
        m_fenceValue = 0;
    }

    void UploadContext::Upload(ID3D12Resource* defaultResource, const void* data, uint64 byteSize)
    {
        assert(m_cmdQueue && "Command queue hasn't been initialized.");

        std::lock_guard lock(m_mutex);

        uint32 frameIndex = GetAvailableFrameIndex();
        UploadFrame& frame = m_uploadFrames[frameIndex];

        ID3D12CommandAllocator* cmdAllocator = frame.m_cmdAllocator.Get();
        GraphicsCommandList* cmdList = frame.m_cmdList.Get();

        Check(cmdAllocator->Reset());
        Check(cmdList->Reset(cmdAllocator, nullptr));

        frame.m_uploadBuffer = Helper::Resource::UploadToDefaultBuffer(*cmdList, defaultResource, data, byteSize);
        assert(frame.m_uploadBuffer);

        Check(cmdList->Close());
        ID3D12CommandList* const cmdLists[] = { cmdList };
        m_cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

        frame.m_fenceValue = ++m_fenceValue;
        m_cmdQueue->Signal(m_fence.Get(), m_fenceValue);

        frame.Wait(m_fence.Get(), m_fenceEvent);
    }

    uint32 UploadContext::GetAvailableFrameIndex()
    {
        uint32 index = 0;

        while (!m_uploadFrames[index].IsReady())
        {
            index = (index + 1) % UPLOAD_FRAME_COUNT;
            std::this_thread::yield();
        }

        return index;
    }

    void UploadContext::UploadFrame::Wait(ID3D12Fence1* fence, HANDLE fenceEvent)
    {
        assert(fence && fenceEvent);

        if (fence->GetCompletedValue() < m_fenceValue)
        {
            Check(fence->SetEventOnCompletion(m_fenceValue, fenceEvent));
            ::WaitForSingleObject(fenceEvent, INFINITE);
        }

        m_uploadBuffer.Reset();
    }
    void UploadContext::UploadFrame::Release()
    {
        m_cmdAllocator.Reset();
        m_cmdList.Reset();
        m_fenceValue = 0;
    }
}