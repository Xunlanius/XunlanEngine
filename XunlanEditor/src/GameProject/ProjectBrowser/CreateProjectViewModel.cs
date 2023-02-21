using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Input;
using XunlanEditor.Utilities;

namespace XunlanEditor.GameProject
{
    class CreateProjectViewModel : ViewModelBase
    {
        private string _projectName = "NewProject";
        public string ProjectName
        {
            get => _projectName;
            set
            {
                if (_projectName != value)
                {
                    _projectName = value;
                    OnPropertyChanged(nameof(ProjectName));
                }
            }
        }

        private string _projectRootPath = $@"{Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments)}\Xunlan Projects\";
        public string ProjectRootPath
        {
            get => _projectRootPath;
            set
            {
                if (_projectRootPath != value)
                {
                    _projectRootPath = value;
                    OnPropertyChanged(nameof(ProjectRootPath));
                }
            }
        }

        public string ProjectDirPath { get => Path.GetFullPath(Path.Combine(ProjectRootPath, ProjectName)); }

        private string _errorMessage;
        public string ErrorMessage
        {
            get => _errorMessage;
            set
            {
                if (_errorMessage != value)
                {
                    _errorMessage = value;
                    OnPropertyChanged(nameof(ErrorMessage));
                }
            }
        }

        private static readonly ObservableCollection<ProjectTemplate> _templateList = new ObservableCollection<ProjectTemplate>();
        public static ReadOnlyObservableCollection<ProjectTemplate> TemplateList { get; } = new ReadOnlyObservableCollection<ProjectTemplate>(_templateList);

        public ICommand CreateProjectCommand { get; }

        public CreateProjectViewModel()
        {
            if (!ProjectTemplate.CheckTemplateFileIntegrity()) ProjectTemplate.CreateTemplateFile();
            LoadTemplate();

            CreateProjectCommand = new RelayCommand<ProjectTemplate>((template) =>
            {
                CreateProject(template);
                Project project = OpenProjectViewModel.OpenProject(new ProjectData() { ProjectName = ProjectName, ProjectDirPath = ProjectDirPath });

                ProjectBrowserViewModel.CreateEditorWindow(project);

            }, (template) => (ErrorMessage = CheckProjectInfoInput(ProjectRootPath, ProjectName)) == string.Empty);
        }

        public void CreateProject(ProjectTemplate template)
        {
            Debug.Assert(template != null);

            string projectDirPath = ProjectDirPath;

            try
            {
                // create necessary folders
                foreach (string folder in Enum.GetNames(typeof(ProjectFileInfo.ProjectFolders)))
                {
                    Directory.CreateDirectory(Path.Combine(projectDirPath, folder));
                }

                // directory "Xunlan" should be hidden
                DirectoryInfo dirInfo = new DirectoryInfo(Path.Combine(projectDirPath, nameof(ProjectFileInfo.ProjectFolders._Xunlan)));
                dirInfo.Attributes |= FileAttributes.Hidden;

                // create the project file ".Xunlan"
                CreateProjectFile(template, ProjectName, projectDirPath);
                CreateMSVCSolution(template, ProjectName, projectDirPath);
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error, $"Failed to create project");

                Debugger.Break();
                throw;
            }
        }

        private static void LoadTemplate()
        {
            string[] templateFilePaths = Directory.GetFiles(ProjectTemplate.TemplateRootPath, "template.xml", SearchOption.AllDirectories);
            Debug.Assert(templateFilePaths.Any());

            foreach(string templateFilePath in templateFilePaths)
            {
                ProjectTemplate template = Serializer.FileToObject<ProjectTemplate>(templateFilePath);
                _templateList.Add(template);
            }
        }

        private static string CheckProjectInfoInput(string projectRootPath, string projectName)
        {
            if (string.IsNullOrWhiteSpace(projectName.Trim()))
            {
                return "Please type in a project name.";
            }
            else if (projectName.IndexOfAny(Path.GetInvalidFileNameChars()) != -1)
            {
                return "Invalid characters used in the project name.";
            }

            if (string.IsNullOrWhiteSpace(projectRootPath.Trim()))
            {
                return "Please type in a project Path.";
            }
            else if (projectRootPath.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                return "Invalid characters used in the project path.";
            }

            string fullPath = Path.Combine(projectRootPath, projectName);

            if (Directory.Exists(fullPath) && Directory.EnumerateFileSystemEntries(fullPath).Any())
            {
                return "Selected project folder already exists.";
            }

            return string.Empty;
        }
        
        private void CreateProjectFile(ProjectTemplate template, string projectName, string projectDirPath)
        {
            // create the project file ".Xunlan"
            string projectXml = File.ReadAllText(template.TemplateProjectFilePath);
            projectXml = string.Format(projectXml, projectName, projectDirPath);
            string projectFilePath = Path.Combine(projectDirPath, $"{projectName}{ProjectFileInfo.ProjectFileSuffix}");
            File.WriteAllText(projectFilePath, projectXml);
        }
        private void CreateMSVCSolution(ProjectTemplate template, string projectName, string projectDirPath)
        {
            string templateSolutionFilePath = Path.Combine(template.TemplateDirPath, "MSVCSolution.txt");
            string templateVSProjectFilePath = Path.Combine(template.TemplateDirPath, "MSVCProject.txt");

            Debug.Assert(File.Exists(templateSolutionFilePath));
            Debug.Assert(File.Exists(templateVSProjectFilePath));

            string vsProjectGuid = "{" + Guid.NewGuid().ToString().ToUpper() + "}";
            string solutionGuid = "{" + Guid.NewGuid().ToString().ToUpper() + "}";

            string solutionFilePath = Path.Combine(projectDirPath, $"{projectName}{ProjectFileInfo.SolutionSuffix}");
            string vsProjectFilePath = Path.Combine(projectDirPath, $@"{nameof(ProjectFileInfo.ProjectFolders.GameCode)}\{projectName}{ProjectFileInfo.VSProjectFileSuffix}");

            string solution = File.ReadAllText(templateSolutionFilePath);
            solution = string.Format(solution, projectName, vsProjectGuid, solutionGuid);
            File.WriteAllText(Path.GetFullPath(solutionFilePath), solution);

            string project = File.ReadAllText(templateVSProjectFilePath);
            project = string.Format(project, projectName, vsProjectGuid, ProjectFileInfo.EngineIncludePath, ProjectFileInfo.EngineLibraryPath);
            File.WriteAllText(Path.GetFullPath(vsProjectFilePath), project);
        }
    }
}