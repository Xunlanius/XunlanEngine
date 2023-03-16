using System.Windows.Controls;

namespace XunlanEditor.GameProject
{
    /// <summary>
    /// CreateProjectView.xaml 的交互逻辑
    /// </summary>
    public partial class CreateProjectView : UserControl
    {
        public CreateProjectView()
        {
            InitializeComponent();
            DataContext = new CreateProjectViewModel();
        }
    }
}
