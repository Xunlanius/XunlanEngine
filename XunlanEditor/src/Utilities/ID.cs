using System;
using System.Collections.Generic;
using System.Text;

namespace XunlanEditor.Utilities
{
    public static class ID
    {
        public const UInt32 InvalidID = UInt32.MaxValue;

        public static bool IsValid(UInt32 id) => id != InvalidID;
    }
}