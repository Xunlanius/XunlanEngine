using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;

namespace XunlanEditor.GameObjects
{
    interface IMultiComponent
    {
    }

    [DataContract]
    [KnownType(typeof(Transformer))]
    [KnownType(typeof(Script))]
    abstract class Component : ViewModelBase
    {
        public abstract IMultiComponent GetMultiComponent(MultiObject multiObject);

        public abstract void WriteToBinary(BinaryWriter bw);

        [DataMember(Order = 0)]
        public GameObject Parent
        {
            get; private set;
        }

        public Component(GameObject parent)
        {
            Debug.Assert(parent != null);
            Parent = parent;
        }
    }

    abstract class MultiComponent<T> : ViewModelBase, IMultiComponent where T : Component
    {
        private bool _enableUpdate = true;

        public List<T> SelectedComponents
        {
            get; private set;
        }

        public MultiComponent(MultiObject multiObject)
        {
            Debug.Assert(multiObject?.SelectedObjects?.Any() == true);

            SelectedComponents = multiObject.SelectedObjects.Select(obj => obj.GetComponent<T>()).ToList();

            PropertyChanged += (sender,e) =>
            {
                if(_enableUpdate) UpdateSelectedComponents(e.PropertyName);
            };
        }

        public void Refresh()
        {
            _enableUpdate = false;
            UpdateProperties();
            _enableUpdate = true;
        }

        protected abstract bool UpdateProperties();
        protected abstract bool UpdateSelectedComponents(string propertyName);
    }
}
