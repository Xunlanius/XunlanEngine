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


    abstract class Asset : ViewModelBase
    {
        public static string AssetFileExtension { get => ".asset"; }

        public AssetType Type { get; set; }
        public Guid Guid { get; protected set; } = Guid.NewGuid();
        public DateTime ImportDate { get; protected set; }

        /// <summary>
        /// Used to compare whether two <see cref="Asset"/> is the same
        /// </summary>
        public byte[] Hash { get; protected set; }
        public string SoursePath { get; protected set; }

        public Asset(AssetType type)
        {
            Debug.Assert(type != AssetType.Unknown);
            Type = type;
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

            if (Hash?.Length > 0)
            {
                writer.Write(Hash.Length);
                writer.Write(Hash);
            }
            else
            {
                writer.Write(0);
            }

            writer.Write(SoursePath ?? "");
        }

        public abstract IEnumerable<string> Save(string filePath);
    }
}
