using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using XunlanEditor.GameObjects;
using XunlanEditor.Utilities;

namespace XunlanEditor.Editors
{
    /// <summary>
    /// GameObjectView.xaml 的交互逻辑
    /// </summary>
    public partial class GameObjectView : UserControl
    {
        public static GameObjectView Instance
        {
            get; private set;
        }

        /// <summary>
        /// Name of the changed property
        /// </summary>
        private string _propertyName;

        private Action _undoAction;
        private Action _redoAction;

        public GameObjectView()
        {
            InitializeComponent();
            Instance = this;
            DataContext = null;
            DataContextChanged += (_,__) =>
            {
                if(DataContext != null)
                {
                    (DataContext as MultiObject).PropertyChanged += (sender,e) => _propertyName = e.PropertyName;
                }
            };
        }

        private Action GetRenameAction()
        {
            MultiObject viewModel = DataContext as MultiObject;

            // Record the previous "Name" of the "GameObject"
            var selection = viewModel.SelectedObjects.Select(obj => (obj, obj.Name)).ToList();

            return new Action(() =>
            {
                selection.ForEach(item => item.obj.Name = item.Name);
                viewModel.ReFresh();
            });
        }

        private Action GetIsEnabledAction()
        {
            MultiObject viewModel = DataContext as MultiObject;

            var selection = viewModel.SelectedObjects.Select(obj => (obj, obj.IsEnabled)).ToList();

            return new Action(() =>
            {
                selection.ForEach(item => item.obj.IsEnabled = item.IsEnabled);
                viewModel.ReFresh();
            });
        }

        private void OnRename_TextBox_GotKeyboardFocus(object sender,KeyboardFocusChangedEventArgs e)
        {
            _propertyName = string.Empty;
            _undoAction = GetRenameAction();
        }

        private void OnRename_TextBox_LostKeyboardFocus(object sender,KeyboardFocusChangedEventArgs e)
        {
            if(_propertyName == nameof(MultiObject.Name) && _undoAction != null)
            {
                _redoAction = GetRenameAction();

                UndoRedo.AddUndoRedoAction(new UndoRedoAction("Rename GameObject",_undoAction,_redoAction));

                _propertyName = null;
            }

            _undoAction = null;
            _redoAction = null;
        }

        private void OnIsEnabled_CheckBox_Click(object sender,RoutedEventArgs e)
        {
            MultiObject viewModel = DataContext as MultiObject;

            Action undoAction = GetIsEnabledAction();
            viewModel.IsEnabled = (sender as CheckBox).IsChecked == true;
            Action redoAction = GetIsEnabledAction();

            UndoRedo.AddUndoRedoAction(new UndoRedoAction("Enable/Disable Object",undoAction,redoAction));
        }

        private void OnAddComponent_Button_PreviewMouse_LBD(object sender,MouseButtonEventArgs e)
        {
            ContextMenu menu = FindResource("addComponentMenu") as ContextMenu;
            ToggleButton btn = sender as ToggleButton;

            btn.IsChecked = true;
            menu.Placement = PlacementMode.Bottom;
            menu.PlacementTarget = btn;
            menu.MinWidth = ActualWidth;
            menu.IsOpen = true;
        }

        private void OnAddScriptComponent(object sender,RoutedEventArgs e)
        {
            AddComponent(ComponentType.Script,(sender as MenuItem).Header.ToString());
        }

        private void AddComponent(ComponentType componentType,object data)
        {
            var creationFunc = ComponentFactory.GetCreationFunc(componentType);
            var changedGameObjects = new List<(GameObject gameObject, Component component)>();
            MultiObject vm = DataContext as MultiObject;

            foreach(GameObject gameObject in vm.SelectedObjects)
            {
                Component component = creationFunc(gameObject,data);
                if(gameObject.AddComponent(component))
                    changedGameObjects.Add((gameObject, component));
            }

            if(changedGameObjects.Any())
            {
                vm.ReFresh();

                UndoRedo.AddUndoRedoAction(new UndoRedoAction(
                    $"Add {componentType} component",
                    () =>
                    {
                        changedGameObjects.ForEach(x => x.gameObject.RemoveComponent(x.component));
                        vm.ReFresh();
                    },
                    () =>
                    {
                        changedGameObjects.ForEach(x => x.gameObject.AddComponent(x.component));
                        vm.ReFresh();
                    }));
            }
        }
    }
}
