using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Reflection.Emit;
using System.Runtime.Serialization;
using System.Text;
using System.Windows;
using System.Windows.Controls;

namespace XunlanEditor
{
    [DataContract(IsReference = true)]
    public class ViewModelBase : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
