using System.Windows.Controls;

namespace XunlanEditor.GameProject
{
    /// <summary>
    /// OpenProjectView.xaml 的交互逻辑
    /// </summary>
    public partial class OpenProjectView : UserControl
    {
        public OpenProjectView()
        {
            InitializeComponent();
            DataContext = new OpenProjectViewModel();
        }
    }
}
