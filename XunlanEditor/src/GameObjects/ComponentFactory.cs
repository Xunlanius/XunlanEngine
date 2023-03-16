using System;
using System.Diagnostics;

namespace XunlanEditor.GameObjects
{
    enum ComponentType
    {
        Transformer,
        Script,
    }

    static class ComponentFactory
    {
        private static readonly Func<GameObject,object,Component>[] _functions = new Func<GameObject,object,Component>[]
        {
            (gameObject, data) => new Transformer(gameObject),
            (gameObject, data) => new Script(gameObject) { ScriptName = (string)data },
        };

        public static Func<GameObject,object,Component> GetCreationFunc(ComponentType componentType)
        {
            Debug.Assert((int)componentType < _functions.Length);
            return _functions[(int)componentType];
        }

        public static ComponentType ToEnumType(Component component)
        {
            return component switch
            {
                Transformer _ => ComponentType.Transformer,
                Script _ => ComponentType.Script,
                _ => throw new ArgumentException("Unknown component type."),
            };
        }
    }
}
