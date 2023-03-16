using System.Diagnostics;
using System.Windows;

namespace XunlanEditor.GameProject
{
    class ProjectBrowserViewModel
    {
        public static void CreateEditorWindow(Project project)
        {
            Debug.Assert(project != null);

            Project.CurrProject = project;

            EditorWindow editorWindow = new EditorWindow { DataContext = project };
            Application.Current.MainWindow.Hide();

            editorWindow.ShowDialog();
        }
    }
}
