using System;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using XunlanEditor.EngineAPIStructs;
using XunlanEditor.GameObjects;
using XunlanEditor.GameProject;

namespace XunlanEditor.EngineAPIStructs
{
    [StructLayout(LayoutKind.Sequential)]
    class TransformerInitInfo
    {
        public Vector3 Position;
        public Vector3 Rotation;
        public Vector3 Scale = new Vector3(1.0f, 1.0f, 1.0f);
    }

    [StructLayout(LayoutKind.Sequential)]
    class ScriptInitInfo
    {
        public IntPtr ScriptCreator;
    }

    [StructLayout(LayoutKind.Sequential)]
    class GameObjectInitInfo
    {
        public TransformerInitInfo TransformerInfo = new TransformerInitInfo();
        public ScriptInitInfo ScriptInfo = new ScriptInitInfo();
    }
}

namespace XunlanEditor.DLLInterface
{
    static class EngineAPI
    {
        private const string _engineDLL = "XunlanDLL.dll";

        [DllImport(_engineDLL, CharSet = CharSet.Ansi)]
        public static extern UInt32 LoadGameCodeDLL(string dllPath);
        [DllImport(_engineDLL)]
        public static extern UInt32 UnloadGameCodeDLL();

        [DllImport(_engineDLL)]
        [return: MarshalAs(UnmanagedType.SafeArray)]
        public static extern string[] GetScriptNames();
        [DllImport(_engineDLL)]
        public static extern IntPtr GetScriptCreator(string scriptName);

        [DllImport(_engineDLL)]
        public static extern UInt32 CreateRenderSurface(IntPtr parent, int width, int height);
        [DllImport(_engineDLL)]
        public static extern void RemoveRenderSurface(UInt32 surfaceID);
        [DllImport(_engineDLL)]
        public static extern IntPtr GetWindowHandle(UInt32 surfaceID);
        [DllImport(_engineDLL)]
        public static extern void ResizeRenderSurface(UInt32 surfaceID);

        internal static class GameObjectAPI
        {
            public static UInt32 CreateEntity(GameObject obj)
            {
                GameObjectInitInfo entityInfo = new GameObjectInitInfo();

                Transformer transformer = obj.GetComponent<Transformer>();
                TransformerInitInfo transformerInfo = new TransformerInitInfo()
                {
                    Position = transformer.Position,
                    Rotation = transformer.Rotation,
                    Scale = transformer.Scale,
                };

                Script script = obj.GetComponent<Script>();
                ScriptInitInfo scriptInitInfo = null;
                if (script != null)
                {
                    scriptInitInfo = new ScriptInitInfo() { ScriptCreator = GetScriptCreator(script.ScriptName) };
                }

                entityInfo.TransformerInfo = transformerInfo;
                entityInfo.ScriptInfo = scriptInitInfo;

                return Create(entityInfo);
            }
            public static void RemoveEntity(GameObject obj) => Remove(obj.ObjectID);

            [DllImport(_engineDLL)]
            private static extern UInt32 Create(GameObjectInitInfo desc);
            [DllImport(_engineDLL)]
            private static extern void Remove(UInt32 id);
        }
    }
}