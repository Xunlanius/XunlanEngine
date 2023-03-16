using Microsoft.Win32;
using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using XunlanEditor.AssetToolStruct;
using XunlanEditor.DLLInterface;
using XunlanEditor.Editors;
using XunlanEditor.GameProject;
using XunlanEditor.Utilities.Controls;

namespace XunlanEditor.Content
{
    /// <summary>
    /// PrimitiveMeshDiag.xaml 的交互逻辑
    /// </summary>
    public partial class PrimitiveMeshDiag : Window
    {
        public PrimitiveMeshDiag()
        {
            InitializeComponent();
            Loaded += On_PrimitiveMeshDiag_Loaded;
        }

        private void On_PrimitiveMeshDiag_Loaded(object sender,RoutedEventArgs e) => UpdatePrimitiveMesh();
        private void On_PrimitiveTypeComboBox_SelectionChanged(object sender,SelectionChangedEventArgs e) => UpdatePrimitiveMesh();
        private void On_Slider_ValueChanged(object sender,RoutedPropertyChangedEventArgs<double> e) => UpdatePrimitiveMesh();
        private void On_ScalarBox_ValueChanged(object sender,RoutedEventArgs e) => UpdatePrimitiveMesh();

        private void OnSave_Button_Click(object sender,RoutedEventArgs e)
        {
            var dlg = new SaveFileDialog()
            {
                InitialDirectory = Project.CurrProject.AssetsDirPath,
                Filter = "Asset file (*.asset)|*.asset",
            };

            if(dlg.ShowDialog() == true)
            {
                Debug.Assert(!string.IsNullOrEmpty(dlg.FileName));
                var asset = (DataContext as IAssetEditor).Asset;
                Debug.Assert(asset != null);
                asset.Save(dlg.FileName);
            }
        }

        private float Value(ScalarBox scalarBox,float min)
        {
            float.TryParse(scalarBox.Value,out var result);
            return Math.Max(result,min);
        }

        private void UpdatePrimitiveMesh()
        {
            if(!IsInitialized)
                return;

            PrimitiveMeshType meshType = (PrimitiveMeshType)primitiveTypeComboBox.SelectedItem;
            var info = new PrimitiveMeshInitInfo(meshType);

            switch(meshType)
            {
                case PrimitiveMeshType.Plane:
                    info.SegmentX = (uint)xSliderPlane.Value;
                    info.SegmentZ = (uint)zSliderPlane.Value;
                    info.Size.X = Value(widthScalarBoxPlane,0.001f);
                    info.Size.Z = Value(lengthScalarBoxPlane,0.001f);
                    break;
                case PrimitiveMeshType.Cube:
                    return;
                case PrimitiveMeshType.UVSphere:
                    return;
                case PrimitiveMeshType.IcoSphere:
                    return;
                case PrimitiveMeshType.Cylinder:
                    return;
                case PrimitiveMeshType.Capsule:
                    return;
                default:
                    return;
            }

            Geometry geometry = new Geometry();
            AssetToolAPI.CreatePrimitiveMesh(geometry,info);
            (DataContext as GeometryEditor).SetAsset(geometry);
        }
    }
}
