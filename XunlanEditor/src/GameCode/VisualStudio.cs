using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using XunlanEditor.GameProject;
using XunlanEditor.Utilities;

namespace XunlanEditor.GameCode
{
    static class VisualStudio
    {
        private static EnvDTE80.DTE2 _vsInstance = null;

        private static readonly string _progID = "VisualStudio.DTE";

        public static bool BuildSucceeded { get; private set; } = true;
        public static bool BuildDone { get; private set; } = true;

        public static bool AddFileToSolution(string solutionPath, string projectName, string[] headerAndCppFiles)
        {
            Debug.Assert(headerAndCppFiles?.Length > 0);

            OpenVisualStudio(solutionPath);

            try
            {
                if (_vsInstance == null) return false;

                if (!_vsInstance.Solution.IsOpen) _vsInstance.Solution.Open(solutionPath);
                else _vsInstance.ExecuteCommand("File.SaveAll");

                foreach (EnvDTE.Project project in _vsInstance.Solution.Projects)
                {
                    if (project.UniqueName.Contains(projectName))
                    {
                        foreach (var file in headerAndCppFiles)
                        {
                            project.ProjectItems.AddFromFile(file);
                        }
                    }
                }
                
                string header = headerAndCppFiles.FirstOrDefault(x => Path.GetExtension(x) == ".h");
                if (!string.IsNullOrEmpty(header))
                {
                    _vsInstance.ItemOperations.OpenFile(header, EnvDTE.Constants.vsViewKindTextView).Visible = true;
                }

                string cpp = headerAndCppFiles.FirstOrDefault(x => Path.GetExtension(x) == ".cpp");
                if (!string.IsNullOrEmpty(cpp))
                {
                    _vsInstance.ItemOperations.OpenFile(cpp, EnvDTE.Constants.vsViewKindTextView).Visible = true;
                }
                _vsInstance.MainWindow.Activate();
                _vsInstance.MainWindow.Visible = true;

                return true;
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error, "Failed to add file to Visual Studio project");
                return false;
            }
        }

        public static void BuildSolution(Project project, string configName, bool showWindow = true)
        {
            if (IsDebugging())
            {
                Logger.LogMessage(MsgType.Error, "Visual Studio is running.");
                return;
            }

            OpenVisualStudio(project.SolutionPath);
            BuildSucceeded = BuildDone = false;

            for (int i = 0; i < 3; ++i)
            {
                try
                {
                    if (!_vsInstance.Solution.IsOpen) _vsInstance.Solution.Open(project.SolutionPath);
                    _vsInstance.MainWindow.Visible = showWindow;

                    _vsInstance.Events.BuildEvents.OnBuildProjConfigBegin += OnBuildSolutionBegin;
                    _vsInstance.Events.BuildEvents.OnBuildProjConfigDone += OnBuildSolutionDone;

                    _vsInstance.Solution.SolutionBuild.SolutionConfigurations.Item(configName).Activate();
                    _vsInstance.ExecuteCommand("Build.BuildSolution");

                    while (!BuildSucceeded) System.Threading.Thread.Sleep(1000);

                    return;
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                    Logger.LogMessage(MsgType.Error, $"Attempt {i}: failed to build {project.ProjectName}");
                    System.Threading.Thread.Sleep(1000);
                }
            }
        }

        public static bool IsDebugging()
        {
            bool result = false;

            for (int i = 0; i < 3; ++i)
            {
                try
                {
                    result = _vsInstance != null &&
                        (_vsInstance.Debugger.CurrentProgram != null ||
                        _vsInstance.Debugger.CurrentMode == EnvDTE.dbgDebugMode.dbgRunMode);

                    if (result) return true;
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                    if (!result) System.Threading.Thread.Sleep(1000);
                }
            }

            return false;
        }

        private static void OpenVisualStudio(string solutionPath)
        {
            IRunningObjectTable runningObjectTable = null;
            IEnumMoniker monikerTable = null;
            IBindCtx bindCtx = null;

            try
            {
                if (_vsInstance != null) return;

                // Find the open VisualStudio
                var hResult = GetRunningObjectTable(0, out runningObjectTable);
                if (hResult < 0 || runningObjectTable == null)
                {
                    throw new COMException($"GetRunningObjectTable() returns HRESULT: {hResult:X8}");
                }

                runningObjectTable.EnumRunning(out monikerTable);
                monikerTable.Reset();

                hResult = CreateBindCtx(0, out bindCtx);
                if (hResult < 0 || bindCtx == null)
                {
                    throw new COMException($"CreateBindCtx() returns HRESULT: {hResult:X8}");
                }

                IMoniker[] currentMoniker = new IMoniker[1];
                while (monikerTable.Next(1, currentMoniker, IntPtr.Zero) == 0)
                {
                    string name = string.Empty;
                    currentMoniker[0]?.GetDisplayName(bindCtx, null, out name);

                    if (name.Contains(_progID))
                    {
                        hResult = runningObjectTable.GetObject(currentMoniker[0], out object obj);
                        if (hResult < 0 || obj == null)
                        {
                            throw new COMException($"{nameof(runningObjectTable)}.GetObject() returns HRESULT: {hResult:X8}");
                        }

                        EnvDTE80.DTE2 dte = obj as EnvDTE80.DTE2;
                        string solutionName = dte.Solution.FullName;
                        if (solutionName == solutionPath)
                        {
                            _vsInstance = dte;
                            break;
                        }
                    }
                }

                if (_vsInstance == null)
                {
                    Type visualStudioType = Type.GetTypeFromProgID(_progID, true);
                    _vsInstance = Activator.CreateInstance(visualStudioType) as EnvDTE80.DTE2;
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error, "Fail to open Visual Studio");
            }
            finally
            {
                if (monikerTable != null) Marshal.ReleaseComObject(monikerTable);
                if (runningObjectTable != null) Marshal.ReleaseComObject(runningObjectTable);
                if (bindCtx != null) Marshal.ReleaseComObject(bindCtx);
            }
        }
        public static void CloseVisualStudio()
        {
            if (_vsInstance?.Solution.IsOpen == true)
            {
                _vsInstance.ExecuteCommand("File.SaveAll");
                _vsInstance.Solution.Close(true);
            }

            _vsInstance?.Quit();
        }

        private static void OnBuildSolutionBegin(string project, string projectConfig, string platform, string solutionConfig)
        {
            Logger.LogMessage(MsgType.Info, $"Building: {project}, {solutionConfig}");
        }
        private static void OnBuildSolutionDone(string project, string projectConfig, string platform, string solutionConfig, bool success)
        {
            if (BuildDone) return;

            BuildDone = true;
            BuildSucceeded = success;

            if (success) Logger.LogMessage(MsgType.Info, $"Building {projectConfig} succeeded");
            else Logger.LogMessage(MsgType.Error, $"Building {projectConfig} failed");
        }

        public static void Run(Project project, string configName, bool debug)
        {
            if (_vsInstance != null && !IsDebugging() && BuildDone && BuildSucceeded)
            {
                _vsInstance.ExecuteCommand(debug ? "Debug.Start" : "Debug.StartWithoutDebugging");
            }
        }

        public static void Stop()
        {
            if (_vsInstance != null && IsDebugging())
            {
                _vsInstance.ExecuteCommand("Debug.StopDebugging");
            }
        }

        [DllImport("ole32.dll")]
        private static extern int GetRunningObjectTable(uint reserved, out IRunningObjectTable pprot);
        [DllImport("ole32.dll")]
        private static extern int CreateBindCtx(uint reserved, out IBindCtx ppbc);
    }
}
