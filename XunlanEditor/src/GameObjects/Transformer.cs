using System;
using System.Collections.Generic;
using System.IO;
using System.Numerics;
using System.Runtime.Serialization;
using System.Text;
using XunlanEditor.Utilities;

namespace XunlanEditor.GameObjects
{
    [DataContract]
    class Transformer : Component
    {
        [DataMember(Name = nameof(Position), Order = 0)]
        private Vector3 _position = Vector3.Zero;
        public Vector3 Position
        {
            get => _position;
            set
            {
                if(_position != value)
                {
                    _position = value;
                    OnPropertyChanged(nameof(Position));
                }
            }
        }

        [DataMember(Name = nameof(Rotation), Order = 1)]
        private Vector3 _rotation = Vector3.Zero;
        public Vector3 Rotation
        {
            get => _rotation;
            set
            {
                if(_rotation != value)
                {
                    _rotation = value;
                    OnPropertyChanged(nameof(Rotation));
                }
            }
        }

        [DataMember(Name = nameof(Scale), Order = 2)]
        private Vector3 _scale = Vector3.One;
        public Vector3 Scale
        {
            get => _scale;
            set
            {
                if(_scale != value)
                {
                    _scale = value;
                    OnPropertyChanged(nameof(Scale));
                }
            }
        }

        public Transformer(GameObject parent)
            : base(parent) {}

        public override IMultiComponent GetMultiComponent(MultiObject multiObject)
        {
            return new MultiTransformer(multiObject);
        }

        public override void WriteToBinary(BinaryWriter bw)
        {
            bw.Write(_position.X);
            bw.Write(_position.Y);
            bw.Write(_position.Z);
            bw.Write(_rotation.X);
            bw.Write(_rotation.Y);
            bw.Write(_rotation.Z);
            bw.Write(_scale.X);
            bw.Write(_scale.Y);
            bw.Write(_scale.Z);
        }
    }

    sealed class MultiTransformer : MultiComponent<Transformer>
    {
        private float? _posX;
        public float? PosX
        {
            get => _posX;
            set
            {
                if (!_posX.IsEqual(value))
                {
                    _posX = value;
                    OnPropertyChanged(nameof(PosX));
                }
            }
        }

        private float? _posY;
        public float? PosY
        {
            get => _posY;
            set
            {
                if (!_posY.IsEqual(value))
                {
                    _posY = value;
                    OnPropertyChanged(nameof(PosY));
                }
            }
        }

        private float? _posZ;
        public float? PosZ
        {
            get => _posZ;
            set
            {
                if (!_posZ.IsEqual(value))
                {
                    _posZ = value;
                    OnPropertyChanged(nameof(PosZ));
                }
            }
        }

        private float? _rotX;
        public float? RotX
        {
            get => _rotX;
            set
            {
                if (!_rotX.IsEqual(value))
                {
                    _rotX = value;
                    OnPropertyChanged(nameof(RotX));
                }
            }
        }

        private float? _rotY;
        public float? RotY
        {
            get => _rotY;
            set
            {
                if (!_rotY.IsEqual(value))
                {
                    _rotY = value;
                    OnPropertyChanged(nameof(RotY));
                }
            }
        }

        private float? _rotZ;
        public float? RotZ
        {
            get => _rotZ;
            set
            {
                if (!_rotZ.IsEqual(value))
                {
                    _rotZ = value;
                    OnPropertyChanged(nameof(RotZ));
                }
            }
        }

        private float? _scaleX;
        public float? ScaleX
        {
            get => _scaleX;
            set
            {
                if (!_scaleX.IsEqual(value))
                {
                    _scaleX = value;
                    OnPropertyChanged(nameof(ScaleX));
                }
            }
        }

        private float? _scaleY;
        public float? ScaleY
        {
            get => _scaleY;
            set
            {
                if (!_scaleY.IsEqual(value))
                {
                    _scaleY = value;
                    OnPropertyChanged(nameof(ScaleY));
                }
            }
        }

        private float? _scaleZ;
        public float? ScaleZ
        {
            get => _scaleZ;
            set
            {
                if (!_scaleZ.IsEqual(value))
                {
                    _scaleZ = value;
                    OnPropertyChanged(nameof(ScaleZ));
                }
            }
        }

        public MultiTransformer(MultiObject multiObject)
            : base(multiObject)
        {
            Refresh();
        }

        protected override bool UpdateProperties()
        {
            PosX = MultiObject.GetMixedValue(SelectedComponents, new Func<Transformer, float>(x => x.Position.X));
            PosY = MultiObject.GetMixedValue(SelectedComponents, new Func<Transformer, float>(x => x.Position.Y));
            PosZ = MultiObject.GetMixedValue(SelectedComponents, new Func<Transformer, float>(x => x.Position.Z));

            RotX = MultiObject.GetMixedValue(SelectedComponents, new Func<Transformer, float>(x => x.Rotation.X));
            RotY = MultiObject.GetMixedValue(SelectedComponents, new Func<Transformer, float>(x => x.Rotation.Y));
            RotZ = MultiObject.GetMixedValue(SelectedComponents, new Func<Transformer, float>(x => x.Rotation.Z));

            ScaleX = MultiObject.GetMixedValue(SelectedComponents, new Func<Transformer, float>(x => x.Scale.X));
            ScaleY = MultiObject.GetMixedValue(SelectedComponents, new Func<Transformer, float>(x => x.Scale.Y));
            ScaleZ = MultiObject.GetMixedValue(SelectedComponents, new Func<Transformer, float>(x => x.Scale.Z));

            return true;
        }
        protected override bool UpdateSelectedComponents(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(PosX):
                case nameof(PosY):
                case nameof(PosZ):
                    SelectedComponents.ForEach(component => component.Position = new Vector3(
                        _posX ?? component.Position.X,
                        _posY ?? component.Position.Y,
                        _posZ ?? component.Position.Z));
                    return true;

                case nameof(RotX):
                case nameof(RotY):
                case nameof(RotZ):
                    SelectedComponents.ForEach(component => component.Rotation = new Vector3(
                        _rotX ?? component.Rotation.X,
                        _rotY ?? component.Rotation.Y,
                        _rotZ ?? component.Rotation.Z));
                    return true;

                case nameof(ScaleX):
                case nameof(ScaleY):
                case nameof(ScaleZ):
                    SelectedComponents.ForEach(component => component.Scale = new Vector3(
                        _scaleX ?? component.Scale.X,
                        _scaleY ?? component.Scale.Y,
                        _scaleZ ?? component.Scale.Z));
                    return true;
            }
            return false;
        }
    }
}
