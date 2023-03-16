using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows;
using XunlanEditor.Content;
using XunlanEditor.Utilities;

namespace XunlanEditor.src.Content
{
    class AssetModifiedEventArgs : EventArgs
    {
        public string FullPath { get; }

        public AssetModifiedEventArgs(string fullPath)
        {
            FullPath = fullPath;
        }
    }

    static class AssetWatcher
    {
        private static readonly FileSystemWatcher _watcher = new FileSystemWatcher()
        {
            IncludeSubdirectories = true,
            Filter = "",
            NotifyFilter = NotifyFilters.CreationTime | NotifyFilters.DirectoryName | NotifyFilters.FileName | NotifyFilters.LastWrite,
        };
        private static readonly DelayEventTimer _refreshTimer = new DelayEventTimer(TimeSpan.FromMilliseconds(250));

        public static event EventHandler<AssetModifiedEventArgs> AssetModified;

        /// <summary>
        /// File watcher is only enabled when the counter is 0
        /// </summary>
        private static int _fileWatcherEnableCounter = 0;

        static AssetWatcher()
        {
            _watcher.Changed += OnAssetModifiedAsync;
            _watcher.Created += OnAssetModifiedAsync;
            _watcher.Deleted += OnAssetModifiedAsync;
            _watcher.Renamed += OnAssetModifiedAsync;

            _refreshTimer.Handler += Refresh;
        }

        public static void EnableFileWatcher(bool isEnabled)
        {
            if(_fileWatcherEnableCounter > 0 && isEnabled) --_fileWatcherEnableCounter;
            else if(!isEnabled) ++_fileWatcherEnableCounter;
        }

        /// <summary>
        /// Should be called when a new project is open
        /// </summary>
        public static void Reset(string assetFolder,string projectDirPath)
        {
            _watcher.EnableRaisingEvents = false;

            if(!string.IsNullOrEmpty(assetFolder))
            {
                Debug.Assert(Directory.Exists(assetFolder));
                _watcher.Path = assetFolder;
                _watcher.EnableRaisingEvents = true;
                AssetRegistry.Reset(assetFolder);
            }
        }

        private static async void OnAssetModifiedAsync(object sender,FileSystemEventArgs e)
        {
            await Application.Current.Dispatcher.BeginInvoke(new Action(() => _refreshTimer.Trigger(e)));
        }

        private static void Refresh(object sender,DelayEventTimerArgs e)
        {
            if(_fileWatcherEnableCounter > 0)
            {
                _refreshTimer.Trigger();
                return;
            }

            e.Data.Cast<FileSystemEventArgs>()
                  .GroupBy(x => x.FullPath)
                  .Select(x => x.First())
                  .ToList()
                  .ForEach(x => AssetModified?.Invoke(null,new AssetModifiedEventArgs(x.FullPath)));
        }
    }
}
