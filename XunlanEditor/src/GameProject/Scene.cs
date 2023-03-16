using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Windows.Input;
using XunlanEditor.GameObjects;
using XunlanEditor.Utilities;

namespace XunlanEditor.GameProject
{
    [DataContract]
    class Scene : ViewModelBase
    {
        [DataMember(Order = 0,Name = nameof(Name))]
        private string _name = "Default scene";
        public string Name
        {
            get => _name;
            set
            {
                if(_name == value) return;
                _name = value;
                OnPropertyChanged(nameof(Name));
            }
        }

        [DataMember(Order = 1)]
        public Project ParentProject { get; private set; }

        [DataMember(Name = "IsActive",Order = 2)]
        private bool _isActive = false;
        public bool IsActive
        {
            get => _isActive;
            set
            {
                if(_isActive == value) return;
                _isActive = value;
                OnPropertyChanged(nameof(IsActive));
            }
        }

        [DataMember(Name = nameof(GameObjectList),Order = 3)]
        private readonly ObservableCollection<GameObject> _gameObjectList = new ObservableCollection<GameObject>();
        public ReadOnlyObservableCollection<GameObject> GameObjectList { get; private set; }

        public ICommand AddGameObjectCommand { get; private set; }
        public ICommand RemoveGameObjectCommand { get; private set; }

        public Scene(Project parentProject,string sceneName)
        {
            Debug.Assert(!string.IsNullOrEmpty(sceneName.Trim()));

            Name = sceneName;
            ParentProject = parentProject;

            OnDeserialized(new StreamingContext());
        }

        [OnDeserialized]
        private void OnDeserialized(StreamingContext context)
        {
            GameObjectList = new ReadOnlyObservableCollection<GameObject>(_gameObjectList);
            OnPropertyChanged(nameof(GameObjectList));

            foreach(GameObject obj in GameObjectList)
            {
                obj.IsAlive = IsActive;
            }

            AddGameObjectCommand = new RelayCommand<GameObject>(obj =>
            {
                Debug.Assert(!GameObjectList.Contains(obj));

                AddGameObject(obj);

                UInt32 index = (UInt32)_gameObjectList.Count - 1;

                UndoRedo.AddUndoRedoAction(new UndoRedoAction(
                    $"Create Entity: {obj.Name}",
                    () => RemoveGameObject(obj),
                    () => AddGameObject(obj,index)));
            });
            RemoveGameObjectCommand = new RelayCommand<GameObject>(obj =>
            {
                Debug.Assert(GameObjectList.Contains(obj));

                UInt32 index = (UInt32)_gameObjectList.IndexOf(obj);
                RemoveGameObject(obj);

                UndoRedo.AddUndoRedoAction(new UndoRedoAction(
                    $"Remove Entity: {obj.Name}",
                    () => AddGameObject(obj,index),
                    () => RemoveGameObject(obj)));
            });
        }

        public void Unload()
        {
            foreach(GameObject obj in GameObjectList)
            {
                obj.IsAlive = false;
            }
        }

        private void AddGameObject(GameObject obj,UInt32 index = ID.InvalidID)
        {
            obj.IsAlive = true;

            if(index == ID.InvalidID)
                _gameObjectList.Add(obj);
            else
                _gameObjectList.Insert((int)index,obj);

            Logger.LogMessage(MsgType.Info,$"Create Entity: {obj.ObjectID}");
        }
        private void RemoveGameObject(GameObject obj)
        {
            UInt32 id = obj.ObjectID;

            obj.IsAlive = false;
            _gameObjectList.Remove(obj);

            Logger.LogMessage(MsgType.Info,$"Remove Entity: {id}");
        }
    }
}