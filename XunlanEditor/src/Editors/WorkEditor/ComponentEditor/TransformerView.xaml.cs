using System;
using System.Diagnostics;
using System.Linq;
using System.Numerics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using XunlanEditor.GameObjects;
using XunlanEditor.Utilities;

namespace XunlanEditor.Editors
{
    /// <summary>
    /// TransformerView.xaml 的交互逻辑
    /// </summary>
    public partial class TransformerView : UserControl
    {
        private Action _undoAction = null;

        private bool _propertyChanged = false;

        public TransformerView()
        {
            InitializeComponent();
            Loaded += OnTransformerViewLoaded;
        }

        private void OnTransformerViewLoaded(object sender,RoutedEventArgs e)
        {
            Loaded -= OnTransformerViewLoaded;
            (DataContext as MultiTransformer).PropertyChanged += (sender,e) => _propertyChanged = true;
        }

        private Action GetAction(
            Func<Transformer,(Transformer transformer, Vector3)> selector,
            Action<(Transformer transformer, Vector3)> forEachAction)
        {
            if(!(DataContext is MultiTransformer viewModel))
            {
                _undoAction = null;
                _propertyChanged = false;
                return null;
            }

            var selection = viewModel.SelectedComponents.Select(selector).ToList();
            return new Action(() =>
            {
                selection.ForEach(forEachAction);
                //(GameObjectView.Instance.DataContext as MultiObject)?.GetMultiComponent<MultiTransformer>().Refresh();
                viewModel.Refresh();
            });
        }

        private Action GetPositionAction() => GetAction(x => (x, x.Position),x => x.transformer.Position = x.Item2);
        private Action GetRotationAction() => GetAction(x => (x, x.Rotation),x => x.transformer.Rotation = x.Item2);
        private Action GetScaleAction() => GetAction(x => (x, x.Scale),x => x.transformer.Scale = x.Item2);

        private void RecordActions(Action redoAction,string name)
        {
            if(_propertyChanged)
            {
                Debug.Assert(_undoAction != null);

                _propertyChanged = false;
                UndoRedo.AddUndoRedoAction(new UndoRedoAction(name,_undoAction,redoAction));

                _undoAction = null;
            }
        }

        private void OnPositionChanged_VectorBox_PreviewMouseLBD(object sender,MouseButtonEventArgs e)
        {
            _propertyChanged = false;
            _undoAction = GetPositionAction();
        }
        private void OnPositionChanged_VectorBox_PreviewMouseLBU(object sender,MouseButtonEventArgs e)
        {
            RecordActions(GetPositionAction(),"Position Changed");
        }

        private void OnRotationChanged_VectorBox_PreviewMouseLBD(object sender,MouseButtonEventArgs e)
        {
            _propertyChanged = false;
            _undoAction = GetRotationAction();
        }
        private void OnRotationChanged_VectorBox_PreviewMouseLBU(object sender,MouseButtonEventArgs e)
        {
            RecordActions(GetRotationAction(),"Rotation Changed");
        }

        private void OnScaleChanged_VectorBox_PreviewMouseLBD(object sender,MouseButtonEventArgs e)
        {
            _propertyChanged = false;
            _undoAction = GetScaleAction();
        }
        private void OnScaleChanged_VectorBox_PreviewMouseLBU(object sender,MouseButtonEventArgs e)
        {
            RecordActions(GetScaleAction(),"Scale Changed");
        }

        private void OnPositionChanged_VectorBox_LostKeyboardFocus(object sender,KeyboardFocusChangedEventArgs e)
        {
            if(_propertyChanged && _undoAction != null)
            {
                OnPositionChanged_VectorBox_PreviewMouseLBU(sender,null);
            }
        }
        private void OnRotationChanged_VectorBox_LostKeyboardFocus(object sender,KeyboardFocusChangedEventArgs e)
        {
            if(_propertyChanged && _undoAction != null)
            {
                OnRotationChanged_VectorBox_PreviewMouseLBU(sender,null);
            }
        }
        private void OnScaleChanged_VectorBox_LostKeyboardFocus(object sender,KeyboardFocusChangedEventArgs e)
        {
            if(_propertyChanged && _undoAction != null)
            {
                OnScaleChanged_VectorBox_PreviewMouseLBU(sender,null);
            }
        }
    }
}
