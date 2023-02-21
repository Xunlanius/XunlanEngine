using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using XunlanEditor.GameProject;

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

        private void OnFilterChanged_CheckBox_Checked(object sender, RoutedEventArgs e)
        {
            int mask = 0;

            if (infoCheck?.IsChecked == true) mask |= (int)MsgType.Info;
            if (warningCheck?.IsChecked == true) mask |= (int)MsgType.Warning;
            if (errorCheck?.IsChecked == true) mask |= (int)MsgType.Error;

            Logger.SetMessageFilter(mask);
        }
    }
}
