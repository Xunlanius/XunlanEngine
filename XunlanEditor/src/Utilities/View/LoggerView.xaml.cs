using System.Windows;
using System.Windows.Controls;

namespace XunlanEditor.Utilities
{
    /// <summary>
    /// LoggerView.xaml 的交互逻辑
    /// </summary>
    public partial class LoggerView : UserControl
    {
        public LoggerView()
        {
            InitializeComponent();
        }

        private void OnFilterChanged_CheckBox_Checked(object sender,RoutedEventArgs e)
        {
            int mask = 0;

            if(infoCheck?.IsChecked == true)
                mask |= (int)MsgType.Info;
            if(warningCheck?.IsChecked == true)
                mask |= (int)MsgType.Warning;
            if(errorCheck?.IsChecked == true)
                mask |= (int)MsgType.Error;

            Logger.SetMessageFilter(mask);
        }
    }
}
