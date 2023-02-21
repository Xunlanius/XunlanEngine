using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using XunlanEditor.Utilities;

namespace XunlanEditor.GameProject
{
    [DataContract]
    class ProjectTemplate
    {
        public enum TemplateType
        {
            Empty,
            FirstPerson,
            ThirdPerson,
        }

        private static readonly string _templateFileName = "template.xml";
        private static readonly string _projectFileName = $"project{ProjectFileInfo.ProjectFileSuffix}";

        public static string TemplateRootPath { get; } = Path.Combine(ProjectFileInfo.EnginePath, @"XunlanEditor\ProjectTemplates");

        [DataMember(Order = 0)]
        public TemplateType Type { get; set; }

        [DataMember(Order = 1)]
        public string TemplateDirPath { get; set; }

        public string TemplateFilePath { get => Path.Combine(TemplateDirPath, _templateFileName); }
        public string TemplateProjectFilePath { get => Path.Combine(TemplateDirPath, _projectFileName); }

        public static bool CheckTemplateFileIntegrity()
        {
            try
            {
                foreach (string templateName in Enum.GetNames(typeof(TemplateType)))
                {
                    string templateDirPath = Path.Combine(TemplateRootPath, templateName);

                    if (!Directory.Exists(templateDirPath) ||
                        !File.Exists(Path.Combine(templateDirPath, _templateFileName)) ||
                        !File.Exists(Path.Combine(templateDirPath, _projectFileName))) return false;
                }

                return true;
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error, $"Failed to check template file integrity");

                Debugger.Break();

                throw;
            }
        }
        public static void CreateTemplateFile()
        {
            try
            {
                int index = 0;

                foreach (string templateName in Enum.GetNames(typeof(TemplateType)))
                {
                    string templateDirPath = Path.Combine(TemplateRootPath, templateName);
                    if (!Directory.Exists(templateDirPath)) Directory.CreateDirectory(templateDirPath);

                    // Create and serialize
                    ProjectTemplate template = new ProjectTemplate()
                    {
                        Type = (TemplateType)index,
                        TemplateDirPath = templateDirPath
                    };
                    Serializer.ObjectToFile(template, Path.Combine(templateDirPath, _templateFileName));

                    Project project = new Project("{0}", "{1}");
                    Serializer.ObjectToFile(project, Path.Combine(templateDirPath, _projectFileName));

                    ++index;
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error, $"Failed to generate template file");

                Debugger.Break();
                throw;
            }
        }
    }
}