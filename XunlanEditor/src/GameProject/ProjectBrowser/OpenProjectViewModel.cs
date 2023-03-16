using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Windows.Input;
using XunlanEditor.Utilities;

namespace XunlanEditor.GameProject
{
    [DataContract]
    class ProjectData
    {
        [DataMember(Order = 0)]
        public string ProjectName
        {
            get; set;
        }
        [DataMember(Order = 1)]
        public string ProjectDirPath
        {
            get; set;
        }
        [DataMember(Order = 2)]
        public DateTime Date
        {
            get; set;
        }

        public string ProjectFilePath
        {
            get => Path.Combine(ProjectDirPath,$@"{ProjectName}{ProjectFileInfo.ProjectFileSuffix}");
        }
    }

    class OpenProjectViewModel : ViewModelBase
    {
        private static readonly string _appDataPath = $@"{Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)}\XunlanEditor\";
        private static readonly string _projectDataPath = $@"{_appDataPath}ProjectData.xml";

        /// <summary>
        /// store the information of the projects which have been created
        /// </summary>
        private static readonly ObservableCollection<ProjectData> _projectDataList = new ObservableCollection<ProjectData>();
        public static ReadOnlyObservableCollection<ProjectData> ProjectDataList { get; } = new ReadOnlyObservableCollection<ProjectData>(_projectDataList);

        public static ICommand OpenProjectCommand
        {
            get;
        }

        static OpenProjectViewModel()
        {
            try
            {
                if(!Directory.Exists(_appDataPath))
                    Directory.CreateDirectory(_appDataPath);

                ReadProjectData();

                OpenProjectCommand = new RelayCommand<ProjectData>((projectData) =>
                {
                    Project project = OpenProject(projectData);

                    ProjectBrowserViewModel.CreateEditorWindow(project);

                },(projectData) => projectData is ProjectData);
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error,$"Failed to construct OpenProjectViewModel");

                Debugger.Break();
                throw;
            }
        }

        public static Project OpenProject(ProjectData data)
        {
            try
            {
                Debug.Assert(data != null);

                ReadProjectData();

                ProjectData projectData = _projectDataList.FirstOrDefault(x => { return x.ProjectFilePath == data.ProjectFilePath; });

                // if not existed, append the project data
                if(projectData == default)
                {
                    projectData = data;
                    projectData.Date = DateTime.Now;
                    _projectDataList.Add(projectData);
                }
                // if existed, just update the Date
                else
                    projectData.Date = DateTime.Now;

                WriteProjectData();

                return Project.Load(projectData.ProjectFilePath);
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error,$"Failed to open project");

                Debugger.Break();
                throw;
            }
        }

        private static void ReadProjectData()
        {
            _projectDataList.Clear();

            try
            {
                if(File.Exists(_projectDataPath))
                {
                    List<ProjectData> projectDatas = Serializer.FileToObject<List<ProjectData>>(_projectDataPath);
                    projectDatas.OrderByDescending(x => x.Date).ToList();

                    foreach(ProjectData projectData in projectDatas)
                    {
                        if(File.Exists(projectData.ProjectFilePath))
                        {
                            _projectDataList.Add(projectData);
                        }
                    }
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error,$"Failed to Read Project Data");

                Debugger.Break();
                throw;
            }
        }
        private static void WriteProjectData()
        {
            try
            {
                List<ProjectData> projectDatas = _projectDataList.OrderBy(x => x.Date).ToList();
                Serializer.ObjectToFile(projectDatas,_projectDataPath);
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error,$"Failed to write project data");

                Debugger.Break();
                throw;
            }
        }
    }
}
