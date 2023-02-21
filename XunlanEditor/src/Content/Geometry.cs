using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using XunlanEditor.Utilities;

namespace XunlanEditor.Content
{
    enum PrimitiveMeshType
    {
        Plane,
        Cube,
        UVSphere,
        IcoSphere,
        Cylinder,
        Capsule,
    }

    class Mesh : ViewModelBase
    {
        private int _vertexByteSize;
        public int VertexByteSize
        {
            get => _vertexByteSize;
            set
            {
                if (_vertexByteSize == value) return;

                _vertexByteSize = value;
                OnPropertyChanged(nameof(VertexByteSize));
            }
        }

        private int _numVertices;
        public int NumVertices
        {
            get => _numVertices;
            set
            {
                if (_numVertices == value) return;

                _numVertices = value;
                OnPropertyChanged(nameof(NumVertices));
            }
        }

        private int _indexByteSize;
        public int IndexByteSize
        {
            get => _indexByteSize;
            set
            {
                if (_indexByteSize == value) return;

                _indexByteSize = value;
                OnPropertyChanged(nameof(IndexByteSize));
            }
        }

        private int _numIndices;
        public int NumIndices
        {
            get => _numIndices;
            set
            {
                if (_numIndices == value) return;

                _numIndices = value;
                OnPropertyChanged(nameof(NumIndices));
            }
        }

        public byte[] VerticesBuffer { get; set; }
        public byte[] IndicesBuffer { get; set; }
    }
    
    /// <summary>
    /// All meshes in <see cref="MeshLOD"/> have the same LOD ID
    /// </summary>
    class MeshLOD : ViewModelBase
    {
        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if (_name == value) return;

                _name = value;
                OnPropertyChanged(nameof(Name));
            }
        }

        private float _lodThreshold;
        public float LodThreshold
        {
            get => _lodThreshold;
            set
            {
                if (_lodThreshold == value) return;

                _lodThreshold = value;
                OnPropertyChanged(nameof(LodThreshold));
            }
        }

        public ObservableCollection<Mesh> Meshes { get; } = new ObservableCollection<Mesh>();
    }

    class LodGroup : ViewModelBase
    {
        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if (_name == value) return;

                _name = value;
                OnPropertyChanged(nameof(Name));
            }
        }

        public ObservableCollection<MeshLOD> LODs { get; } = new ObservableCollection<MeshLOD>();
    }

    class GeometryImportSettings : ViewModelBase
    {
        private bool _calculateNormals;
        public bool CalculateNormals
        {
            get => _calculateNormals;
            set
            {
                if (_calculateNormals == value) return;
                _calculateNormals = value;
                OnPropertyChanged(nameof(CalculateNormals));
            }
        }

        private bool _calculateTangents;
        public bool CalculateTangents
        {
            get => _calculateTangents;
            set
            {
                if (_calculateTangents == value) return;
                _calculateTangents = value;
                OnPropertyChanged(nameof(CalculateTangents));
            }
        }

        private float _smoothingAngle;
        public float SmoothingAngle
        {
            get => _smoothingAngle;
            set
            {
                if (_smoothingAngle == value) return;
                _smoothingAngle = value;
                OnPropertyChanged(nameof(SmoothingAngle));
            }
        }

        private bool _reverseHandedness;
        public bool ReverseHandedness
        {
            get => _reverseHandedness;
            set
            {
                if (_reverseHandedness == value) return;
                _reverseHandedness = value;
                OnPropertyChanged(nameof(ReverseHandedness));
            }
        }

        public GeometryImportSettings()
        {
            CalculateNormals = false;
            CalculateTangents = false;
            SmoothingAngle = 178f;
            ReverseHandedness = false;
        }

        public void ToBinary(BinaryWriter writer)
        {
            writer.Write(CalculateNormals);
            writer.Write(CalculateTangents);
            writer.Write(SmoothingAngle);
            writer.Write(ReverseHandedness);
        }
    }

    class Geometry : Asset
    {
        private readonly List<LodGroup> _lodGroups = new List<LodGroup>();
        public LodGroup GetLodGroupAt(int index = 0)
        {
            if (_lodGroups.Count == 0) return null;

            Debug.Assert(index >= 0 && index < _lodGroups.Count);
            return _lodGroups[index];
        }

        public Geometry() : base(AssetType.Mesh) {}

        public void UnpackRawData(byte[] datas)
        {
            Debug.Assert(datas != null);
            Debug.Assert(datas.Length > 0);

            _lodGroups.Clear();

            using BinaryReader reader = new BinaryReader(new MemoryStream(datas));

            // length of scene name
            int nameLen = reader.ReadInt32();
            // skip the scene name
            reader.BaseStream.Position += nameLen;

            int numLODs = reader.ReadInt32();
            Debug.Assert(numLODs > 0);
            for (int i = 0; i < numLODs; ++i)
            {
                // LOD group's name
                int lodGroupNameLen = reader.ReadInt32();
                string lodGroupName;
                if(lodGroupNameLen > 0)
                    lodGroupName = Encoding.UTF8.GetString(reader.ReadBytes(lodGroupNameLen));
                else
                    lodGroupName = $"LOD_{AssetHelper.GetRandomString()}";

                // number of meshes
                int numMeshes = reader.ReadInt32();
                Debug.Assert(numMeshes > 0);
                List<MeshLOD> lods = ReadMeshLods(numMeshes, reader);

                LodGroup lodGroup = new LodGroup() { Name = lodGroupName };
                lods.ForEach(lod => lodGroup.LODs.Add(lod));

                _lodGroups.Add(lodGroup);
            }
        }

        private static List<MeshLOD> ReadMeshLods(int numMeshes, BinaryReader reader)
        {
            List<uint> lodIDs = new List<uint>();
            List<MeshLOD> lodList = new List<MeshLOD>();

            for (int i = 0; i < numMeshes; ++i)
            {
                ReadMesh(reader, lodIDs, lodList);
            }

            return lodList;
        }
        private static void ReadMesh(BinaryReader reader, List<uint> lodIDs, List<MeshLOD> lodList)
        {
            // mesh's name
            int nameLen = reader.ReadInt32();
            string meshName;
            if(nameLen > 0)
                meshName = Encoding.UTF8.GetString(reader.ReadBytes(nameLen));
            else
                meshName = $"Mesh_{AssetHelper.GetRandomString()}";

            Mesh mesh = new Mesh();

            // LOD ID
            uint lodID = reader.ReadUInt32();

            // vertex byte size
            mesh.VertexByteSize = reader.ReadInt32();
            // the number of vertices
            mesh.NumVertices = reader.ReadInt32();
            // index byte size
            mesh.IndexByteSize = reader.ReadInt32();
            // the number of indices
            mesh.NumIndices = reader.ReadInt32();

            // LOD threshold
            float lodThreshold = reader.ReadSingle();

            // vertex buffer
            mesh.VerticesBuffer = reader.ReadBytes(mesh.VertexByteSize * mesh.NumVertices);
            // index buffer
            mesh.IndicesBuffer = reader.ReadBytes(mesh.IndexByteSize * mesh.NumIndices);

            // determine which MeshLOD the mesh should be added to
            MeshLOD meshLOD;
            if (ID.IsValid(lodID) && lodIDs.Contains(lodID))
            {
                meshLOD = lodList[lodIDs.IndexOf(lodID)];
                Debug.Assert(meshLOD != null);
            }
            else
            {
                lodIDs.Add(lodID);
                meshLOD = new MeshLOD() { Name = meshName, LodThreshold = lodID };
                lodList.Add(meshLOD);
            }

            meshLOD.Meshes.Add(mesh);
        }

        public override IEnumerable<string> Save(string filePath)
        {
            List<string> savedFiles = new List<string>();

            if (!_lodGroups.Any())
            {
                Debug.Assert(false);
                return savedFiles;
            }

            string path = Path.GetDirectoryName(filePath) + Path.DirectorySeparatorChar;
            string fileName = Path.GetFileNameWithoutExtension(filePath);

            try
            {
                foreach (LodGroup group in _lodGroups)
                {
                    Debug.Assert(group.LODs.Any());
                    string meshFileName = AssetHelper.SanitizeFileName(path + fileName + "_" + group.LODs[0].Name + AssetFileExtension);

                    Guid = Guid.NewGuid();

                    byte[] data = null;
                    using (var writer = new BinaryWriter(new MemoryStream()))
                    {
                        writer.Write(group.Name);
                        writer.Write(group.LODs.Count);

                        List<byte> hashes = new List<byte>();
                        foreach (MeshLOD lod in group.LODs)
                        {
                            LODToBinary(lod, writer, out byte[] hash);
                            hashes.AddRange(hash);
                        }

                        Hash = AssetHelper.ComputeHash(hashes.ToArray());
                        data = (writer.BaseStream as MemoryStream).ToArray();
                    }

                    Debug.Assert(data?.Length > 0);

                    using (BinaryWriter writer = new BinaryWriter(File.Open(meshFileName, FileMode.Create, FileAccess.Write)))
                    {
                        WriteAssetFileHeader(writer);
                        //ImportSettings.ToBinary(writer);
                        writer.Write(data.Length);
                        writer.Write(data);
                    }

                    savedFiles.Add(meshFileName);
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error, $"Failed to save geometry to [{filePath}]");
                throw;
            }

            return savedFiles;
        }
        private void LODToBinary(MeshLOD lod, BinaryWriter writer, out byte[] hash)
        {
            writer.Write(lod.Name);
            writer.Write(lod.LodThreshold);
            writer.Write(lod.Meshes.Count);

            Int64 meshDataBegin = writer.BaseStream.Position;

            foreach (Mesh mesh in lod.Meshes)
            {
                writer.Write(mesh.VertexByteSize);
                writer.Write(mesh.NumVertices);
                writer.Write(mesh.IndexByteSize);
                writer.Write(mesh.NumIndices);
                writer.Write(mesh.VerticesBuffer);
                writer.Write(mesh.IndicesBuffer);
            }

            Int64 meshDataSize = writer.BaseStream.Position - meshDataBegin;
            Debug.Assert(meshDataSize > 0);

            byte[] buffer = (writer.BaseStream as MemoryStream).ToArray();
            hash = AssetHelper.ComputeHash(buffer, (int)meshDataBegin, (int)meshDataSize);
        }
    }
}
