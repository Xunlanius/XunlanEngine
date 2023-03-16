using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Threading;
using XunlanEditor.GameProject;
using XunlanEditor.Utilities;

namespace XunlanEditor.GameCode
{
    enum BuildConfiguration
    {
        Debug,
        DebugEditor,
        Release,
        ReleaseEditor,
    }

    static class VisualStudio
    {
        private static EnvDTE80.DTE2 _vsInstance = null;
        private static readonly string _progID = "VisualStudio.DTE";
        private static readonly ManualResetEventSlim _resetEvent = new ManualResetEventSlim(false);
        private static readonly object _lock = new object();

        public static bool BuildSucceeded { get; private set; } = false;
        public static bool BuildDone { get; private set; } = false;

        /// <summary>
        /// Add .h and .cpp files to certain project of certain solution
        /// </summary>
        public static bool AddFileToSolution(string solutionPath,string projectName,string[] headerAndCppFiles)
        {
            lock(_lock)
                return AddFileToSolution_Internal(solutionPath,projectName,headerAndCppFiles);
        }
        public static void BuildSolution(Project project,BuildConfiguration config,bool showWindow = true)
        {
            lock(_lock)
                BuildSolution_Internal(project,config,showWindow);
        }
        public static bool IsDebugging()
        {
            lock(_lock)
                return IsDebugging_Internal();
        }
        public static void CloseVisualStudio()
        {
            lock(_lock)
                CloseVisualStudio_Internal();
        }

        public static void Run(Project project,BuildConfiguration config,bool debug)
        {
            lock(_lock)
                Run_Internal(project,config,debug);
        }
        public static void Stop()
        {
            lock(_lock)
                Stop_Internal();
        }

        private static bool AddFileToSolution_Internal(string solutionPath,string projectName,string[] headerAndCppFiles)
        {
            Debug.Assert(headerAndCppFiles?.Length > 0);

            OpenVisualStudio_Internal(solutionPath);

            try
            {
                if(_vsInstance == null)
                    return false;

                CallOnSTAThread(() =>
                {
                    // Open the solution
                    if(!_vsInstance.Solution.IsOpen)
                        _vsInstance.Solution.Open(solutionPath);
                    else
                        _vsInstance.ExecuteCommand("File.SaveAll");

                    // Add files to each project
                    foreach(EnvDTE.Project project in _vsInstance.Solution.Projects)
                    {
                        if(project.UniqueName.Contains(projectName))
                        {
                            foreach(string file in headerAndCppFiles)
                            {
                                project.ProjectItems.AddFromFile(file);
                            }
                        }
                    }

                    // Open the cpp files
                    string header = headerAndCppFiles.FirstOrDefault(x => Path.GetExtension(x) == ".h");
                    string cpp = headerAndCppFiles.FirstOrDefault(x => Path.GetExtension(x) == ".cpp");

                    if(!string.IsNullOrEmpty(cpp))
                    {
                        _vsInstance.ItemOperations.OpenFile(cpp,EnvDTE.Constants.vsViewKindTextView).Visible = true;
                    }

                    _vsInstance.MainWindow.Activate();
                    _vsInstance.MainWindow.Visible = true;
                });

                return true;
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error,"Failed to add file to Visual Studio project");
                return false;
            }
        }
        private static void BuildSolution_Internal(Project project,BuildConfiguration config,bool showWindow = true)
        {
            if(IsDebugging_Internal())
            {
                Logger.LogMessage(MsgType.Error,"Visual Studio is running.");
                return;
            }

            OpenVisualStudio_Internal(project.SolutionPath);

            BuildDone = false;
            BuildSucceeded = false;

            string configName = config.ToString();

            try
            {
                string outputPath = Path.Combine($"{project.ProjectDirPath}",$@"bin\output\x64\{configName}\");
                foreach(string pdb in Directory.GetFiles(outputPath,"*.pdb"))
                    File.Delete(pdb);
            }
            catch(Exception ex)
            {
                Debug.WriteLine($"Error: {ex.Message}");
            }

            CallOnSTAThread(() =>
            {
                if(!_vsInstance.Solution.IsOpen)
                    _vsInstance.Solution.Open(project.SolutionPath);
                _vsInstance.MainWindow.Visible = showWindow;

                _vsInstance.Events.BuildEvents.OnBuildProjConfigBegin += OnBuildSolutionBegin;
                _vsInstance.Events.BuildEvents.OnBuildProjConfigDone += OnBuildSolutionDone;

                _vsInstance.Solution.SolutionBuild.SolutionConfigurations.Item(configName).Activate();
                _vsInstance.ExecuteCommand("Build.BuildSolution");

                _resetEvent.Wait();
                _resetEvent.Reset();
            });
        }
        private static bool IsDebugging_Internal()
        {
            bool result = false;

            CallOnSTAThread(() =>
            {
                result = _vsInstance != null &&
                    (_vsInstance.Debugger.CurrentProgram != null || _vsInstance.Debugger.CurrentMode == EnvDTE.dbgDebugMode.dbgRunMode);
            });

            return result;
        }
        private static void OpenVisualStudio_Internal(string solutionPath)
        {
            IRunningObjectTable runningObjectTable = null;
            IEnumMoniker monikerTable = null;
            IBindCtx bindCtx = null;

            try
            {
                if(_vsInstance != null)
                    return;

                // Find the open VisualStudio
                var hResult = GetRunningObjectTable(0,out runningObjectTable);
                if(hResult < 0 || runningObjectTable == null)
                {
                    throw new COMException($"GetRunningObjectTable() returns HRESULT: {hResult:X8}");
                }

                runningObjectTable.EnumRunning(out monikerTable);
                monikerTable.Reset();

                hResult = CreateBindCtx(0,out bindCtx);
                if(hResult < 0 || bindCtx == null)
                    throw new COMException($"CreateBindCtx() returns HRESULT: {hResult:X8}");

                IMoniker[] currentMoniker = new IMoniker[1];
                while(monikerTable.Next(1,currentMoniker,IntPtr.Zero) == 0)
                {
                    string name = string.Empty;
                    currentMoniker[0]?.GetDisplayName(bindCtx,null,out name);

                    if(name.Contains(_progID))
                    {
                        hResult = runningObjectTable.GetObject(currentMoniker[0],out object obj);
                        if(hResult < 0 || obj == null)
                        {
                            throw new COMException($"{nameof(runningObjectTable)}.GetObject() returns HRESULT: {hResult:X8}");
                        }

                        EnvDTE80.DTE2 dte = obj as EnvDTE80.DTE2;

                        string solutionName = string.Empty;
                        CallOnSTAThread(() => { solutionName = dte.Solution.FullName; });

                        if(solutionName == solutionPath)
                        {
                            _vsInstance = dte;
                            break;
                        }
                    }
                }

                if(_vsInstance == null)
                {
                    Type visualStudioType = Type.GetTypeFromProgID(_progID,true);
                    _vsInstance = Activator.CreateInstance(visualStudioType) as EnvDTE80.DTE2;
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error,"Failed to open Visual Studio");
            }
            finally
            {
                if(monikerTable != null)
                    Marshal.ReleaseComObject(monikerTable);
                if(runningObjectTable != null)
                    Marshal.ReleaseComObject(runningObjectTable);
                if(bindCtx != null)
                    Marshal.ReleaseComObject(bindCtx);
            }
        }
        private static void CloseVisualStudio_Internal()
        {
            CallOnSTAThread(() =>
            {
                // Save before close
                if(_vsInstance?.Solution.IsOpen == true)
                    _vsInstance.Solution.Close(true);

                _vsInstance?.Quit();
                _vsInstance = null;
            });
        }

        private static void Run_Internal(Project project,BuildConfiguration config,bool debug)
        {
            if(_vsInstance == null || IsDebugging_Internal() || !BuildSucceeded)
                return;

            CallOnSTAThread(() =>
            {
                _vsInstance.ExecuteCommand(debug ? "Debug.Start" : "Debug.StartWithoutDebugging");
            });
        }
        private static void Stop_Internal()
        {
            if(_vsInstance == null || !IsDebugging_Internal())
                return;

            CallOnSTAThread(() =>
            {
                _vsInstance.ExecuteCommand("Debug.StopDebugging");
            });
        }

        private static void OnBuildSolutionBegin(string project,string projectConfig,string platform,string solutionConfig)
        {
            if(BuildDone)
                return;
            Logger.LogMessage(MsgType.Info,$"Building: {project}, {solutionConfig}");
        }
        private static void OnBuildSolutionDone(string project,string projectConfig,string platform,string solutionConfig,bool success)
        {
            BuildDone = true;
            BuildSucceeded = success;

            _resetEvent.Set();

            if(success)
                Logger.LogMessage(MsgType.Info,$"Building {solutionConfig} succeeded");
            else
                Logger.LogMessage(MsgType.Error,$"Building {solutionConfig} failed");
        }

        [DllImport("ole32.dll")]
        private static extern int GetRunningObjectTable(uint reserved,out IRunningObjectTable pprot);
        [DllImport("ole32.dll")]
        private static extern int CreateBindCtx(uint reserved,out IBindCtx ppbc);

        private static void CallOnSTAThread(Action action)
        {
            Debug.Assert(action != null);
            Thread thread = new Thread(() =>
            {
                MessageFilter.Register();
                try
                {
                    action();
                }
                catch(Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                    Logger.LogMessage(MsgType.Warning,ex.Message);
                }
                finally { MessageFilter.Revoke(); }
            });

            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();
            thread.Join();
        }

        [ComImport(), Guid("00000016-0000-0000-C000-000000000046"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
        private interface IOleMessageFilter
        {
            [PreserveSig]
            int HandleInComingCall(int dwCallType,IntPtr hTaskCaller,int dwTickCount,IntPtr lpInterfaceInfo);

            [PreserveSig]
            int RetryRejectedCall(IntPtr hTaskCallee,int dwTickCount,int dwRejectType);

            [PreserveSig]
            int MessagePending(IntPtr hTaskCallee,int dwTickCount,int dwPendingType);
        }
        private class MessageFilter : IOleMessageFilter
        {
            private const int SERVERCALL_ISHANDLED = 0;
            private const int PENDINGMSG_WAITDEFPROCESS = 2;
            private const int SERVERCALL_RETRYLATER = 2;

            // implement IOleMessageFilter interface.
            [DllImport("Ole32.dll")]
            private static extern int CoRegisterMessageFilter(IOleMessageFilter newFilter,out IOleMessageFilter oldFilter);

            public static void Register()
            {
                IOleMessageFilter newFilter = new MessageFilter();
                int hr = CoRegisterMessageFilter(newFilter,out var oldFilter);
                Debug.Assert(hr >= 0,"Registering COM IMessageFilter failed.");
            }

            public static void Revoke()
            {
                int hr = CoRegisterMessageFilter(null,out var oldFilter);
                Debug.Assert(hr >= 0,"Unregistering COM IMessageFilter failed.");
            }


            int IOleMessageFilter.HandleInComingCall(int dwCallType,System.IntPtr hTaskCaller,int dwTickCount,System.IntPtr lpInterfaceInfo)
            {
                return SERVERCALL_ISHANDLED;
            }


            int IOleMessageFilter.RetryRejectedCall(System.IntPtr hTaskCallee,int dwTickCount,int dwRejectType)
            {
                // Thread call was refused, try again.
                if(dwRejectType == SERVERCALL_RETRYLATER)
                {
                    // Retry thread call at once, if return value >= 0 & < 100.
                    Debug.WriteLine("COM server busy. Retrying call to EnvDTE interface.");
                    return 500;
                }
                // Too busy. Cancel call.
                return -1;
            }

            int IOleMessageFilter.MessagePending(System.IntPtr hTaskCallee,int dwTickCount,int dwPendingType)
            {
                return PENDINGMSG_WAITDEFPROCESS;
            }
        }
    }
}
