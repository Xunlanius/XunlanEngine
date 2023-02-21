using System;
using System.Diagnostics;
using System.Numerics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using XunlanEditor.AssertToolStruct;
using XunlanEditor.DLLInterface;
using XunlanEditor.Editors;
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
        private void On_PrimitiveTypeComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e) => UpdatePrimitiveMesh();
        private void On_Slider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e) => UpdatePrimitiveMesh();
        private void On_ScalarBox_ValueChanged(object sender, RoutedEventArgs e) => UpdatePrimitiveMesh();

        private float Value(ScalarBox scalarBox, float min)
        {
            float.TryParse(scalarBox.Value, out var result);
            return Math.Max(result, min);
        }

        private void UpdatePrimitiveMesh()
        {
            if (!IsInitialized) return;

            PrimitiveMeshType meshType = (PrimitiveMeshType)primitiveTypeComboBox.SelectedItem;
            var info = new PrimitiveMeshInitInfo(meshType);

            switch (meshType)
            {
                case PrimitiveMeshType.Plane:
                    info.SegmentX = (uint)xSliderPlane.Value;
                    info.SegmentZ = (uint)zSliderPlane.Value;
                    info.Size.X = Value(widthScalarBoxPlane, 0.001f);
                    info.Size.Z = Value(lengthScalarBoxPlane, 0.001f);
                    break;
                case PrimitiveMeshType.Cube:
                    break;
                case PrimitiveMeshType.UVSphere:
                    break;
                case PrimitiveMeshType.IcoSphere:
                    break;
                case PrimitiveMeshType.Cylinder:
                    break;
                case PrimitiveMeshType.Capsule:
                    break;
                default:
                    break;
            }

            Geometry geometry = AssertToolAPI.CreateMesh(info);
            Debug.Assert(geometry != null);
            (DataContext as GeometryEditorViewModel).SetAsset(geometry);
        }
    }
}
