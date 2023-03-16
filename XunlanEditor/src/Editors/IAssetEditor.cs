using XunlanEditor.Content;

namespace XunlanEditor.Editors
{
    interface IAssetEditor
    {
        Asset Asset
        {
            get;
        }
        void SetAsset(AssetInfo info);
    }
}