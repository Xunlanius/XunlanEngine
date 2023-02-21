using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace XunlanEditor.Editors
{
    /// <summary>
    /// ComponentView.xaml 的交互逻辑
    /// </summary>
    [ContentProperty("ComponentContent")]
    public partial class ComponentView : UserControl
    {
        public string Header
        {
            get => (string)GetValue(HeaderProperty);
            set => SetValue(HeaderProperty, value);
        }
        public static readonly DependencyProperty HeaderProperty = DependencyProperty.Register(
            nameof(Header),
            typeof(string),
            typeof(ComponentView),
            new PropertyMetadata(string.Empty));

        public FrameworkElement ComponentContent
        {
            get => (FrameworkElement)GetValue(ComponentContentProperty);
            set => SetValue(ComponentContentProperty, value);
        }
        public static readonly DependencyProperty ComponentContentProperty = DependencyProperty.Register(
            nameof(ComponentContent),
            typeof(FrameworkElement),
            typeof(ComponentView));

        public ComponentView()
        {
            InitializeComponent();
        }
    }
}
