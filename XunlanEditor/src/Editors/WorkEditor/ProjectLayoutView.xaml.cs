using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using XunlanEditor.GameObjects;
using XunlanEditor.GameProject;
using XunlanEditor.Utilities;

namespace XunlanEditor.Editors
{
    /// <summary>
    /// ProjectLayoutView.xaml 的交互逻辑
    /// </summary>
    public partial class ProjectLayoutView : UserControl
    {
        public ProjectLayoutView()
        {
            InitializeComponent();
        }

        private void OnAddGameObject_Button_Click(object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            Scene scene = button.DataContext as Scene;

            GameObject gameObject = new GameObject("Empty", scene);

            if (scene.AddGameObjectCommand.CanExecute(gameObject))
            {
                scene.AddGameObjectCommand.Execute(gameObject);
            }
        }

        private void OnGameObjectChanged_ListBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ListBox listBox = sender as ListBox;
            
            IEnumerable<GameObject> addedItems = e.AddedItems.Cast<GameObject>();
            IEnumerable<GameObject> removedItems = e.RemovedItems.Cast<GameObject>();

            List<GameObject> currSelectedItems = listBox.SelectedItems.Cast<GameObject>().ToList();
            List<GameObject> prevSelectedItems = currSelectedItems.Except(addedItems).Concat(removedItems).ToList();

            Project.CurrProject.UndoRedo.AddUndoRedoAction(new UndoRedoAction(
                "Object selection changed",
                () =>
                {
                    listBox.UnselectAll();
                    prevSelectedItems.ForEach(item =>
                    {
                        ListBoxItem listBoxItem = listBox.ItemContainerGenerator.ContainerFromItem(item) as ListBoxItem;
                        listBoxItem.IsSelected = true;
                    });
                },
                () =>
                {
                    listBox.UnselectAll();
                    currSelectedItems.ForEach(item =>
                    {
                        ListBoxItem listBoxItem = listBox.ItemContainerGenerator.ContainerFromItem(item) as ListBoxItem;
                        listBoxItem.IsSelected = true;
                    });
                }));

            MultiGameObject multiGameObject = null;
            if (currSelectedItems.Any())
            {
                multiGameObject = new MultiGameObject(currSelectedItems);
            }
            GameObjectView.Instance.DataContext = multiGameObject;
        }
    }
}
