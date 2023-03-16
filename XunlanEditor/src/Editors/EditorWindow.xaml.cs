using System;
using System.ComponentModel;
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

        private void OnEditorWindow_Loaded(object sender,RoutedEventArgs e)
        {
            Loaded -= OnEditorWindow_Loaded;
        }

        private void OnEditorWindow_Closing(object sender,CancelEventArgs e)
        {
            Closing -= OnEditorWindow_Closing;
            Project.CurrProject?.Unload();
            DataContext = null;
        }

        private void OnEditorWindow_Closed(object sender,EventArgs e)
        {
            Closed -= OnEditorWindow_Closed;
            Application.Current.MainWindow.Show();
        }
    }
}
