using System;
using System.Windows.Threading;

namespace XunlanEditor.Utilities
{
    class DelayEventTimerArgs : EventArgs
    {
        //public bool IsRepeatEvent { get; set; }
        public object Data { get; set; }

        public DelayEventTimerArgs(object data)
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
        private object _data;

        public DelayEventTimer(TimeSpan delayInterval, DispatcherPriority priority = DispatcherPriority.Normal)
        {
            _delayInterval = delayInterval;
            _timer = new DispatcherTimer(priority)
            {
                Interval = TimeSpan.FromMilliseconds(delayInterval.TotalMilliseconds * 0.5),
            };
            _timer.Tick += OnTimerTick;
        }

        public void Enable(object data = null)
        {
            _lastEventTime = DateTime.Now;
            _timer.IsEnabled = true;
            _data = data;
        }
        public void Disable() { _timer.IsEnabled = false; }

        private void OnTimerTick(object sender, EventArgs e)
        {
            if ((DateTime.Now - _lastEventTime) < _delayInterval) return;

            DelayEventTimerArgs eventArgs = new DelayEventTimerArgs(_data);
            Handler?.Invoke(this, eventArgs);
            Disable();

            //if (!eventArgs.IsRepeatEvent) Disable();
        }
    }
}
