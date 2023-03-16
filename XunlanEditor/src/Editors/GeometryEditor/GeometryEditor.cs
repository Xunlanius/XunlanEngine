using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Media3D;
using XunlanEditor.Content;

namespace XunlanEditor.Editors
{
    // This class will be obsolete after having the DX12 renderer
    class MeshRendererVertexData : ViewModelBase
    {
        public string Name { get; set; }

        private bool _isHighLighted;
        public bool IsHighLighted
        {
            get => _isHighLighted;
            set
            {
                if(_isHighLighted == value) return;
                _isHighLighted = value;
                OnPropertyChanged(nameof(IsHighLighted));
                OnPropertyChanged(nameof(Diffuse));
            }
        }

        private Brush _specular = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#ff111111"));
        public Brush Specular
        {
            get => _specular;
            set
            {
                if(_specular == value) return;
                _specular = value;
                OnPropertyChanged(nameof(Specular));
            }
        }

        private Brush _diffuse = Brushes.White;
        public Brush Diffuse
        {
            get => IsHighLighted ? Brushes.Pink : _diffuse;
            set
            {
                if(_diffuse == value) return;
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
        public ObservableCollection<MeshRendererVertexData> Meshes { get; } = new ObservableCollection<MeshRendererVertexData>();

        private Vector3D _cameraDir = new Vector3D(0,0,-10);
        public Vector3D CameraDir
        {
            get => _cameraDir;
            set
            {
                if(_cameraDir == value)
                    return;
                _cameraDir = value;
                OnPropertyChanged(nameof(CameraDir));
            }
        }

        private Point3D _cameraPos = new Point3D(0,0,10);
        public Point3D CameraPos
        {
            get => _cameraPos;
            set
            {
                if(_cameraPos == value)
                    return;
                _cameraPos = value;
                CameraDir = new Vector3D(-value.X,-value.Y,-value.Z);
                OnPropertyChanged(nameof(CameraPos));
                OnPropertyChanged(nameof(OffsetCameraPos));
            }
        }

        private Point3D _cameraTarget = new Point3D(0,0,0);
        public Point3D CameraTarget
        {
            get => _cameraTarget;
            set
            {
                if(_cameraTarget == value)
                    return;
                _cameraTarget = value;
                OnPropertyChanged(nameof(CameraTarget));
                OnPropertyChanged(nameof(OffsetCameraPos));
            }
        }

        public Point3D OffsetCameraPos => new Point3D(CameraPos.X + CameraTarget.X,CameraPos.Y + CameraTarget.Y,CameraPos.Z + CameraTarget.Z);

        private Color _keyLight = (Color)ColorConverter.ConvertFromString("#ffaeaeae");
        public Color KeyLight
        {
            get => _keyLight;
            set
            {
                if(_keyLight == value)
                    return;
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
                if(_skyLight == value)
                    return;
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
                if(_groundLight == value)
                    return;
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
                if(_ambientLight == value)
                    return;
                _ambientLight = value;
                OnPropertyChanged(nameof(AmbientLight));
            }
        }

        public MeshRenderer(MeshLOD lod,MeshRenderer oldRenderer)
        {
            Debug.Assert(lod?.Meshes.Any() == true);

            // We need to know the object's bounding box
            double minX, minY, minZ;
            double maxX, maxY, maxZ;
            minX = minY = minZ = double.MaxValue;
            maxX = maxY = maxZ = double.MinValue;

            Vector3D avgNormal = new Vector3D();

            // Unpack the packed normals
            var intervals = 2.0f / ((1 << 16) - 1);

            foreach(var mesh in lod.Meshes)
            {
                var vertexData = new MeshRendererVertexData() { Name = mesh.Name };

                // Vertices
                using(var reader = new BinaryReader(new MemoryStream(mesh.PositionBuffer)))
                for(int i = 0; i < mesh.NumVertices; ++i)
                {
                    // Positions
                    float posX = reader.ReadSingle();
                    float posY = reader.ReadSingle();
                    float posZ = reader.ReadSingle();
                    vertexData.Positions.Add(new Point3D(posX, posY, posZ));

                    // Adjust bounding box
                    minX = Math.Min(minX, posX);
                    minY = Math.Min(minY, posY);
                    minZ = Math.Min(minZ, posZ);
                    maxX = Math.Max(maxX, posX);
                    maxY = Math.Max(maxY, posY);
                    maxZ = Math.Max(maxZ, posZ);
                }

                if (mesh.ElementType.HasFlag(ElementType.Normals))
                {
                    int tSpaceOffset = 0;
                    if (mesh.ElementType.HasFlag(ElementType.Joints)) tSpaceOffset += sizeof(short) * 4;

                    using (var reader = new BinaryReader (new MemoryStream(mesh.ElementBuffer)))
                        for(int i = 0; i < mesh.NumVertices; ++i)
                        {
                            // Normals
                            byte sign = (byte)((reader.ReadUInt32() >> 24) & 0x000000ff);
                            reader.BaseStream.Position += tSpaceOffset;
                            float normalX = reader.ReadUInt16() * intervals - 1.0f;
                            float normalY = reader.ReadUInt16() * intervals - 1.0f;
                            float normalZ = (float)(Math.Sqrt(Math.Clamp(1f - normalX * normalX - normalY * normalY, 0f, 1f)) * ((sign & 2) - 1f));
                            Vector3D normal = new Vector3D(normalX,normalY,normalZ);
                            normal.Normalize();
                            vertexData.Normals.Add(normal);
                            avgNormal += normal;

                            if (mesh.ElementType.HasFlag(ElementType.TSpace))
                            {
                                // Skip tangents
                                reader.BaseStream.Position += sizeof(short) * 2;

                                // UVs
                                float u = reader.ReadSingle();
                                float v = reader.ReadSingle();
                                vertexData.UVs.Add(new Point(u,v));
                            }

                            if (mesh.ElementType.HasFlag(ElementType.Joints) && mesh.ElementType.HasFlag(ElementType.Colors))
                            {
                                reader.BaseStream.Position += 4;
                            }
                        }
                }

                // Indices
                using(var reader = new BinaryReader(new MemoryStream(mesh.IndexBuffer)))
                for(int i = 0;i < mesh.NumIndices;++i)
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
            if(oldRenderer != null)
            {
                CameraPos = oldRenderer.CameraPos;
                CameraTarget = oldRenderer.CameraTarget;

                foreach (var mesh in oldRenderer.Meshes)
                {
                    mesh.IsHighLighted = false;
                }
            }
            else
            {
                // Compute bounding box
                double x = maxX - minX;
                double y = maxY - minY;
                double z = maxZ - minZ;
                double radius = new Vector3D(x,y,z).Length * 1.2;

                if(avgNormal.Length > 0.8)
                {
                    avgNormal.Normalize();
                    avgNormal *= radius;
                    CameraPos = new Point3D(avgNormal.X,avgNormal.Y,avgNormal.Z);
                }
                else
                {
                    CameraPos = new Point3D(x,y * 0.5,radius);
                }

                CameraTarget = new Point3D(minX + x * 0.5,minY + y * 0.5,minZ + z * 0.5);
            }
        }
    }

    class GeometryEditor : ViewModelBase, IAssetEditor
    {
        public Asset Asset { get => Geometry; }

        private Content.Geometry _geometry;
        public Content.Geometry Geometry
        {
            get => _geometry;
            set
            {
                if(_geometry == value)
                    return;
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
                if(_meshRenderer == value)
                    return;
                _meshRenderer = value;

                var lods = Geometry.GetLodGroupAt().LODs;
                MaxLOD = lods.Count > 0 ? lods.Count - 1 : 0;

                if(lods.Count > 0)
                {
                    MeshRenderer.PropertyChanged += (sender,e) =>
                    {
                        if(e.PropertyName == nameof(MeshRenderer.OffsetCameraPos) && AutoLOD)
                            ComputeLOD(lods);
                    };
                }

                OnPropertyChanged(nameof(MeshRenderer));
            }
        }

        private int _maxLOD;
        public int MaxLOD
        {
            get => _maxLOD;
            private set
            {
                if(_maxLOD == value)
                    return;
                MaxLOD = value;
                OnPropertyChanged(nameof(MaxLOD));
            }
        }

        private int _currLOD;
        public int CurrLOD
        {
            get => _currLOD;
            set
            {
                var lods = Geometry.GetLodGroupAt().LODs;
                value = Math.Clamp(value,0,lods.Count - 1);

                if(_currLOD == value)
                    return;
                _currLOD = value;
                OnPropertyChanged(nameof(CurrLOD));

                MeshRenderer = new MeshRenderer(lods[value],MeshRenderer);
            }
        }

        private bool _autoLOD = true;
        public bool AutoLOD
        {
            get => _autoLOD;
            set
            {
                if(_autoLOD == value)
                    return;
                _autoLOD = value;
                OnPropertyChanged(nameof(AutoLOD));
            }
        }

        private void ComputeLOD(IList<MeshLOD> lods)
        {
            if(!AutoLOD) return;

            Point3D pos = MeshRenderer.OffsetCameraPos;
            double distance = new Vector3D(pos.X,pos.Y,pos.Z).Length;

            for(int i = MaxLOD;i >= 0;--i)
            {
                if(lods[i].LodThreshold < distance)
                {
                    CurrLOD = i;
                    break;
                }
            }
        }

        public void SetAsset(Asset asset)
        {
            Debug.Assert(asset is Content.Geometry);

            if(asset is Content.Geometry geometry)
            {
                _geometry = geometry;

                int numLODs = geometry.GetLodGroupAt().LODs.Count;
                if(CurrLOD >= numLODs)
                {
                    CurrLOD = numLODs - 1;
                }
                else
                {
                    MeshRenderer = new MeshRenderer(Geometry.GetLodGroupAt().LODs[0],MeshRenderer);
                }
            }
        }
        public async void SetAsset(AssetInfo info)
        {
            try
            {
                Debug.Assert(info != null && File.Exists(info.FullPath));

                var geometry = new Content.Geometry();
                await Task.Run(() => geometry.Load(info.FullPath));
                SetAsset(geometry);
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
        }
    }
}
