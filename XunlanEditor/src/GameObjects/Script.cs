using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Text;

namespace XunlanEditor.GameObjects
{
    [DataContract]
    class Script : Component
    {
        private string _scriptName;
        [DataMember(Order = 0)]
        public string ScriptName
        {
            get => _scriptName;
            set
            {
                if (_scriptName == value) return;

                _scriptName = value;
                OnPropertyChanged(nameof(ScriptName));
            }
        }

        public Script(GameObject parent)
            : base(parent) {}

        public override IMultiComponent GetMultiComponent(MultiObject multiObject)
        {
            return new MultiScript(multiObject);
        }

        public override void WriteToBinary(BinaryWriter bw)
        {
            var nameByte = Encoding.UTF8.GetBytes(ScriptName);
            bw.Write(nameByte.Length);
            bw.Write(nameByte);
        }
    }

    sealed class MultiScript : MultiComponent<Script>
    {
        private string _scriptName;
        public string ScriptName
        {
            get => _scriptName;
            set
            {
                if (_scriptName == value) return;

                _scriptName = value;
                OnPropertyChanged(nameof(ScriptName));
            }
        }

        public MultiScript(MultiObject multiObject)
            : base(multiObject)
        {
            Refresh();
        }

        protected override bool UpdateProperties()
        {
            ScriptName = MultiObject.GetMixedValue(SelectedComponents, new Func<Script, string>(x => x.ScriptName));
            return true;
        }

        protected override bool UpdateSelectedComponents(string propertyName)
        {
            if (propertyName != ScriptName) return false;

            SelectedComponents.ForEach(component => component.ScriptName = ScriptName);
            return true;
        }
    }
}
