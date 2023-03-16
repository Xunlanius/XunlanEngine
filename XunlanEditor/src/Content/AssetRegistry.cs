using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using XunlanEditor.src.Content;

namespace XunlanEditor.Content
{
    static class AssetRegistry
    {
        private static readonly Dictionary<string,AssetInfo> _assetDict = new Dictionary<string,AssetInfo>();

        private static readonly ObservableCollection<AssetInfo> _assets = new ObservableCollection<AssetInfo>();
        public static ReadOnlyObservableCollection<AssetInfo> Assets { get; } = new ReadOnlyObservableCollection<AssetInfo>(_assets);

        public static AssetInfo GetAssetInfo(string file)
        {
            return _assetDict.ContainsKey(file) ? _assetDict[file] : null;
        }
        public static AssetInfo GetAssetInfo(Guid guid)
        {
            return _assets.FirstOrDefault(x => x.Guid == guid);
        }

        public static void Reset(string assetFolder)
        {
            AssetWatcher.AssetModified -= OnAssetModified;

            _assetDict.Clear();
            _assets.Clear();

            Debug.Assert(Directory.Exists(assetFolder));
            RegisterAllAssets(assetFolder);

            AssetWatcher.AssetModified += OnAssetModified;
        }

        private static void RegisterAllAssets(string file)
        {
            Debug.Assert(Directory.Exists(file));
            foreach(var entry in Directory.GetFileSystemEntries(file))
            {
                if(AssetHelper.IsDirectory(entry))
                    RegisterAllAssets(entry);
                else
                    RegisterAsset(entry);
            }
        }
        private static void RegisterAsset(string file)
        {
            try
            {
                AssetInfo assetInfo = Asset.GetAssetInfo(file);
                FileInfo fileInfo = new FileInfo(file);

                // Update the dictionary
                if(!_assetDict.ContainsKey(file) || _assetDict[file].RegisterDate < fileInfo.LastWriteTime)
                {
                    assetInfo.RegisterDate = DateTime.Now;
                    Debug.Assert(assetInfo != null);
                    _assetDict[file] = assetInfo;
                    _assets.Add(assetInfo);
                }
            }
            catch(Exception ex) { Debug.WriteLine(ex.Message); }
        }
        private static void UnregisterAsset(string file)
        {
            if(_assetDict.ContainsKey(file))
            {
                _assets.Remove(_assetDict[file]);
                _assetDict.Remove(file);
            }
        }

        private static void OnAssetModified(object sender,AssetModifiedEventArgs e)
        {
            if(AssetHelper.IsDirectory(e.FullPath))
                RegisterAllAssets(e.FullPath);
            else if(File.Exists(e.FullPath))
                RegisterAsset(e.FullPath);

            _assets.Where(x => !File.Exists(x.FullPath)).ToList().ForEach(x => UnregisterAsset(x.FullPath));
        }
    }
}
