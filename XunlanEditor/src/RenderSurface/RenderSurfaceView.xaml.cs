using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;

namespace XunlanEditor.RenderSurface
{
    /// <summary>
    /// RenderSurfaceView.xaml 的交互逻辑
    /// </summary>
    public partial class RenderSurfaceView : UserControl, IDisposable
    {
        private enum Win32Msg
        {
            WM_SIZE = 0x0005,
            WM_SIZING = 0x0214,
            WM_ENTERSIZEMOVE = 0x0231,
            WM_EXITSIZEMOVE = 0x0232,
        }

        private RenderSurfaceHost _host = null;

        public RenderSurfaceView()
        {
            InitializeComponent();

            Loaded += OnRenderSurfaceViewLoaded;
        }

        private void OnRenderSurfaceViewLoaded(object sender,RoutedEventArgs e)
        {
            Loaded -= OnRenderSurfaceViewLoaded;

            _host = new RenderSurfaceHost(ActualWidth,ActualHeight);
            _host.MessageHook += new HwndSourceHook(HostMsgFilter);
            Content = _host;
        }

        private IntPtr HostMsgFilter(IntPtr hwnd,int msg,IntPtr wParam,IntPtr lParam,ref bool handled)
        {
            switch((Win32Msg)msg)
            {
                case Win32Msg.WM_SIZE:
                    break;

                case Win32Msg.WM_SIZING:
                    break;

                case Win32Msg.WM_ENTERSIZEMOVE:
                    break;

                case Win32Msg.WM_EXITSIZEMOVE:
                    break;
            }

            return IntPtr.Zero;
        }

        #region IDisposable support
        private bool _disposedValue;
        protected virtual void Dispose(bool disposing)
        {
            if(!_disposedValue)
            {
                if(disposing)
                    _host.Dispose();

                _disposedValue = true;
            }
        }
        public void Dispose()
        {
            // 不要更改此代码。请将清理代码放入“Dispose(bool disposing)”方法中
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
        #endregion
    }
}
