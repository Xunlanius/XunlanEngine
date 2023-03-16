using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.Serialization;

namespace XunlanEditor.Utilities
{
    public static class Serializer
    {
        public static void ObjectToFile<T>(T instance,string path) where T : class
        {
            try
            {
                DataContractSerializer serializer = new DataContractSerializer(typeof(T));
                using FileStream fileStream = new FileStream(path,FileMode.Create);
                {
                    serializer.WriteObject(fileStream,instance);
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Debugger.Break();
                throw;
            }
        }

        public static T FileToObject<T>(string path) where T : class
        {
            try
            {
                DataContractSerializer serializer = new DataContractSerializer(typeof(T));
                using FileStream fileStream = new FileStream(path,FileMode.Open);
                {
                    return serializer.ReadObject(fileStream) as T;
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Debugger.Break();
                throw;
            }
        }
    }
}
