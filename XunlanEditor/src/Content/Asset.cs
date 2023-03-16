using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

namespace XunlanEditor.Content
{
    enum AssetType
    {
        Unknown,
        Mesh,
        Texture,
        Material,
        Animation,
        Audio,
        Skeleton,
    }

    sealed class AssetInfo
    {
        public AssetType Type
        {
            get; set;
        }
        public Guid Guid
        {
            get; set;
        }
        public DateTime RegisterDate
        {
            get; set;
        }
        public DateTime ImportDate
        {
            get; set;
        }
        public byte[] Hash
        {
            get; set;
        }

        public string FullPath
        {
            get; set;
        }
        public string FileName
        {
            get => Path.GetFileNameWithoutExtension(FullPath);
        }
    }

    abstract class Asset : ViewModelBase
    {
        public static string AssetFileExtension
        {
            get => ".asset";
        }

        public AssetType Type
        {
            get; set;
        }
        public Guid Guid { get; protected set; } = Guid.NewGuid();
        public DateTime ImportDate
        {
            get; protected set;
        }

        /// <summary>
        /// Used to compare if two <see cref="Asset"/> is the same
        /// </summary>
        public byte[] Hash
        {
            get; protected set;
        }

        public string _filePath;
        public string FilePath
        {
            get => _filePath;
            set
            {
                if(_filePath == value)
                    return;
                _filePath = value;
                OnPropertyChanged(nameof(FilePath));
                OnPropertyChanged(nameof(FileName));
            }
        }
        public string FileName
        {
            get => Path.GetFileNameWithoutExtension(FilePath);
        }

        public Asset(AssetType type)
        {
            Debug.Assert(type != AssetType.Unknown);
            Type = type;
        }

        public static AssetInfo GetAssetInfo(string file)
        {
            Debug.Assert(File.Exists(file) && Path.GetExtension(file) == AssetFileExtension);

            try
            {
                using BinaryReader reader = new BinaryReader(File.Open(file,FileMode.Open,FileAccess.Read));
                AssetInfo info = GetAssetInfo(reader);
                info.FullPath = file;
                return info;
            }
            catch(Exception ex) { Debug.WriteLine(ex.Message); }

            return null;
        }
        private static AssetInfo GetAssetInfo(BinaryReader reader)
        {
            AssetInfo info = new AssetInfo();
            reader.BaseStream.Position = 0;

            info.Type = (AssetType)reader.ReadInt32();
            int idLen = reader.ReadInt32();
            info.Guid = new Guid(reader.ReadBytes(idLen));
            info.ImportDate = DateTime.FromBinary(reader.ReadInt64());
            int hashLen = reader.ReadInt32();
            if(hashLen > 0)
            {
                info.Hash = reader.ReadBytes(hashLen);
            }

            return info;
        }

        protected void WriteAssetFileHeader(BinaryWriter writer)
        {
            byte[] id = Guid.ToByteArray();
            Int64 importDate = DateTime.Now.ToBinary();

            writer.BaseStream.Position = 0;

            writer.Write((int)Type);
            writer.Write(id.Length);
            writer.Write(id);
            writer.Write(importDate);

            if(Hash?.Length > 0)
            {
                writer.Write(Hash.Length);
                writer.Write(Hash);
            }
            else
                writer.Write(0);
        }
        protected void ReadAssetFileHeader(BinaryReader reader)
        {
            AssetInfo info = GetAssetInfo(reader);

            Debug.Assert(Type == info.Type);
            Guid = info.Guid;
            ImportDate = info.ImportDate;
            Hash = info.Hash;
            FilePath = info.FullPath;
        }

        public abstract void Import(string filePath);
        public abstract void Load(string filePath);
        public abstract IEnumerable<string> Save(string filePath);
    }
}
