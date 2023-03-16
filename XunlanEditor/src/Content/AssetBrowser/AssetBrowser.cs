using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Threading.Tasks;
using XunlanEditor.GameProject;
using XunlanEditor.src.Content;
using XunlanEditor.Utilities;

namespace XunlanEditor.Content
{
    sealed class ContentInfo
    {
        public string FullPath
        {
            get;
        }
        public string FullName
        {
            get => Path.GetFileNameWithoutExtension(FullPath);
        }
        public bool IsDirectory
        {
            get;
        }
        public DateTime DateModified
        {
            get;
        }
        public long? Size
        {
            get;
        }

        public ContentInfo(string fullPath,DateTime? lastModified = null)
        {
            Debug.Assert(File.Exists(fullPath) || Directory.Exists(fullPath));
            FileInfo fileInfo = new FileInfo(fullPath);
            IsDirectory = AssetHelper.IsDirectory(fullPath);
            DateModified = lastModified ?? fileInfo.LastWriteTime;
            Size = IsDirectory ? (long?)null : fileInfo.Length;
            FullPath = fullPath;
        }
    }

    class AssetBrowser : ViewModelBase
    {
        private static readonly object _lock = new object();

        private static readonly DelayEventTimer _refreshTimer = new DelayEventTimer(TimeSpan.FromMilliseconds(250));

        public string AssetFolder
        {
            get;
        }

        private readonly ObservableCollection<ContentInfo> _folderContent = new ObservableCollection<ContentInfo>();
        public ReadOnlyObservableCollection<ContentInfo> FolderContent
        {
            get;
        }

        private string _selectedFolder;
        public string SelectedFolder
        {
            get => _selectedFolder;
            set
            {
                if(value == _selectedFolder)
                    return;
                _selectedFolder = value;
                if(!string.IsNullOrEmpty(_selectedFolder))
                {
                    GetFolderContent();
                }
                OnPropertyChanged(nameof(SelectedFolder));
            }
        }

        public AssetBrowser(Project project)
        {
            Debug.Assert(project != null);
            string assetFolder = project.AssetsDirPath;
            Debug.Assert(!string.IsNullOrEmpty(assetFolder.Trim()));
            assetFolder = Path.TrimEndingDirectorySeparator(assetFolder);
            AssetFolder = assetFolder;
            SelectedFolder = AssetFolder;
            FolderContent = new ReadOnlyObservableCollection<ContentInfo>(_folderContent);

            AssetWatcher.AssetModified += OnAssetModified;
            _refreshTimer.Handler += Refresh;
        }

        private void OnAssetModified(object sender,AssetModifiedEventArgs e)
        {
            if(Path.GetDirectoryName(e.FullPath) != SelectedFolder)
                return;
            _refreshTimer.Trigger();
        }

        private void Refresh(object sender,DelayEventTimerArgs e)
        {
            GetFolderContent();
        }

        private async void GetFolderContent()
        {
            var folderContent = new List<ContentInfo>();
            await Task.Run(() => folderContent = GetFolderContent(SelectedFolder));

            _folderContent.Clear();
            folderContent.ForEach(x => _folderContent.Add(x));
        }
        private List<ContentInfo> GetFolderContent(string path)
        {
            Debug.Assert(!string.IsNullOrEmpty(path));
            var folderContent = new List<ContentInfo>();

            try
            {
                // Get folders
                foreach(string dir in Directory.GetDirectories(path))
                {
                    folderContent.Add(new ContentInfo(dir));
                }

                // Get files
                lock(_lock)
                {
                    foreach(string file in Directory.GetFiles(path,$"*{Asset.AssetFileExtension}"))
                    {
                        folderContent.Add(new ContentInfo(file));
                    }
                }
            }
            catch(IOException ex)
            {
                Debug.WriteLine(ex.Message);
            }

            return folderContent;
        }
    }
}
