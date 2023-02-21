using System;
using System.Collections.Generic;
using System.Text;

namespace XunlanEditor.Utilities
{
    public static class MathUtil
    {
        public static float Epsilon => 0.00001f;

        public static bool IsEqual(this float a, float b)
        {
            return Math.Abs(a - b) < Epsilon;
        }
        public static bool IsEqual(this float? a, float? b)
        {
            if (a == null || b == null) return false;
            return Math.Abs(a.Value - b.Value) < Epsilon;
        }
    }
}
