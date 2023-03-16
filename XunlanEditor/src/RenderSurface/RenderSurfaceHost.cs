using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Windows.Interop;
using XunlanEditor.DLLInterface;
using XunlanEditor.Utilities;

namespace XunlanEditor.RenderSurface
{
    class RenderSurfaceHost : HwndHost
    {
        private IntPtr _renderWindowHandle = IntPtr.Zero;
        private readonly int _width = 800;
        private readonly int _height = 600;

        public UInt32 RenderSurfaceID { get; private set; } = ID.InvalidID;

        private DelayEventTimer _resizeDelayTimer;

        public RenderSurfaceHost(double width,double height)
        {
            _width = (int)width;
            _height = (int)height;

            _resizeDelayTimer = new DelayEventTimer(TimeSpan.FromMilliseconds(250.0));
            _resizeDelayTimer.Handler += Resize;
            SizeChanged += (s,e) => _resizeDelayTimer.Trigger();
        }

        private void Resize(object sender,DelayEventTimerArgs e)
        {
            EngineAPI.ResizeRenderSurface(RenderSurfaceID);
        }

        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            RenderSurfaceID = EngineAPI.CreateRenderSurface(hwndParent.Handle,_width,_height);
            Debug.Assert(ID.IsValid(RenderSurfaceID));

            _renderWindowHandle = EngineAPI.GetWindowHandle(RenderSurfaceID);
            Debug.Assert(_renderWindowHandle != IntPtr.Zero);

            return new HandleRef(this,_renderWindowHandle);
        }
        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            EngineAPI.RemoveRenderSurface(RenderSurfaceID);
            RenderSurfaceID = ID.InvalidID;
            _renderWindowHandle = IntPtr.Zero;
        }
    }
}
