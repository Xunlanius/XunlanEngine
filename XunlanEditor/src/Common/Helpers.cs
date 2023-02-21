using System;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Windows;
using System.Windows.Media;

namespace XunlanEditor
{
    static class VisualExtensions
    {
        public static T FindVisualParent<T>(this DependencyObject dependencyObject) where T : DependencyObject
        {
            if (!(dependencyObject is Visual)) return null;

            DependencyObject parent = VisualTreeHelper.GetParent(dependencyObject);
            while (parent != null)
            {
                if (parent is T type) return type;
                parent = VisualTreeHelper.GetParent(parent);
            }

            return null;
        }
    }

    static class AssetHelper
    {
        public static string GetRandomString(int len = 8)
        {
            if (len <= 0) len = 8;

            int n = len / 11;
            StringBuilder stringBuilder = new StringBuilder();

            for (int i = 0; i <= n; ++i)
            {
                stringBuilder.Append(Path.GetRandomFileName().Replace(".", ""));
            }
            return stringBuilder.ToString(0, len);
        }

        public static string SanitizeFileName(string fileName)
        {
            StringBuilder path = new StringBuilder(fileName.Substring(0, fileName.LastIndexOf(Path.DirectorySeparatorChar) + 1));
            StringBuilder file = new StringBuilder(fileName[(fileName.LastIndexOf(Path.DirectorySeparatorChar) + 1)..]);

            foreach (char c in Path.GetInvalidPathChars())
            {
                path.Replace(c, '_');
            }
            foreach (char c in Path.GetInvalidFileNameChars())
            {
                file.Replace(c, '_');
            }

            return path.Append(file).ToString();
        }

        public static byte[] ComputeHash(byte[] data, int offset = 0, int byteSize = 0)
        {
            if (data == null || data.Length == 0)
            {
                return null;
            }

            using var sha256 = SHA256.Create();
            return sha256.ComputeHash(data, offset, byteSize > 0 ? byteSize : data.Length);
        }
    }
}
