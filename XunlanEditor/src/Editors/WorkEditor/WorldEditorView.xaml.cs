using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using XunlanEditor.Content;
using XunlanEditor.GameCode;
using XunlanEditor.GameProject;

namespace XunlanEditor.Editors
{
    /// <summary>
    /// WorldEditorView.xaml 的交互逻辑
    /// </summary>
    public partial class WorldEditorView : UserControl
    {
        public WorldEditorView()
        {
            InitializeComponent();

            Loaded += OnWorldEditorViewLoaded;
        }

        private void OnWorldEditorViewLoaded(object sender,RoutedEventArgs e)
        {
            Loaded -= OnWorldEditorViewLoaded;

            Focus();

            Project project = DataContext as Project;
            //((INotifyCollectionChanged)project.UndoRedo.UndoList).CollectionChanged += (sender, e) => Focus();
        }

        private void On_CreateScriptButton_Click(object sender,RoutedEventArgs e)
        {
            new CreateScriptDiag().ShowDialog();
        }

        private void On_CreatePrimitiveButton_Click(object sender,RoutedEventArgs e)
        {
            var diag = new PrimitiveMeshDiag();
            diag.ShowDialog();
        }

        private void OnNewProject(object sender,ExecutedRoutedEventArgs e)
        {
            Window engineWindow = Window.GetWindow(this);
            engineWindow.Close();
        }
        private void OnOpenProject(object sender,ExecutedRoutedEventArgs e)
        {
            Window engineWindow = Window.GetWindow(this);
            engineWindow.Close();
        }
        private void OnCloseProject(object sender,ExecutedRoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }
    }
}
