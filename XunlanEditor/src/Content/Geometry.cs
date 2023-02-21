using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Windows.Documents;
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
    }
}
