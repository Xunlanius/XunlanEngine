using System.Windows;

namespace XunlanEditor.GameProject
{
    /// <summary>
    /// ProjectBrowserDialogue.xaml 的交互逻辑
    /// </summary>
    public partial class ProjectBrowser : Window
    {
        private bool _isLeft = true;

        public ProjectBrowser()
        {
            ProjectFileInfo.UpdateEnginePath();

            InitializeComponent();
        }

        private void OnToggleClick(object sender,RoutedEventArgs e)
        {
            if(sender == openProjectButton)
            {
                if(!_isLeft)
                {
                    createProjectButton.IsChecked = false;
                    _isLeft = true;

                    openProjectView.IsEnabled = true;
                    createProjectView.IsEnabled = false;

                    //browserContent.Margin = new Thickness(0);
                    openProjectView.Visibility = Visibility.Visible;
                    createProjectView.Visibility = Visibility.Collapsed;
                }
                else
                {
                    openProjectButton.IsChecked = true;
                }
            }
            else if(sender == createProjectButton)
            {
                if(_isLeft)
                {
                    openProjectButton.IsChecked = false;
                    _isLeft = false;

                    openProjectView.IsEnabled = false;
                    createProjectView.IsEnabled = true;

                    //browserContent.Margin = new Thickness(0, -Height, 0, 0);
                    openProjectView.Visibility = Visibility.Collapsed;
                    createProjectView.Visibility = Visibility.Visible;
                }
                else
                {
                    createProjectButton.IsChecked = true;
                }
            }
        }
    }
}
