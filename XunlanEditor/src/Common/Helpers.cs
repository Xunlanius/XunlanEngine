using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using XunlanEditor.Content;
using XunlanEditor.src.Content;
using XunlanEditor.Utilities;

namespace XunlanEditor
{
    static class AssetHelper
    {
        public static bool IsDirectory(string path)
        {
            try
            {
                return File.GetAttributes(path).HasFlag(FileAttributes.Directory);
            }
            catch(Exception ex) { Debug.WriteLine(ex.Message); }
            return false;
        }

        public static string GetRandomString(int len = 8)
        {
            if(len <= 0)
                len = 8;

            int n = len / 11;
            StringBuilder stringBuilder = new StringBuilder();

            for(int i = 0;i <= n;++i)
            {
                stringBuilder.Append(Path.GetRandomFileName().Replace(".",""));
            }
            return stringBuilder.ToString(0,len);
        }

        public static string SanitizeFileName(string fileName)
        {
            StringBuilder path = new StringBuilder(fileName.Substring(0,fileName.LastIndexOf(Path.DirectorySeparatorChar) + 1));
            StringBuilder file = new StringBuilder(fileName[(fileName.LastIndexOf(Path.DirectorySeparatorChar) + 1)..]);

            foreach(char c in Path.GetInvalidPathChars())
            {
                path.Replace(c,'_');
            }
            foreach(char c in Path.GetInvalidFileNameChars())
            {
                file.Replace(c,'_');
            }

            return path.Append(file).ToString();
        }

        public static byte[] ComputeHash(byte[] data,int offset = 0,int byteSize = 0)
        {
            if(data == null || data.Length == 0)
                return null;

            using var sha256 = SHA256.Create();
            return sha256.ComputeHash(data,offset,byteSize > 0 ? byteSize : data.Length);
        }

        public static async Task ImportFilesAsync(string[] files,string dst)
        {
            try
            {
                Debug.Assert(!string.IsNullOrEmpty(dst));
                AssetWatcher.EnableFileWatcher(false);
                var tasks = files.Select(async file => await Task.Run(() => { ImportFile(file,dst); }));
                await Task.WhenAll(tasks);
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.LogMessage(MsgType.Error,$"Failed to import file(s) to [{dst}].");
            }
            finally
            {
                AssetWatcher.EnableFileWatcher(true);
            }
        }

        private static void ImportFile(string filePath,string dst)
        {
            Debug.Assert(!string.IsNullOrEmpty(filePath));
            if(IsDirectory(filePath))
                return;
            if(!dst.EndsWith(Path.DirectorySeparatorChar))
                dst += Path.DirectorySeparatorChar;

            string name = Path.GetFileNameWithoutExtension(filePath).ToLower();
            string extension = Path.GetExtension(filePath).ToLower();

            Asset asset = null;

            switch(extension)
            {
                case ".fbx":
                    asset = new Geometry();
                    break;
                default:
                    break;
            }

            if(asset == null)
                return;

            asset.FilePath = dst + name + Asset.AssetFileExtension;
            asset.Import(filePath);
            asset.Save(asset.FilePath);
        }
    }
}
