using System.IO;
using System.Windows;

namespace XunlanEditor.GameProject
{
    /// <summary>
    /// EnginePathDiag.xaml 的交互逻辑
    /// </summary>
    public partial class EnginePathDiag : Window
    {
        public string EnginePath
        {
            get; private set;
        }

        public EnginePathDiag()
        {
            InitializeComponent();

            Owner = Application.Current.MainWindow;
        }

        private void On_OKButton_Click(object sender,RoutedEventArgs e)
        {
            string path = pathTextBox.Text.Trim();
            messageTextBlock.Text = string.Empty;

            if(string.IsNullOrEmpty(path))
            {
                messageTextBlock.Text = "Invalid path";
            }
            else if(path.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                messageTextBlock.Text = "Invalid characters used in the path";
            }
            else if(!Directory.Exists(Path.Combine(path,@"XunlanLib\src")))
            {
                messageTextBlock.Text = "Unable to find the engine";
            }

            if(string.IsNullOrEmpty(messageTextBlock.Text))
            {
                if(!Path.EndsInDirectorySeparator(path))
                    path += @"\";

                EnginePath = path;
                DialogResult = true;
                Close();
            }
        }
    }
}
