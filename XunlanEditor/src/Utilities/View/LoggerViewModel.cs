using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Data;
using System.Windows.Input;

namespace XunlanEditor.Utilities
{
    public enum MsgType
    {
        Info = 1,
        Warning = 2,
        Error = 4,
    }

    public class LogMessage
    {
        public DateTime Time { get; }
        public MsgType MsgType { get; }
        public string Message { get; }
        public string FileName { get; }
        public string Caller { get; }
        public int Line { get; }

        public string MetaData
        {
            get => $"{FileName}: {Caller}({Line})";
        }

        public LogMessage(MsgType type,string message,string filePath,string caller,int line)
        {
            Time = DateTime.Now;
            MsgType = type;
            Message = message;
            FileName = Path.GetFileName(filePath);
            Caller = caller;
            Line = line;
        }
    }

    static class Logger
    {
        private static int _filter = (int)(MsgType.Info | MsgType.Warning | MsgType.Error);

        private static readonly ObservableCollection<LogMessage> _messageList = new ObservableCollection<LogMessage>();
        public static ReadOnlyObservableCollection<LogMessage> MessageList { get; } = new ReadOnlyObservableCollection<LogMessage>(_messageList);
        public static CollectionViewSource FilteredMessageList { get; } = new CollectionViewSource() { Source = MessageList };

        public static ICommand ClearCommand { get; }

        static Logger()
        {
            FilteredMessageList.Filter += (sender,e) =>
            {
                LogMessage message = e.Item as LogMessage;
                e.Accepted = ((int)message.MsgType & _filter) != 0;
            };

            ClearCommand = new RelayCommand<object>(x => Clear());
        }

        public static async void LogMessage(
            MsgType type,
            string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string caller = "",
            [CallerLineNumber] int line = 0)
        {
            await Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                _messageList.Add(new LogMessage(type,message,filePath,caller,line));
            }));
        }

        public static async void Clear()
        {
            await Application.Current.Dispatcher.BeginInvoke(new Action(() => _messageList.Clear()));
        }

        public static async void SetMessageFilter(int mask)
        {
            _filter = mask;
            await Application.Current.Dispatcher.BeginInvoke(new Action(() => FilteredMessageList.View.Refresh()));
        }
    }
}
