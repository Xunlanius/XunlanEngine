using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Windows;
using XunlanEditor.GameProject;

namespace XunlanEditor.GameProject
{
    class ProjectBrowserViewModel
    {
        public static void CreateEditorWindow(Project project)
        {
            if (project != null)
            {
                Project.CurrProject = project;

                EditorWindow editorWindow = new EditorWindow { DataContext = project };
                Application.Current.MainWindow.Visibility = Visibility.Hidden;

                editorWindow.ShowDialog();
            }
            else Application.Current.Shutdown();
        }
    } 
}
