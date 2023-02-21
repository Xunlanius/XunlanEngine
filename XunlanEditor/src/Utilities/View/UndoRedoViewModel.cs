using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace XunlanEditor.Utilities
{
    /// <summary>
    /// Provides the function of undoing and redoing
    /// </summary>
    public interface IUndoRedo
    {
        public string Name { get; }

        public void Undo();
        public void Redo();
    }

    public class UndoRedoAction : IUndoRedo
    {
        public string Name { get; }

        private Action _undo;
        private Action _redo;

        public void Undo() => _undo();
        public void Redo() => _redo();

        public UndoRedoAction(string name, Action undo, Action redo)
        {
            Debug.Assert(undo != null && redo != null);

            Name = name;
            _undo = undo;
            _redo = redo;
        }
    }

    public class UndoRedo
    {
        private readonly ObservableCollection<IUndoRedo> _undoList = new ObservableCollection<IUndoRedo>();
        private readonly ObservableCollection<IUndoRedo> _redoList = new ObservableCollection<IUndoRedo>();
        public ReadOnlyObservableCollection<IUndoRedo> UndoList { get; }
        public ReadOnlyObservableCollection<IUndoRedo> RedoList { get; }

        /// <summary>
        /// Prevents <see cref="AddUndoRedoAction"/> being called indirectly by <see cref="Undo"/> or <see cref="Redo"/>
        /// </summary>
        private bool _isEnabled = true;

        public UndoRedo()
        {
            UndoList = new ReadOnlyObservableCollection<IUndoRedo>(_undoList);
            RedoList = new ReadOnlyObservableCollection<IUndoRedo>(_redoList);
        }

        /// <summary>
        /// Adds a <see cref="IUndoRedo"/> command that supports undo and redo
        /// </summary>
        /// <param name="cmd"></param>
        public void AddUndoRedoAction(IUndoRedo cmd)
        {
            if (_isEnabled)
            {
                _undoList.Add(cmd);
                _redoList.Clear();
            }
        }
        public void Undo()
        {
            if (_undoList.Any())
            {
                IUndoRedo cmd = _undoList.Last();
                _undoList.RemoveAt(_undoList.Count - 1);

                _isEnabled = false;
                cmd.Undo();
                _isEnabled = true;

                _redoList.Insert(0, cmd);
            }
        }
        public void Redo()
        {
            if (_redoList.Any())
            {
                IUndoRedo cmd = _redoList.First();
                _redoList.RemoveAt(0);

                _isEnabled = false;
                cmd.Redo();
                _isEnabled = true;

                _undoList.Add(cmd);
            }
        }
        public void Reset()
        {
            _undoList.Clear();
            _redoList.Clear();
        }
    }
}