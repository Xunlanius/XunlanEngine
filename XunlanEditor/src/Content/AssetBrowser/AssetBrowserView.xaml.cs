using System;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using XunlanEditor.Editors;
using XunlanEditor.GameProject;

namespace XunlanEditor.Content
{
    /// <summary>
    /// AssertBrowserView.xaml 的交互逻辑
    /// </summary>
    public partial class AssetBrowserView : UserControl
    {
        public AssetBrowserView()
        {
            DataContext = null;
            InitializeComponent();
            Loaded += OnContentBrowserLoaded;
        }

        private void OnContentBrowserLoaded(object sender,RoutedEventArgs e)
        {
            Loaded -= OnContentBrowserLoaded;

            AssetBrowser viewModel = new AssetBrowser(Project.CurrProject);
            viewModel.PropertyChanged += OnSelectedFolderChanged;
            DataContext = viewModel;
            AllowDrop = true;
        }

        private void OnSelectedFolderChanged(object sender,PropertyChangedEventArgs e)
        {
            AssetBrowser viewModel = DataContext as AssetBrowser;

            if(e.PropertyName == nameof(viewModel.SelectedFolder) && !string.IsNullOrEmpty(viewModel.SelectedFolder))
            {
                GeneratePathStackButton();
            }
        }

        /// <summary>
        /// Show the path stack of the directory
        /// </summary>
        private void GeneratePathStackButton()
        {
            AssetBrowser viewModel = DataContext as AssetBrowser;
            string path = Directory.GetParent(Path.TrimEndingDirectorySeparator(viewModel.SelectedFolder)).FullName;
            string assetPath = Path.TrimEndingDirectorySeparator(viewModel.AssetFolder);

            pathStack.Children.RemoveRange(0,pathStack.Children.Count);
            if(viewModel.SelectedFolder == viewModel.AssetFolder)
                return;

            string[] paths = new string[3];
            string[] labels = new string[3];

            int i = 0;
            for(;i < 3;++i)
            {
                paths[i] = path;
                labels[i] = path[(path.LastIndexOf(Path.DirectorySeparatorChar) + 1)..];
                if(path == assetPath)
                    break;
                path = path.Substring(0,path.LastIndexOf(Path.DirectorySeparatorChar));
            }

            if(i == 3)
                i = 2;
            for(;i >= 0;--i)
            {
                Button button = new Button()
                {
                    DataContext = paths[i],
                    Content = new TextBlock() { Text = labels[i],TextTrimming = TextTrimming.CharacterEllipsis },
                };
                pathStack.Children.Add(button);
                if(i > 0)
                    pathStack.Children.Add(new System.Windows.Shapes.Path());
            }
        }

        private void OnContentItem_MouseDoubleClick(object sender,MouseButtonEventArgs e)
        {
            ContentInfo info = (sender as FrameworkElement).DataContext as ContentInfo;
            ExecuteSelection(info);
        }

        private void ExecuteSelection(ContentInfo info)
        {
            if(info == null)
                return;
            AssetBrowser viewModel = DataContext as AssetBrowser;

            if(info.IsDirectory)
            {
                viewModel.SelectedFolder = info.FullPath;
            }
            else
            {
                AssetInfo assetInfo = Asset.GetAssetInfo(info.FullPath);
                if(assetInfo == null)
                    return;

                CreateAssetEditor(assetInfo);
            }
        }

        private IAssetEditor CreateAssetEditor(AssetInfo info)
        {
            IAssetEditor editor = null;

            try
            {
                switch(info.Type)
                {
                    case AssetType.Mesh:
                        editor = OpenEditorPanel<GeometryEditorView>(info,info.Guid,"GeometryEditor");
                        break;
                    case AssetType.Texture:
                        break;
                    case AssetType.Material:
                        break;
                    case AssetType.Animation:
                        break;
                    case AssetType.Audio:
                        break;
                    case AssetType.Skeleton:
                        break;
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }

            return editor;
        }

        private IAssetEditor OpenEditorPanel<T>(AssetInfo info,Guid guid,string title) where T : FrameworkElement, new()
        {
            // Activate and return the editor window if already existed
            foreach(Window window in Application.Current.Windows)
            {
                if(window.Content is FrameworkElement content &&
                    content.DataContext is IAssetEditor editor &&
                    editor.Asset.Guid == info.Guid)
                {
                    window.Activate();
                    return editor;
                }
            }

            // Create a new editor window
            T newEditorView = new T();
            Debug.Assert(newEditorView.DataContext is IAssetEditor);
            (newEditorView.DataContext as IAssetEditor).SetAsset(info);

            Window newWindow = new Window()
            {
                Content = newEditorView,
                Title = title,
                Owner = Application.Current.MainWindow,
                WindowStartupLocation = WindowStartupLocation.CenterOwner,
                Style = Application.Current.FindResource("XunlanWindowStyle") as Style,
            };

            newWindow.Show();
            return newEditorView.DataContext as IAssetEditor;
        }

        private void OnPathStackButton_Click(object sender,RoutedEventArgs e)
        {
            AssetBrowser viewModel = DataContext as AssetBrowser;
            viewModel.SelectedFolder = (sender as Button).DataContext as string;
        }

        /// <summary>
        /// When drop an asset to the view
        /// </summary>
        private void OnFolderContent_ListView_Drop(object sender,DragEventArgs e)
        {
            AssetBrowser viewModel = DataContext as AssetBrowser;
            if(viewModel.SelectedFolder != null && e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
                if(files?.Length > 0 && Directory.Exists(viewModel.SelectedFolder))
                {
                    _ = AssetHelper.ImportFilesAsync(files,viewModel.SelectedFolder);
                    e.Handled = true;
                }
            }
        }
    }
}
