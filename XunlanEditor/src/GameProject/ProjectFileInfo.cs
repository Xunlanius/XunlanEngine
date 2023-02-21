using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Windows;

namespace XunlanEditor.GameProject
{
    static class ProjectFileInfo
    {
        public enum ProjectFolders
        {
            _Xunlan,
            Content,
            GameCode
        }

        public static string ProjectFileSuffix { get; } = ".Xunlan";
        public static string SolutionSuffix { get; } = ".sln";
        public static string VSProjectFileSuffix { get; } = ".vcxproj";

        public static string EnginePath { get; private set; }
        public static string EngineIncludePath { get => Path.Combine(EnginePath, @"XunlanLib\src"); }
        public static string EngineLibraryPath { get => Path.Combine(EnginePath, @"bin\output\$(Platform)\$(Configuration)\"); }

        public static void UpdateEnginePath()
        {
            string enginePath = Environment.GetEnvironmentVariable("XUNLAN_ENGINE", EnvironmentVariableTarget.User);

            if (enginePath == null || !Directory.Exists(Path.Combine(enginePath, @"XunlanLib\src")))
            {
                var diag = new EnginePathDiag();
                if (diag.ShowDialog() == true)
                {
                    EnginePath = diag.EnginePath;
                    Environment.SetEnvironmentVariable("XUNLAN_ENGINE", EnginePath.ToUpper(), EnvironmentVariableTarget.User);
                }
                else Application.Current.Shutdown();
            }
            else EnginePath = enginePath;
        }
    }
}
