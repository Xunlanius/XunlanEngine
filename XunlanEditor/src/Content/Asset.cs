using System.Diagnostics;

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
        public AssetType Type { get; set; }

        public Asset(AssetType type)
        {
            Debug.Assert(type != AssetType.Unknown);
            Type = type;
        }
    }
}
