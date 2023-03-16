using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
using XunlanEditor.DLLInterface;
using XunlanEditor.GameProject;
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

    enum ElementType
    {
        Position = 0,
        Normals = 1,
        TSpace = 3,
        Colors = 4,
        Joints = 8,
    }

    class Mesh : ViewModelBase
    {
        public static int PositionSize = sizeof(float) * 3;

        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if(_name == value) return;
                _name = value;
                OnPropertyChanged(nameof(Name));
            }
        }

        private int _numVertices;
        public int NumVertices
        {
            get => _numVertices;
            set
            {
                if(_numVertices == value) return;
                _numVertices = value;
                OnPropertyChanged(nameof(NumVertices));
            }
        }

        private int _numIndices;
        public int NumIndices
        {
            get => _numIndices;
            set
            {
                if(_numIndices == value) return;
                _numIndices = value;
                OnPropertyChanged(nameof(NumIndices));
            }
        }

        public ElementType ElementType { get; set; }

        private int _elementSize;
        public int ElementSize
        {
            get => _elementSize;
            set
            {
                if(_elementSize == value) return;
                _elementSize = value;
                OnPropertyChanged(nameof(ElementSize));
            }
        }

        public byte[] PositionBuffer { get; set; }
        public byte[] IndexBuffer { get; set; }
        public byte[] ElementBuffer { get; set; }
    }

    /// <summary>
    /// All <see cref="Mesh"/>es in a <see cref="MeshLOD"/> have the same LOD ID
    /// </summary>
    class MeshLOD : ViewModelBase
    {
        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if(_name == value) return;
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
                if(_lodThreshold == value) return;
                _lodThreshold = value;
                OnPropertyChanged(nameof(LodThreshold));
            }
        }

        public ObservableCollection<Mesh> Meshes { get; } = new ObservableCollection<Mesh>();
    }

    class LODGroup : ViewModelBase
    {
        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if(_name == value) return;
                _name = value;
                OnPropertyChanged(nameof(Name));
            }
        }

        public ObservableCollection<MeshLOD> LODs { get; } = new ObservableCollection<MeshLOD>();
    }

    class GeometryImportSettings : ViewModelBase
    {
        private float _smoothingAngle;
        public float SmoothingAngle
        {
            get => _smoothingAngle;
            set
            {
                if(_smoothingAngle == value) return;
                _smoothingAngle = value;
                OnPropertyChanged(nameof(SmoothingAngle));
            }
        }

        private bool _calculateNormals;
        public bool CalculateNormals
        {
            get => _calculateNormals;
            set
            {
                if(_calculateNormals == value) return;
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
                if(_calculateTangents == value) return;
                _calculateTangents = value;
                OnPropertyChanged(nameof(CalculateTangents));
            }
        }

        private bool _reverseHandedness;
        public bool ReverseHandedness
        {
            get => _reverseHandedness;
            set
            {
                if(_reverseHandedness == value) return;
                _reverseHandedness = value;
                OnPropertyChanged(nameof(ReverseHandedness));
            }
        }

        private bool _importEmbededTextures;
        public bool ImportEmbededTextures
        {
            get => _importEmbededTextures;
            set
            {
                if(_importEmbededTextures == value) return;
                _importEmbededTextures = value;
                OnPropertyChanged(nameof(ImportEmbededTextures));
            }
        }

        private bool _importAnimations;
        public bool ImportAnimations
        {
            get => _importAnimations;
            set
            {
                if(_importAnimations == value) return;
                _importAnimations = value;
                OnPropertyChanged(nameof(ImportAnimations));
            }
        }

        public GeometryImportSettings()
        {
            SmoothingAngle = 178f;
            CalculateNormals = false;
            CalculateTangents = true;
            ReverseHandedness = false;
            ImportEmbededTextures = true;
            ImportAnimations = true;
        }

        public void ToBinary(BinaryWriter writer)
        {
            writer.Write(SmoothingAngle);
            writer.Write(CalculateNormals);
            writer.Write(CalculateTangents);
            writer.Write(ReverseHandedness);
            writer.Write(ImportEmbededTextures);
            writer.Write(ImportAnimations);
        }

        public void FromBinary(BinaryReader reader)
        {
            SmoothingAngle = reader.ReadSingle();
            CalculateNormals = reader.ReadBoolean();
            CalculateTangents = reader.ReadBoolean();
            ReverseHandedness = reader.ReadBoolean();
            ImportEmbededTextures = reader.ReadBoolean();
            ImportAnimations = reader.ReadBoolean();
        }
    }

    class Geometry : Asset
    {
        private readonly object _lock = new object();

        private readonly List<LODGroup> _lodGroups = new List<LODGroup>();

        public GeometryImportSettings ImportSettings { get; } = new GeometryImportSettings();

        public Geometry() : base(AssetType.Mesh) { }

        public LODGroup GetLodGroupAt(int index = 0)
        {
            if(_lodGroups.Count == 0) return null;

            Debug.Assert(index >= 0 && index < _lodGroups.Count);
            return (index >= 0 && index < _lodGroups.Count) ? _lodGroups[index] : null;
        }

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
            for(int i = 0;i < numLODs;++i)
            {
                // LOD group's name
                int lodGroupNameLen = reader.ReadInt32();
                string lodGroupName;
                if(lodGroupNameLen > 0) lodGroupName = Encoding.UTF8.GetString(reader.ReadBytes(lodGroupNameLen));
                else lodGroupName = $"LOD_{AssetHelper.GetRandomString()}";

                // number of meshes
                int numMeshes = reader.ReadInt32();
                Debug.Assert(numMeshes > 0);
                List<MeshLOD> lods = ReadMeshLods(numMeshes,reader);

                LODGroup lodGroup = new LODGroup() { Name = lodGroupName };
                lods.ForEach(lod => lodGroup.LODs.Add(lod));

                _lodGroups.Add(lodGroup);
            }
        }

        private static List<MeshLOD> ReadMeshLods(int numMeshes, BinaryReader reader)
        {
            List<uint> lodIDs = new List<uint>();
            List<MeshLOD> lodList = new List<MeshLOD>();

            for(int i = 0;i < numMeshes;++i)
            {
                ReadMesh(reader,lodIDs,lodList);
            }

            return lodList;
        }
        private static void ReadMesh(BinaryReader reader, List<uint> lodIDs, List<MeshLOD> lodList)
        {
            // mesh's name
            int nameLen = reader.ReadInt32();
            string meshName;
            if(nameLen > 0) meshName = Encoding.UTF8.GetString(reader.ReadBytes(nameLen));
            else meshName = $"Mesh_{AssetHelper.GetRandomString()}";

            Mesh mesh = new Mesh() { Name = meshName };

            // Number of vertices
            mesh.NumVertices = reader.ReadInt32();

            // Number of indices
            mesh.NumIndices = reader.ReadInt32();

            // Element type
            mesh.ElementType = (ElementType)reader.ReadInt32();
            // Element size
            mesh.ElementSize = reader.ReadInt32();

            // LOD ID
            uint lodID = reader.ReadUInt32();
            // LOD threshold
            float lodThreshold = reader.ReadSingle();

            // Vertex buffer
            mesh.PositionBuffer = reader.ReadBytes(Mesh.PositionSize * mesh.NumVertices);
            // Index buffer
            mesh.IndexBuffer = reader.ReadBytes(sizeof(uint) * mesh.NumIndices);
            // Element buffer
            mesh.ElementBuffer = reader.ReadBytes(mesh.ElementSize * mesh.NumVertices);

            // Determine which MeshLOD the mesh should be added to
            MeshLOD meshLOD;
            if(ID.IsValid(lodID) && lodIDs.Contains(lodID))
            {
                meshLOD = lodList[lodIDs.IndexOf(lodID)];
                Debug.Assert(meshLOD != null);
            }
            else
            {
                lodIDs.Add(lodID);
                meshLOD = new MeshLOD() { Name = meshName, LodThreshold = lodThreshold };
                lodList.Add(meshLOD);
            }

            meshLOD.Meshes.Add(mesh);
        }

        public override IEnumerable<string> Save(string filePath)
        {
            List<string> savedFiles = new List<string>();

            if(!_lodGroups.Any()) return savedFiles;

            string path = Path.GetDirectoryName(filePath) + Path.DirectorySeparatorChar;
            string fileName = Path.GetFileNameWithoutExtension(filePath);

            try
            {
                foreach(LODGroup group in _lodGroups)
                {
                    Debug.Assert(group.LODs.Any());
                    string meshFileName = path + fileName;
                    if(_lodGroups.Count == 1) meshFileName = AssetHelper.SanitizeFileName(meshFileName + AssetFileExtension);
                    else
                    {
                        meshFileName = meshFileName + "_";
                        if(group.LODs.Count == 1) meshFileName = AssetHelper.SanitizeFileName(meshFileName + group.Name + AssetFileExtension);
                        else meshFileName = AssetHelper.SanitizeFileName(meshFileName + group.LODs[0].Name + AssetFileExtension);
                    }

                    Guid = Guid.NewGuid();

                    byte[] data = null;
                    using(var writer = new BinaryWriter(new MemoryStream()))
                    {
                        writer.Write(group.Name);
                        writer.Write(group.LODs.Count);

                        List<byte> hashes = new List<byte>();
                        foreach(MeshLOD lod in group.LODs)
                        {
                            LODToBinary(lod,writer,out byte[] hash);
                            hashes.AddRange(hash);
                        }

                        Hash = AssetHelper.ComputeHash(hashes.ToArray());
                        data = (writer.BaseStream as MemoryStream).ToArray();
                    }

                    Debug.Assert(data?.Length > 0);

                    // Write .asset file
                    using(BinaryWriter writer = new BinaryWriter(File.Open(meshFileName,FileMode.Create,FileAccess.Write)))
                    {
                        WriteAssetFileHeader(writer);
                        ImportSettings.ToBinary(writer);
                        writer.Write(data.Length);
                        writer.Write(data);
                    }

                    savedFiles.Add(meshFileName);

                    Logger.LogMessage(MsgType.Info,$"Successfully saved geometry to [{meshFileName}].");
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error,$"Failed to save geometry to [{filePath}].");
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

            foreach(Mesh mesh in lod.Meshes)
            {
                writer.Write(mesh.Name);
                writer.Write(mesh.NumVertices);
                writer.Write(mesh.NumIndices);
                writer.Write((int)mesh.ElementType);
                writer.Write(mesh.ElementSize);
                writer.Write(mesh.PositionBuffer);
                writer.Write(mesh.IndexBuffer);
                writer.Write(mesh.ElementBuffer);
            }

            Int64 meshDataSize = writer.BaseStream.Position - meshDataBegin;
            Debug.Assert(meshDataSize > 0);

            byte[] buffer = (writer.BaseStream as MemoryStream).ToArray();
            hash = AssetHelper.ComputeHash(buffer,(int)meshDataBegin,(int)meshDataSize);
        }

        public override void Load(string filePath)
        {
            Debug.Assert(File.Exists(filePath));
            Debug.Assert(Path.GetExtension(filePath) == AssetFileExtension);

            try
            {
                byte[] data = null;
                using(var reader = new BinaryReader(File.Open(filePath,FileMode.Open,FileAccess.Read)))
                {
                    ReadAssetFileHeader(reader);
                    ImportSettings.FromBinary(reader);
                    int dataLen = reader.ReadInt32();
                    Debug.Assert(dataLen > 0);
                    data = reader.ReadBytes(dataLen);
                }

                Debug.Assert(data.Length > 0);

                using(var reader = new BinaryReader(new MemoryStream(data)))
                {
                    LODGroup lodGroup = new LODGroup() { Name = reader.ReadString() };

                    int lodCount = reader.ReadInt32();
                    for(int i = 0;i < lodCount;++i)
                    {
                        lodGroup.LODs.Add(BinaryToLOD(reader));
                    }

                    _lodGroups.Clear();
                    _lodGroups.Add(lodGroup);
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error,$"Failed to load geometry asset from file: {filePath}");
            }
        }

        private MeshLOD BinaryToLOD(BinaryReader reader)
        {
            MeshLOD lod = new MeshLOD();
            lod.Name = reader.ReadString();
            lod.LodThreshold = reader.ReadInt32();

            int numMeshes = reader.ReadInt32();
            for(int i = 0;i < numMeshes;++i)
            {
                Mesh mesh = new Mesh()
                {
                    Name = reader.ReadString(),
                    NumVertices = reader.ReadInt32(),
                    NumIndices = reader.ReadInt32(),
                    ElementType = (ElementType)reader.ReadInt32(),
                    ElementSize = reader.ReadInt32(),
                };

                mesh.PositionBuffer = reader.ReadBytes(Mesh.PositionSize * mesh.NumVertices);
                mesh.IndexBuffer = reader.ReadBytes(sizeof(uint) * mesh.NumIndices);
                mesh.ElementBuffer = reader.ReadBytes(mesh.ElementSize * mesh.NumVertices);

                lod.Meshes.Add(mesh);
            }

            return lod;
        }

        public override void Import(string filePath)
        {
            Debug.Assert(File.Exists(filePath));
            string extension = Path.GetExtension(filePath).ToLower();

            try
            {
                if(extension == ".fbx") ImportFBX(filePath);

                Logger.LogMessage(MsgType.Info,$"Successfully imported [{filePath}].");
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error,$"Failed to import [{filePath}].");
            }
        }

        private void ImportFBX(string filePath)
        {
            Logger.LogMessage(MsgType.Info,$"Importing FBX file [{filePath}]...");

            string tempDirPath = Application.Current.Dispatcher.Invoke(() => Project.CurrProject.TempDirPath);

            lock(_lock)
            {
                if(!Directory.Exists(tempDirPath)) Directory.CreateDirectory(tempDirPath);
            }

            string tempFilePath = $@"{tempDirPath}{AssetHelper.GetRandomString()}.fbx";
            File.Copy(filePath,tempFilePath,true);
            AssetToolAPI.ImportFBX(filePath,this);
        }
    }
}
