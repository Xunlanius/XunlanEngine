using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Media3D;

namespace XunlanEditor.Editors
{
    // This class will be obsolete after having the DX12 renderer
    class MeshRendererVertexData : ViewModelBase
    {
        private Brush _specular = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#ff111111"));
        public Brush Specular
        {
            get => _specular;
            set
            {
                if (_specular == value) return;
                _specular = value;
                OnPropertyChanged(nameof(Specular));
            }
        }

        private Brush _diffuse = Brushes.White;
        public Brush Diffuse
        {
            get => _diffuse;
            set
            {
                if (_diffuse == value) return;
                _diffuse = value;
                OnPropertyChanged(nameof(Diffuse));
            }
        }

        public Point3DCollection Positions { get; } = new Point3DCollection();
        public Vector3DCollection Normals { get; } = new Vector3DCollection();
        public PointCollection UVs { get; } = new PointCollection();
        public Int32Collection Indices { get; } = new Int32Collection();
    }

    // This class will be obsolete after having the DX12 renderer
    class MeshRenderer : ViewModelBase
    {
        public ObservableCollection<MeshRendererVertexData> Meshes = new ObservableCollection<MeshRendererVertexData>();

        private Vector3D _cameraDir = new Vector3D(0, 0, -10);
        public Vector3D CameraDir
        {
            get => _cameraDir;
            set
            {
                if (_cameraDir == value) return;
                _cameraDir = value;
                OnPropertyChanged(nameof(CameraDir));
            }
        }

        private Point3D _cameraPos = new Point3D(0, 0, 10);
        public Point3D CameraPos
        {
            get => _cameraPos;
            set
            {
                if (_cameraPos == value) return;
                _cameraPos = value;
                CameraDir = new Vector3D(-value.X, -value.Y, -value.Z);
                OnPropertyChanged(nameof(CameraPos));
                OnPropertyChanged(nameof(OffsetCameraPos));
            }
        }

        private Point3D _cameraTarget = new Point3D(0, 0, 0);
        public Point3D CameraTarget
        {
            get => _cameraTarget;
            set
            {
                if (_cameraTarget == value) return;
                _cameraTarget = value;
                OnPropertyChanged(nameof(CameraTarget));
                OnPropertyChanged(nameof(OffsetCameraPos));
            }
        }

        public Point3D OffsetCameraPos => new Point3D(CameraPos.X + CameraTarget.X, CameraPos.Y + CameraTarget.Y, CameraPos.Z + CameraTarget.Z);

        private Color _keyLight = (Color)ColorConverter.ConvertFromString("#ffaeaeae");
        public Color KeyLight
        {
            get => _keyLight;
            set
            {
                if (_keyLight == value) return;
                _keyLight = value;
                OnPropertyChanged(nameof(KeyLight));
            }
        }

        private Color _skyLight = (Color)ColorConverter.ConvertFromString("#ff111b30");
        public Color SkyLight
        {
            get => _skyLight;
            set
            {
                if (_skyLight == value) return;
                _skyLight = value;
                OnPropertyChanged(nameof(SkyLight));
            }
        }

        private Color _groundLight = (Color)ColorConverter.ConvertFromString("#ff3f2f1e");
        public Color GroundLight
        {
            get => _groundLight;
            set
            {
                if (_groundLight == value) return;
                _groundLight = value;
                OnPropertyChanged(nameof(GroundLight));
            }
        }

        private Color _ambientLight = (Color)ColorConverter.ConvertFromString("#ff3b3b3b");
        public Color AmbientLight
        {
            get => _ambientLight;
            set
            {
                if (_ambientLight == value) return;
                _ambientLight = value;
                OnPropertyChanged(nameof(AmbientLight));
            }
        }

        public MeshRenderer(Content.MeshLOD lod, MeshRenderer old)
        {
            Debug.Assert(lod?.Meshes.Any() == true);

            int offset = lod.Meshes[0].VertexByteSize - 3 * sizeof(float) - 4 * sizeof(byte) - 2 * sizeof(short);

            // We need to know the object's bounding box
            double minX, minY, minZ;
            double maxX, maxY, maxZ;
            minX = minY = minZ = double.MaxValue;
            maxX = maxY = maxZ = double.MinValue;

            Vector3D avgNormal = new Vector3D();

            // Unpack the packed normals
            var intervals = 2.0f / ((1 << 16) - 1);

            foreach (var mesh in lod.Meshes)
            {
                var vertexData = new MeshRendererVertexData();

                // Vertices
                using (var reader = new BinaryReader(new MemoryStream(mesh.VerticesBuffer)))
                    for (int i = 0; i < mesh.NumVertices; ++i)
                    {
                        // Positions
                        float posX = reader.ReadSingle();
                        float posY = reader.ReadSingle();
                        float posZ = reader.ReadSingle();
                        byte sign = (byte)((reader.ReadUInt32() >> 24) & 0x000000ff);
                        vertexData.Positions.Add(new Point3D(posX, posY, posZ));

                        // Adjust bounding box
                        minX = Math.Min(minX, posX);
                        minY = Math.Min(minY, posY);
                        minZ = Math.Min(minZ, posZ);
                        maxX = Math.Max(maxX, posX);
                        maxY = Math.Max(maxY, posY);
                        maxZ = Math.Max(maxZ, posZ);

                        // Normals
                        float normalX = reader.ReadUInt16() * intervals - 1.0f;
                        float normalY = reader.ReadUInt16() * intervals - 1.0f;
                        float normalZ = (float)(Math.Sqrt(Math.Clamp(1f - normalX * normalX - normalY * normalY, 0f, 1f)) * ((sign & 2) - 1f));
                        Vector3D normal = new Vector3D(normalX, normalY, normalZ);
                        normal.Normalize();
                        vertexData.Normals.Add(normal);
                        avgNormal += normal;

                        // Skip tangents
                        reader.BaseStream.Position += 2 * sizeof(UInt16);

                        // UVs
                        float u = reader.ReadSingle();
                        float v = reader.ReadSingle();
                        vertexData.UVs.Add(new Point(u, v));
                    }

                // Indices
                using (var reader = new BinaryReader(new MemoryStream(mesh.IndicesBuffer)))
                    for (int i = 0; i < mesh.NumIndices; ++i)
                    {
                        vertexData.Indices.Add(reader.ReadInt32());
                    }

                vertexData.Positions.Freeze();
                vertexData.Normals.Freeze();
                vertexData.UVs.Freeze();
                vertexData.Indices.Freeze();
                Meshes.Add(vertexData);
            }

            // Set camera position and target
            if (old != null)
            {
                CameraPos = old.CameraPos;
                CameraTarget = old.CameraTarget;
            }
            else
            {
                // Compute bounding box
                double x = maxX - minX;
                double y = maxY - minY;
                double z = maxZ - minZ;
                double radius = new Vector3D(x, y, z).Length * 1.2;

                if (avgNormal.Length > 0.8)
                {
                    avgNormal.Normalize();
                    avgNormal *= radius;
                    CameraPos = new Point3D(avgNormal.X, avgNormal.Y, avgNormal.Z);
                }
                else
                {
                    CameraPos = new Point3D(x, y * 0.5, radius);
                }

                CameraTarget = new Point3D(minX + x * 0.5, minY + y * 0.5, minZ + z * 0.5);
            }
        }
    }

    class GeometryEditorViewModel : ViewModelBase, IAssetEditor
    {
        public Content.Asset Asset { get => Geometry; }

        private Content.Geometry _geometry;
        public Content.Geometry Geometry
        {
            get => _geometry;
            set
            {
                if (_geometry == value) return;

                _geometry = value;
                OnPropertyChanged(nameof(Geometry));
            }
        }

        private MeshRenderer _meshRenderer;
        public MeshRenderer MeshRenderer
        {
            get => _meshRenderer;
            set
            {
                if (_meshRenderer == value) return;
                _meshRenderer = value;
                OnPropertyChanged(nameof(MeshRenderer));
            }
        }

        public void SetAsset(Content.Asset asset)
        {
            Debug.Assert(asset is Content.Geometry);
            if (asset is Content.Geometry geometry)
            {
                _geometry = geometry;
                MeshRenderer = new MeshRenderer(Geometry.GetLodGroupAt().LODs[0], MeshRenderer);
            }
        }
    }
}
