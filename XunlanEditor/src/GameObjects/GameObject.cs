using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;
using XunlanEditor.DLLInterface;
using XunlanEditor.GameProject;
using XunlanEditor.Utilities;

namespace XunlanEditor.GameObjects
{
    [DataContract]
    class GameObject : ViewModelBase
    {
        private UInt32 _objectID = ID.InvalidID;
        public UInt32 ObjectID
        {
            get => _objectID;
            private set
            {
                if (_objectID == value) return;

                _objectID = value;
                OnPropertyChanged(nameof(ObjectID));
            }
        }

        private bool _isAlive = false;
        public bool IsAlive
        {
            get => _isAlive;
            set
            {
                if (_isAlive == value) return;

                _isAlive = value;

                if (_isAlive)
                {
                    ObjectID = EngineAPI.GameObjectAPI.CreateEntity(this);
                    Debug.Assert(ID.IsValid(ObjectID));
                }
                else if (ID.IsValid(ObjectID))
                {
                    EngineAPI.GameObjectAPI.RemoveEntity(this);
                    ObjectID = ID.InvalidID;
                }

                OnPropertyChanged(nameof(IsAlive));
            }
        }

        [DataMember(Name = nameof(Name), Order = 0)]
        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if (_name == value) return;

                _name = value;
                OnPropertyChanged(nameof(Name));
            }
        }

        [DataMember(Name = nameof(IsEnabled), Order=1)]
        private bool _isEnabled = true;
        public bool IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled == value) return;

                _isEnabled = value;
                OnPropertyChanged(nameof(IsEnabled));
            }
        }

        [DataMember(Order = 2)]
        public Scene ParentScene { get; private set; }

        [DataMember(Name = nameof(ComponentList), Order = 3)]
        private readonly ObservableCollection<Component> _componentList = new ObservableCollection<Component>();
        public ReadOnlyObservableCollection<Component> ComponentList { get; private set; }

        public GameObject(string name, Scene parentScene)
        {
            Name = name;
            ParentScene = parentScene;
            _componentList.Add(new Transformer(this));

            OnDeserialized(new StreamingContext());
        }

        [OnDeserialized]
        private void OnDeserialized(StreamingContext context)
        {
            ComponentList = new ReadOnlyObservableCollection<Component>(_componentList);
            OnPropertyChanged(nameof(ComponentList));
        }

        public bool AddComponent(Component component)
        {
            Debug.Assert(component != null);

            if (ComponentList.Any(com => com.GetType() == component.GetType()))
            {
                Logger.LogMessage(MsgType.Warning, $"GameObject [{Name}] already has a [{component.GetType().Name}] component.");
                return false;
            }

            IsAlive = false;
            _componentList.Add(component);
            IsAlive = true;

            return true;
        }
        public void RemoveComponent(Component component)
        {
            Debug.Assert(component != null);

            // Transformer cannot be removed
            if (component is Transformer) return;

            if (_componentList.Contains(component))
            {
                IsAlive = false;
                _componentList.Remove(component);
                IsAlive = true;
            }
        }

        public T GetComponent<T>() where T : Component => _componentList.FirstOrDefault(x => x.GetType() == typeof(T)) as T;
        public Component GetComponent(Type type) => _componentList.FirstOrDefault(x => x.GetType() == type);
    }

    abstract class MultiObject : ViewModelBase
    {
        /// <summary>
        /// Enables update to selected GameObjects
        /// </summary>
        private bool _enableUpdate = true;

        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if (_name == value) return;

                _name = value;
                OnPropertyChanged(nameof(Name));
            }
        }

        /// <summary>
        /// <see cref="Nullable"/>: null indicates the mixed values
        /// </summary>
        private bool? _isEnabled = true;
        public bool? IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled == value) return;

                _isEnabled = value;
                OnPropertyChanged(nameof(IsEnabled));
            }
        }

        public List<GameObject> SelectedObjects { get; }

        private readonly ObservableCollection<IMultiComponent> _componentList = new ObservableCollection<IMultiComponent>();
        public ReadOnlyObservableCollection<IMultiComponent> ComponentList { get; private set; }

        public MultiObject(List<GameObject> selectedObjects)
        {
            Debug.Assert(selectedObjects.Any());

            ComponentList = new ReadOnlyObservableCollection<IMultiComponent>(_componentList);
            SelectedObjects = selectedObjects;

            PropertyChanged += (sender, e) =>
            {
                if (_enableUpdate) UpdateSelectedObjects(e.PropertyName);
            };
        }

        public T GetMultiComponent<T>() where T : IMultiComponent
        {
            return (T)ComponentList.FirstOrDefault(x => x.GetType() == typeof(T));
        }

        public void ReFresh()
        {
            _enableUpdate = false;

            UpdateProperties();
            CreateComponentList();

            _enableUpdate = true;
        }

        protected bool UpdateProperties()
        {
            Name = GetMixedValue(SelectedObjects, new Func<GameObject, string>(x => x.Name));
            IsEnabled = GetMixedValue(SelectedObjects, new Func<GameObject, bool>(x => x.IsEnabled));

            return true;
        }

        private void CreateComponentList()
        {
            _componentList.Clear();

            var firstObject = SelectedObjects.First();
            if (firstObject == null) return;

            foreach (Component component in firstObject.ComponentList)
            {
                Type type = component.GetType();

                if(!SelectedObjects.Skip(1).Any(obj => obj.GetComponent(type) == null))
                {
                    _componentList.Add(component.GetMultiComponent(this));
                }
            }
        }

        public static bool? GetMixedValue<T>(List<T> objectList, Func<T, bool> getProperty)
        {
            bool value = getProperty(objectList.First());
            return objectList.Any(obj => getProperty(obj) != value) ? (bool?)null : value;
        }
        public static float? GetMixedValue<T>(List<T> objectList, Func<T, float> getProperty)
        {
            float value = getProperty(objectList.First());
            return objectList.Any(obj => !getProperty(obj).IsEqual(value)) ? (float?)null : value;
        }
        public static string GetMixedValue<T>(List<T> objectList, Func<T, string> getProperty)
        {
            string value = getProperty(objectList.First());
            return objectList.Any(obj => getProperty(obj) != value) ? null : value;
        }

        protected virtual bool UpdateSelectedObjects(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(Name):
                    SelectedObjects.ForEach(obj => obj.Name = Name);
                    return true;
                case nameof(IsEnabled):
                    SelectedObjects.ForEach(obj => obj.IsEnabled = IsEnabled.Value);
                    return true;
            }
            return false;
        }
    }

    class MultiGameObject : MultiObject
    {
        public MultiGameObject(List<GameObject> objectList)
            : base(objectList)
        {
            ReFresh();
        }
    }
}
