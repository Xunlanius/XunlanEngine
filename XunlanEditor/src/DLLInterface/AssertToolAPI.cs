using System;
using System.Numerics;
using System.Runtime.InteropServices;
using XunlanEditor.Content;
using XunlanEditor.AssertToolStruct;
using System.Diagnostics;
using XunlanEditor.Utilities;

namespace XunlanEditor.AssertToolStruct
{
    [StructLayout(LayoutKind.Sequential)]
    class GeometryImportSettings
    {
        public float SmoothingAngle = 170;
        public byte CalculateNormals = 0;
        public byte CalculateTangents = 1;
        public byte ReverseHandedness = 0;
        public byte ImportEmbededTextures = 1;
        public byte ImportAnimations = 1;
    }

    [StructLayout(LayoutKind.Sequential)]
    class SceneData : IDisposable
    {
        public IntPtr Buffer;
        public int BufferByteSize;
        public GeometryImportSettings Settings = new GeometryImportSettings();

        ~SceneData() { Dispose(); }

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
    class AssertToolAPI
    {
        private const string _assertToolDLL = "AssertToolDLL";

        public static Geometry CreateMesh(PrimitiveMeshInitInfo info)
        {
            Debug.Assert(info != null);

            using SceneData sceneData = new SceneData();
            try
            {
                CreatePrimitiveMesh(sceneData, info);

                Debug.Assert(sceneData.Buffer != IntPtr.Zero);
                Debug.Assert(sceneData.BufferByteSize > 0);

                byte[] pDst = new byte[sceneData.BufferByteSize];
                Marshal.Copy(sceneData.Buffer, pDst, 0, sceneData.BufferByteSize);

                Geometry geometry = new Geometry();
                geometry.UnpackRawData(pDst);

                return geometry;
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error, $"Failed to create primitive mesh [{info.MeshType}]");
                throw;
            }
        }

        [DllImport(_assertToolDLL)]
        private static extern void CreatePrimitiveMesh([In, Out] SceneData data, PrimitiveMeshInitInfo info);
    }
}
