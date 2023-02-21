using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace XunlanEditor.Editors
{
    /// <summary>
    /// GeometryView.xaml 的交互逻辑
    /// </summary>
    public partial class GeometryView : UserControl
    {
        public GeometryView()
        {
            InitializeComponent();

            // MeshRenderer will changed frequently
            DataContextChanged += (s, e) => SetGeometry();
        }

        public void SetGeometry(int index = -1)
        {
            if (!(DataContext is MeshRenderer vm)) return;

            if (vm.Meshes.Any() && viewport.Children.Count == 2)
            {
                viewport.Children.RemoveAt(1);
            }

            int meshIndex = 0;
            Model3DGroup modelGroup = new Model3DGroup();
            foreach (var mesh in vm.Meshes)
            {
                // Skip over meshed that we don't want to display
                if (index != -1 && meshIndex != index)
                {
                    ++meshIndex;
                    continue;
                }

                MeshGeometry3D mesh3D = new MeshGeometry3D()
                {
                    Positions = mesh.Positions,
                    Normals = mesh.Normals,
                    TriangleIndices = mesh.Indices,
                    TextureCoordinates = mesh.UVs,
                };

                DiffuseMaterial diffuse = new DiffuseMaterial(mesh.Diffuse);
                SpecularMaterial specular = new SpecularMaterial(mesh.Specular, 50);
                MaterialGroup matGroup = new MaterialGroup();
                matGroup.Children.Add(diffuse);
                matGroup.Children.Add(specular);

                GeometryModel3D model = new GeometryModel3D(mesh3D, matGroup);
                modelGroup.Children.Add(model);

                Binding binding = new Binding(nameof(mesh.Diffuse)) { Source = mesh };
                BindingOperations.SetBinding(diffuse, DiffuseMaterial.BrushProperty, binding);

                if (meshIndex == index) break;
            }

            ModelVisual3D visual = new ModelVisual3D() { Content = modelGroup };
            viewport.Children.Add(visual);
        }

        private Point _clickedPos;
        private bool _captureLeft = false;
        private bool _captureRight = false;

        private void OnGrid_Mouse_LBD(object sender, MouseButtonEventArgs e)
        {
            _clickedPos = e.GetPosition(this);
            _captureLeft = true;
            Mouse.Capture(sender as UIElement);
        }
        private void OnGrid_Mouse_LBU(object sender, MouseButtonEventArgs e)
        {
            _captureLeft = false;
            if (!_captureRight)
                Mouse.Capture(null);
        }
        private void OnGrid_Mouse_RBD(object sender, MouseButtonEventArgs e)
        {
            _clickedPos = e.GetPosition(this);
            _captureRight = true;
            Mouse.Capture(sender as UIElement);
        }
        private void OnGrid_Mouse_RBU(object sender, MouseButtonEventArgs e)
        {
            _captureRight = false;
            if (!_captureLeft)
                Mouse.Capture(null);
        }
        private void OnGrid_MouseMove(object sender, MouseEventArgs e)
        {
            if (!_captureLeft && !_captureRight) return;

            var currPos = e.GetPosition(this);
            var distance = currPos - _clickedPos;

            if (_captureLeft && !_captureRight)
            {
                //MoveCamera(distance.X, distance.Y, 0);
            }
            else if (!_captureLeft && _captureRight)
            {
                MeshRenderer renderer = DataContext as MeshRenderer;
                Point3D cameraPos = renderer.CameraPos;
                double yOffset = distance.Y * 0.001 * Math.Sqrt(cameraPos.X * cameraPos.X + cameraPos.Y * cameraPos.Y);
                renderer.CameraTarget = new Point3D(renderer.CameraTarget.X, renderer.CameraTarget.Y + yOffset, renderer.CameraTarget.Z);
            }
        }
        private void OnGrid_MouseWheel(object sender, MouseWheelEventArgs e)
        {

        }
    }
}
