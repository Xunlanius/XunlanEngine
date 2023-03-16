#include "DX12Upload.h"
#include "Renderer/DX12/DX12Core.h"

namespace Xunlan::Graphics::DX12::Upload
{
    using namespace Microsoft::WRL;

    namespace
    {
        struct UploadFrame
        {
            bool IsReady() const { return uploadBuffer == nullptr; }
            void Wait();
            void Release();

            ComPtr<ID3D12CommandAllocator> cmdAllocator = nullptr;
            ComPtr<GraphicsCommandList> cmdList = nullptr;
            ComPtr<ID3D12Resource> uploadBuffer = nullptr;
            uint64 fenceValue = 0;
        };

        constexpr uint32 UPLOAD_FRAME_COUNT = 4;
        ComPtr<ID3D12CommandQueue> g_cmdQueue = nullptr;
        std::array<UploadFrame, UPLOAD_FRAME_COUNT> g_uploadFrames;
        ComPtr<ID3D12Fence1> g_fence = nullptr;
        HANDLE g_fenceEvent = {};
        uint64 g_fenceValue = 0;
        std::mutex g_mutex = {};

        void UploadFrame::Wait()
        {
            assert(g_fence && g_fenceEvent);

            if (g_fence->GetCompletedValue() < fenceValue)
            {
                Check(g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent));
                ::WaitForSingleObject(g_fenceEvent, INFINITE);
            }

            uploadBuffer.Reset();
        }
        void UploadFrame::Release()
        {
            Wait();
            cmdAllocator.Reset();
            cmdList.Reset();
            fenceValue = 0;
        }

        /// <summary>
        /// This method should be locked before being called
        /// </summary>
        /// <returns></returns>
        uint32 GetAvailableFrameIndex()
        {
            uint32 index = 0;

            while (!g_uploadFrames[index].IsReady())
            {
                index = (index + 1) % UPLOAD_FRAME_COUNT;
                std::this_thread::yield();
            }

            return index;
        }
    }

    bool Initialize()
    {
        Device& device = Core::GetDevice();

        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;

        Check(device.CreateCommandQueue(&desc, IID_PPV_ARGS(&g_cmdQueue)));
        NAME_OBJECT(g_cmdQueue, L"Upload Command Queue");

        Check(device.CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)));
        NAME_OBJECT(g_fence, L"Upload Fence");

        g_fenceEvent = ::CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        if (!g_fenceEvent) { assert(false); return false; }

        for (uint32 i = 0; i < UPLOAD_FRAME_COUNT; ++i)
        {
            UploadFrame& frame = g_uploadFrames[i];

            Check(device.CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&frame.cmdAllocator)));
            Check(device.CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, frame.cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&frame.cmdList)));
            frame.cmdList->Close();
            NAME_OBJECT_INDEX(frame.cmdAllocator, L"Upload Command Allocator", i);
            NAME_OBJECT_INDEX(frame.cmdList, L"Upload Command List", i);
        }

        return true;
    }
    void Shutdown()
    {
        for (UploadFrame& frame : g_uploadFrames) { frame.Release(); }

        ::CloseHandle(g_fenceEvent);
        g_fence.Reset();
        g_cmdQueue.Reset();
        g_fenceValue = 0;
    }

    UploadContext::UploadContext(uint64 alignedSize)
    {
        {
            std::lock_guard lock(g_mutex);
            m_frameIndex = GetAvailableFrameIndex();
            g_uploadFrames[m_frameIndex].uploadBuffer = (ID3D12Resource*)1;
        }

        UploadFrame& frame = g_uploadFrames[m_frameIndex];

        frame.uploadBuffer = Helper::Resource::CreateBuffer(nullptr, alignedSize, true);
        NAME_OBJECT_INDEX(frame.uploadBuffer, L"Upload Buffer - Size: ", alignedSize);

        const D3D12_RANGE range = {};
        Check(m_uploadBuffer->Map(0, &range, &m_cpuAddress));
        assert(m_cpuAddress);

        m_cmdList = frame.cmdList.Get();
        m_uploadBuffer = frame.uploadBuffer.Get();
        assert(m_cmdList && m_uploadBuffer);

        Check(frame.cmdAllocator->Reset());
        Check(frame.cmdList->Reset(frame.cmdAllocator.Get(), nullptr));
    }

    void UploadContext::EndUpload()
    {
        const D3D12_RANGE range = {};
        m_uploadBuffer->Unmap(0, &range);

        m_cmdList->Close();
        ID3D12CommandList* const cmdLists[] = { m_cmdList };
        g_cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

        UploadFrame& frame = g_uploadFrames[m_frameIndex];

        frame.fenceValue = ++g_fenceValue;
        g_cmdQueue->Signal(g_fence.Get(), g_fenceValue);

        frame.Wait();
    }
}