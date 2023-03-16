using System;
using System.Diagnostics;
using System.Numerics;
using System.Runtime.InteropServices;
using XunlanEditor.AssetToolStruct;
using XunlanEditor.Content;
using XunlanEditor.Utilities;

namespace XunlanEditor.AssetToolStruct
{
    [StructLayout(LayoutKind.Sequential)]
    class GeometryImportSettings
    {
        public float SmoothingAngle = 170;
        public bool CalculateNormals = false;
        public bool CalculateTangents = true;
        public bool ReverseHandedness = false;
        public bool ImportEmbededTextures = true;
        public bool ImportAnimations = true;
    }

    [StructLayout(LayoutKind.Sequential)]
    class SceneData : IDisposable
    {
        public IntPtr Buffer;
        public int BufferByteSize;
        public GeometryImportSettings Settings = new GeometryImportSettings();

        ~SceneData()
        {
            Dispose();
        }

        public void Dispose()
        {
            Marshal.FreeCoTaskMem(Buffer);
            GC.SuppressFinalize(this);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    class PrimitiveMeshInitInfo
    {
        public PrimitiveMeshType MeshType;
        public UInt32 SegmentX = 1;
        public UInt32 SegmentY = 1;
        public UInt32 SegmentZ = 1;
        public Vector3 Size = new Vector3(1.0f);
        public UInt32 LOD = 0;

        public PrimitiveMeshInitInfo(PrimitiveMeshType type)
        {
            MeshType = type;
        }
    }
}

namespace XunlanEditor.DLLInterface
{
    class AssetToolAPI
    {
        private const string _assetToolDLL = "AssetToolDLL";

        public static void CreatePrimitiveMesh(Geometry geometry,PrimitiveMeshInitInfo info)
        {
            CreateGeometry(
                geometry,
                (sceneData) => CreatePrimitiveMesh(info,sceneData),
                $"Failed to create primitive mesh [{info.MeshType}]");
        }

        public static void ImportFBX(string filePath,Geometry geometry)
        {
            CreateGeometry(
                geometry,
                (sceneData) => ImportFBX(filePath,sceneData),
                $"Failed to import FBX file: {filePath}.");
        }

        private static void CreateGeometry(Geometry geometry,Action<SceneData> sceneDataGenerator,string errorMsg)
        {
            Debug.Assert(geometry != null);

            using SceneData sceneData = new SceneData();
            try
            {
                sceneDataGenerator(sceneData);

                Debug.Assert(sceneData.Buffer != IntPtr.Zero);
                Debug.Assert(sceneData.BufferByteSize > 0);

                byte[] pDst = new byte[sceneData.BufferByteSize];
                Marshal.Copy(sceneData.Buffer,pDst,0,sceneData.BufferByteSize);

                geometry.UnpackRawData(pDst);
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error,errorMsg);
            }
        }

        [DllImport(_assetToolDLL)]
        private static extern void CreatePrimitiveMesh(PrimitiveMeshInitInfo info,[In, Out] SceneData data);

        [DllImport(_assetToolDLL)]
        private static extern void ImportFBX(string filePath,[In, Out] SceneData sceneData);
    }
}
