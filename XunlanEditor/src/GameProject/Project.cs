using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Threading.Tasks;
using System.Windows.Input;
using XunlanEditor.DLLInterface;
using XunlanEditor.GameCode;
using XunlanEditor.GameObjects;
using XunlanEditor.src.Content;
using XunlanEditor.Utilities;

namespace XunlanEditor.GameProject
{
    [DataContract(Name = "Project")]
    class Project : ViewModelBase
    {
        public static Project CurrProject
        {
            get; set;
        }

        [DataMember(Order = 0)]
        public string ProjectName
        {
            get; private set;
        }
        [DataMember(Order = 1)]
        public string ProjectDirPath
        {
            get; private set;
        }

        public string ProjectFilePath
        {
            get => Path.Combine(ProjectDirPath,$"{ProjectName}{ProjectFileInfo.ProjectFileSuffix}");
        }
        public string SolutionPath
        {
            get => Path.Combine(ProjectDirPath,$"{ProjectName}{ProjectFileInfo.SolutionSuffix}");
        }
        public string XunlanDirPath
        {
            get => Path.Combine(ProjectDirPath,nameof(ProjectFileInfo.ProjectFolders._Xunlan));
        }
        public string AssetsDirPath
        {
            get => Path.Combine(ProjectDirPath,nameof(ProjectFileInfo.ProjectFolders.Assets));
        }
        public string GameCodeDirPath
        {
            get => Path.Combine(ProjectDirPath,nameof(ProjectFileInfo.ProjectFolders.GameCode));
        }
        public string TempDirPath
        {
            get => Path.Combine(XunlanDirPath,@"Temp\");
        }
        public string DLLPath
        {
            get => $@"{ProjectDirPath}\bin\output\x64\{DLLBuildConfig}\{ProjectName}.dll";
        }

        private int _buildConfig;
        [DataMember(Order = 2)]
        public int BuildConfig
        {
            get => _buildConfig;
            set
            {
                if(_buildConfig == value)
                    return;
                _buildConfig = value;
                OnPropertyChanged(nameof(BuildConfig));
            }
        }

        public BuildConfiguration StandAloneBuildConfig
        {
            get => BuildConfig == 0 ? BuildConfiguration.Debug : BuildConfiguration.Release;
        }
        public BuildConfiguration DLLBuildConfig
        {
            get => BuildConfig == 0 ? BuildConfiguration.DebugEditor : BuildConfiguration.ReleaseEditor;
        }

        private string[] _availableScriptNames;
        public string[] AvailableScriptNames
        {
            get => _availableScriptNames;
            set
            {
                if(_availableScriptNames == value)
                    return;
                _availableScriptNames = value;
                OnPropertyChanged(nameof(AvailableScriptNames));
            }
        }

        [DataMember(Name = nameof(SceneList),Order = 3)]
        private readonly ObservableCollection<Scene> _sceneList = new ObservableCollection<Scene>();
        public ReadOnlyObservableCollection<Scene> SceneList
        {
            get; private set;
        }

        public Scene ActiveScene
        {
            get => SceneList.FirstOrDefault(scene => scene.IsActive);
        }

        public ICommand UndoCommand
        {
            get; private set;
        }
        public ICommand RedoCommand
        {
            get; private set;
        }
        public ICommand SaveCommand
        {
            get; private set;
        }
        public ICommand AddSceneCommand
        {
            get; private set;
        }
        public ICommand RemoveSceneCommand
        {
            get; private set;
        }
        public ICommand BuildCommand
        {
            get; private set;
        }
        public ICommand StartDebugCommand
        {
            get; private set;
        }
        public ICommand StartWithoutDebugCommand
        {
            get; private set;
        }
        public ICommand StopCommand
        {
            get; private set;
        }

        /// <summary>
        /// Only called when creating the project template
        /// </summary>
        public Project(string projectName,string projectDirPath)
        {
            ProjectName = projectName;
            ProjectDirPath = projectDirPath;

            Scene defaultScene = new Scene(this,"Default Scene");
            defaultScene.IsActive = true;

            _sceneList.Add(defaultScene);

            OnDeserialized(new StreamingContext());
        }

        [OnDeserialized]
        private async void OnDeserialized(StreamingContext context)
        {
            SceneList = new ReadOnlyObservableCollection<Scene>(_sceneList);
            OnPropertyChanged(nameof(SceneList));

            SetCommand();

            Logger.Clear();
            Logger.LogMessage(MsgType.Info,$"Project [{ProjectName}] open");
            AssetWatcher.Reset(AssetsDirPath,ProjectDirPath);
            UndoRedo.Reset();

            await BuildGameCodeDLL(false);
        }

        private void SetCommand()
        {
            UndoCommand = new RelayCommand<object>(x => UndoRedo.Undo(),x => UndoRedo.UndoList.Any());
            RedoCommand = new RelayCommand<object>(x => UndoRedo.Redo(),x => UndoRedo.RedoList.Any());
            SaveCommand = new RelayCommand<object>(x => Save());
            AddSceneCommand = new RelayCommand<object>(x =>
            {
                Scene scene = new Scene(this,$"New Scene {_sceneList.Count}");
                _sceneList.Add(scene);
                int index = _sceneList.Count - 1;

                UndoRedo.AddUndoRedoAction(new UndoRedoAction(
                    $"Add Scene: {scene.Name}",
                    () => _sceneList.RemoveAt(index),
                    () => _sceneList.Insert(index,scene)));
            });
            RemoveSceneCommand = new RelayCommand<Scene>(scene =>
            {
                int index = _sceneList.IndexOf(scene);
                _sceneList.RemoveAt(index);

                UndoRedo.AddUndoRedoAction(new UndoRedoAction(
                    $"Remove Scene: {scene.Name}",
                    () => _sceneList.Insert(index,scene),
                    () => _sceneList.RemoveAt(index)));
            },scene => !scene.IsActive);
            BuildCommand = new RelayCommand<bool>(async showWindow => await BuildGameCodeDLL(showWindow),x => !VisualStudio.IsDebugging() && VisualStudio.BuildDone);
            StartDebugCommand = new RelayCommand<object>(async x => await RunGame(true),x => !VisualStudio.IsDebugging() && VisualStudio.BuildDone);
            StartWithoutDebugCommand = new RelayCommand<object>(async x => await RunGame(false),x => !VisualStudio.IsDebugging() && VisualStudio.BuildDone);
            StopCommand = new RelayCommand<object>(async x => await StopGame(),x => VisualStudio.IsDebugging());

            OnPropertyChanged(nameof(UndoCommand));
            OnPropertyChanged(nameof(RedoCommand));
            OnPropertyChanged(nameof(SaveCommand));
            OnPropertyChanged(nameof(AddSceneCommand));
            OnPropertyChanged(nameof(RemoveSceneCommand));
            OnPropertyChanged(nameof(BuildCommand));
            OnPropertyChanged(nameof(StartDebugCommand));
            OnPropertyChanged(nameof(StartWithoutDebugCommand));
            OnPropertyChanged(nameof(StopCommand));
        }

        public static Project Load(string projectFilePath)
        {
            Debug.Assert(File.Exists(projectFilePath));
            return Serializer.FileToObject<Project>(projectFilePath);
        }
        public void Unload()
        {
            Logger.Clear();
            UndoRedo.Reset();

            foreach(Scene scene in SceneList)
                scene.Unload();

            DeleteTempFolder();

            UnloadGameCodeDLL();
            VisualStudio.CloseVisualStudio();
        }

        private void DeleteTempFolder()
        {
            if(Directory.Exists(TempDirPath))
                Directory.Delete(TempDirPath,true);
        }

        private void Save()
        {
            Serializer.ObjectToFile(this,ProjectFilePath);
            Logger.LogMessage(MsgType.Info,$"Project [{ProjectName}] saved to [{ProjectFilePath}]");
        }
        private void SaveToBinary()
        {
            string configName = StandAloneBuildConfig.ToString();
            string binPath = Path.Combine(ProjectDirPath,$@"bin\output\x64\{configName}\game.bin");

            using var bw = new BinaryWriter(File.Open(binPath,FileMode.Create,FileAccess.Write));

            // GameObject count
            bw.Write(ActiveScene.GameObjectList.Count);
            foreach(GameObject obj in ActiveScene.GameObjectList)
            {
                // GameObject type
                bw.Write(0);

                // Component count
                bw.Write(obj.ComponentList.Count);
                foreach(Component component in obj.ComponentList)
                {
                    // Component type
                    bw.Write((int)ComponentFactory.ToEnumType(component));
                    component.WriteToBinary(bw);
                }
            }
        }

        private async Task BuildGameCodeDLL(bool showWindow = true)
        {
            try
            {
                UnloadGameCodeDLL();
                await Task.Run(() => VisualStudio.BuildSolution(this,DLLBuildConfig,showWindow));
                LoadGameCodeDLL();
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                throw;
            }
        }
        private void LoadGameCodeDLL()
        {
            string dllPath = DLLPath;

            if(!File.Exists(dllPath))
            {
                Logger.LogMessage(MsgType.Error,$"{ProjectName}.dll is not found. Please build the project first.");
                return;
            }

            if(EngineAPI.LoadGameCodeDLL(dllPath) != 0)
            {
                AvailableScriptNames = EngineAPI.GetScriptNames();

                // Load all the GameObjects that have Script component
                ActiveScene.GameObjectList.Where(x => x.GetComponent<Script>() != null).ToList().ForEach(x => x.IsAlive = true);

                Logger.LogMessage(MsgType.Info,$"{ProjectName}.dll loaded successfully.");
            }
            else
                Logger.LogMessage(MsgType.Error,$"Failed to load {ProjectName}.dll.");
        }
        private void UnloadGameCodeDLL()
        {
            AvailableScriptNames = null;

            // Unload all the GameObjects that have Script component
            ActiveScene.GameObjectList.Where(x => x.GetComponent<Script>() != null).ToList().ForEach(x => x.IsAlive = false);

            if(EngineAPI.UnloadGameCodeDLL() != 0)
                Logger.LogMessage(MsgType.Info,$"{ProjectName}.dll unloaded successfully.");
        }

        private async Task RunGame(bool debug)
        {
            BuildConfiguration config = StandAloneBuildConfig;

            await Task.Run(() => VisualStudio.BuildSolution(this,config,debug));

            if(VisualStudio.BuildSucceeded)
            {
                SaveToBinary();
                await Task.Run(() => VisualStudio.Run(this,config,debug));
            }
        }
        private async Task StopGame()
        {
            await Task.Run(() => VisualStudio.Stop());
        }
    }
}