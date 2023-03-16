using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;

namespace XunlanEditor.Utilities
{
    /// <summary>
    /// Provides the function of undoing and redoing
    /// </summary>
    public interface IUndoRedo
    {
        public string Name
        {
            get;
        }

        public void Undo();
        public void Redo();
    }

    public class UndoRedoAction : IUndoRedo
    {
        public string Name
        {
            get;
        }

        private Action _undo;
        private Action _redo;

        public void Undo() => _undo();
        public void Redo() => _redo();

        public UndoRedoAction(string name,Action undo,Action redo)
        {
            Debug.Assert(undo != null && redo != null);

            Name = name;
            _undo = undo;
            _redo = redo;
        }
    }

    static class UndoRedo
    {
        private static readonly ObservableCollection<IUndoRedo> _undoList = new ObservableCollection<IUndoRedo>();
        private static readonly ObservableCollection<IUndoRedo> _redoList = new ObservableCollection<IUndoRedo>();
        public static ReadOnlyObservableCollection<IUndoRedo> UndoList { get; } = new ReadOnlyObservableCollection<IUndoRedo>(_undoList);
        public static ReadOnlyObservableCollection<IUndoRedo> RedoList { get; } = new ReadOnlyObservableCollection<IUndoRedo>(_redoList);

        /// <summary>
        /// Prevents <see cref="AddUndoRedoAction"/> being called indirectly by <see cref="Undo"/> or <see cref="Redo"/>
        /// </summary>
        private static bool _isEnabled = true;

        /// <summary>
        /// Adds a <see cref="IUndoRedo"/> command that supports undo and redo
        /// </summary>
        /// <param name="cmd"></param>
        public static void AddUndoRedoAction(IUndoRedo cmd)
        {
            if(_isEnabled)
            {
                _undoList.Add(cmd);
                _redoList.Clear();
            }
        }
        public static void Undo()
        {
            if(_undoList.Any())
            {
                IUndoRedo cmd = _undoList.Last();
                _undoList.RemoveAt(_undoList.Count - 1);

                _isEnabled = false;
                cmd.Undo();
                _isEnabled = true;

                _redoList.Insert(0,cmd);
            }
        }
        public static void Redo()
        {
            if(_redoList.Any())
            {
                IUndoRedo cmd = _redoList.First();
                _redoList.RemoveAt(0);

                _isEnabled = false;
                cmd.Redo();
                _isEnabled = true;

                _undoList.Add(cmd);
            }
        }
        public static void Reset()
        {
            _undoList.Clear();
            _redoList.Clear();
        }
    }
}