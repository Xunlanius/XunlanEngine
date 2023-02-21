using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using XunlanEditor.GameProject;
using XunlanEditor.Utilities;

namespace XunlanEditor.GameCode
{
    /// <summary>
    /// CreateScriptDiag.xaml 的交互逻辑
    /// </summary>
    public partial class CreateScriptDiag : Window
    {
        private static readonly string _headerCode =
@"#pragma once

#include ""EngineAPI/Script.h""

namespace {0}
{{
    SCRIPT_DECL({1})
}}";

        private static readonly string _cppCode = 
@"#include ""{1}.h""

namespace {0}
{{
    namespace
    {{
        SCRIPT_REGISTER({1})
    }}

    void {1}::Initialize()
    {{

    }}

    void {1}::OnUpdate(float deltaTime)
    {{

    }}
}}";

        private static readonly string _namespace = GetNamespaceFromProjectName();

        public CreateScriptDiag()
        {
            InitializeComponent();

            Owner = Application.Current.MainWindow;
            scriptPathTextBox.Text = $@"{nameof(ProjectFileInfo.ProjectFolders.GameCode)}\";
        }

        private void On_scriptNameTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            IsValid();
        }

        private void On_scriptPathTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            IsValid();
        }

        private async void On_CreateButton_Click(object sender, RoutedEventArgs e)
        {
            if (!IsValid()) return;

            IsEnabled = false;

            string scriptName = scriptNameTextBox.Text.Trim();
            string scriptPath = Path.GetFullPath(Path.Combine(Project.CurrProject.ProjectDirPath, scriptPathTextBox.Text.Trim()));
            string solutionPath = Project.CurrProject.SolutionPath;
            string projectName = Project.CurrProject.ProjectName;

            await Task.Run(() => CreateScript(scriptName, scriptPath, solutionPath, projectName));

            Close();
        }

        private bool IsValid()
        {
            string name = scriptNameTextBox.Text.Trim();
            string path = scriptPathTextBox.Text.Trim();

            string projectDirPath = Project.CurrProject.ProjectDirPath;

            if(string.IsNullOrEmpty(name))
            {
                messageTextBlock.Text = "Script name cannot be empty";
                return false;
            }
            else if(name.IndexOfAny(Path.GetInvalidFileNameChars()) != -1 || name.Any(x => char.IsWhiteSpace(x)))
            {
                messageTextBlock.Text = "Invalid charactors used in the script name";
                return false;
            }
            else if(string.IsNullOrEmpty(name))
            {
                messageTextBlock.Text = "Script path cannot be empty";
                return false;
            }
            else if (path.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                messageTextBlock.Text = "Invalid charactors used in the script path";
                return false;
            }
            else if (!Path.GetFullPath(Path.Combine(projectDirPath, path)).Contains(Path.Combine(projectDirPath, nameof(ProjectFileInfo.ProjectFolders.GameCode))))
            {
                messageTextBlock.Text = $"Script must be added to (a subfolder of) {nameof(ProjectFileInfo.ProjectFolders.GameCode)}";
                return false;
            }
            else if (File.Exists(Path.GetFullPath(Path.Combine(Path.Combine(projectDirPath, path), $"{name}.h"))) ||
                File.Exists(Path.GetFullPath(Path.Combine(Path.Combine(projectDirPath, path), $"{name}.cpp"))))
            {
                messageTextBlock.Text = $"Script {name} already exists in this folder";
                return false;
            }

            messageTextBlock.Text = string.Empty;

            return true;
        }

        private void CreateScript(string scriptName, string scriptPath, string solutionPath, string projectName)
        {
            try
            {
                if (!Directory.Exists(scriptPath)) Directory.CreateDirectory(scriptPath);
            
                string headerFile = Path.GetFullPath(Path.Combine(scriptPath, $"{scriptName}.h"));
                string cppFile = Path.GetFullPath(Path.Combine(scriptPath, $"{scriptName}.cpp"));
                
                using (var sw = File.CreateText(headerFile))
                {
                    sw.Write(string.Format(_headerCode, _namespace, scriptName));
                }

                using (var sw = File.CreateText(cppFile))
                {
                    sw.Write(string.Format(_cppCode, _namespace, scriptName));
                }

                string[] headerAndCppFiles = new string[] { headerFile, cppFile };

                for (int i = 0; i < 3; ++i)
                {
                    if (!VisualStudio.AddFileToSolution(solutionPath, projectName, headerAndCppFiles)) System.Threading.Thread.Sleep(1000);
                    else break;
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error, $"Failed to create script {scriptNameTextBox.Text}");
            }
        }

        private static string GetNamespaceFromProjectName()
        {
            string projectName = Project.CurrProject.ProjectName;
            projectName = projectName.Replace(" ", "_");
            return projectName;
        }
    }
}
