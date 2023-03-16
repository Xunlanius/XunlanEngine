using System;
using System.Collections.Generic;
using System.Windows.Threading;

namespace XunlanEditor.Utilities
{
    class DelayEventTimerArgs : EventArgs
    {
        public IEnumerable<object> Data
        {
            get; set;
        }

        public DelayEventTimerArgs(IEnumerable<object> data)
        {
            Data = data;
        }
    }

    class DelayEventTimer
    {
        private readonly DispatcherTimer _timer;
        private readonly TimeSpan _delayInterval;
        private DateTime _lastEventTime = DateTime.Now;

        public event EventHandler<DelayEventTimerArgs> Handler;
        private readonly List<object> _eventArgsSet = new List<object>();

        public DelayEventTimer(TimeSpan delayInterval,DispatcherPriority priority = DispatcherPriority.Normal)
        {
            _delayInterval = delayInterval;
            _timer = new DispatcherTimer(priority)
            {
                Interval = TimeSpan.FromMilliseconds(delayInterval.TotalMilliseconds * 0.5),
            };
            _timer.Tick += OnTimerTick;
        }

        public void Trigger(object eventArgs = null)
        {
            _lastEventTime = DateTime.Now;
            _timer.IsEnabled = true;

            if(eventArgs != null)
                _eventArgsSet.Add(eventArgs);
        }
        public void Disable()
        {
            _timer.IsEnabled = false;
        }

        private void OnTimerTick(object sender,EventArgs e)
        {
            if((DateTime.Now - _lastEventTime) < _delayInterval)
                return;

            DelayEventTimerArgs eventArgs = new DelayEventTimerArgs(_eventArgsSet);
            Handler?.Invoke(this,eventArgs);
            _eventArgsSet.Clear();
            Disable();
        }
    }
}
