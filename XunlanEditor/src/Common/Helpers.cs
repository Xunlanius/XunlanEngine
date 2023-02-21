using System.IO;
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
    }
}
