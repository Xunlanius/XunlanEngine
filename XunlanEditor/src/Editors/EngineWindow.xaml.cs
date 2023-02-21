using System;
using System.ComponentModel;
using System.IO;
using System.Windows;
using XunlanEditor.GameProject;

namespace XunlanEditor
{
    public partial class EditorWindow : Window
    {
        public EditorWindow()
        {
            InitializeComponent();

            Loaded += OnEditorWindow_Loaded;
            Closing += OnEditorWindow_Closing;
            Closed += OnEditorWindow_Closed;
        }

        private void OnEditorWindow_Loaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnEditorWindow_Loaded;
        }

        private void OnEditorWindow_Closing(object sender, CancelEventArgs e)
        {
            Closing-= OnEditorWindow_Closing;

            Project.CurrProject?.Unload();
        }

        private void OnEditorWindow_Closed(object sender, EventArgs e)
        {
            Application.Current.Shutdown();
        }

        //private void OpenProjectBrowser()
        //{
        //    ProjectBrowser projectBrowser = new ProjectBrowser();

        //    if (projectBrowser.ShowDialog() == true && projectBrowser.DataContext != null)
        //    {
        //        Project.CurrProject?.Unload();
        //        DataContext = projectBrowser.DataContext;
        //        Visibility = Visibility.Visible;

        //        WorldEditorView worldEditorView = new WorldEditorView()
        //        {
        //            Width = this.Width,
        //            Height = this.Height,
        //        };
        //        canvas.Children.Add(worldEditorView);
        //    }
        //    else
        //    {
        //        Application.Current.Shutdown();
        //    }
        //}
    }
}
