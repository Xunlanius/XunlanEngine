using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;

namespace XunlanEditor.Dictionaries
{
    public partial class ControlTemplatesDictionary : ResourceDictionary
    {
        private void OnTextBox_KeyDown(object sender,KeyEventArgs e)
        {
            TextBox textBox = sender as TextBox;
            BindingExpression bindingExpression = textBox.GetBindingExpression(TextBox.TextProperty);

            if(bindingExpression == null)
                return;

            if(e.Key == Key.Enter)
            {
                if(textBox.Tag is ICommand cmd && cmd.CanExecute(textBox.Text))
                {
                    cmd.Execute(textBox.Text);
                }
                else
                {
                    bindingExpression.UpdateSource();
                }

                Keyboard.ClearFocus();
                e.Handled = true;
            }
            else if(e.Key == Key.Escape)
            {
                // discard changes
                bindingExpression.UpdateTarget();

                Keyboard.ClearFocus();
                e.Handled = true;
            }
        }

        private void OnTextBoxRename_KeyDown(object sender,KeyEventArgs e)
        {
            TextBox textBox = sender as TextBox;
            BindingExpression bindingExpression = textBox.GetBindingExpression(TextBox.TextProperty);

            if(bindingExpression == null)
                return;

            if(e.Key == Key.Enter)
            {
                if(textBox.Tag is ICommand cmd && cmd.CanExecute(textBox.Text))
                {
                    cmd.Execute(textBox.Text);
                }
                else
                {
                    bindingExpression.UpdateSource();
                }

                textBox.Visibility = Visibility.Collapsed;
                e.Handled = true;
            }
            else if(e.Key == Key.Escape)
            {
                // discard changes
                bindingExpression.UpdateTarget();

                textBox.Visibility = Visibility.Collapsed;
                e.Handled = true;
            }
        }

        private void OnTextBoxRename_LostFocus(object sender,RoutedEventArgs e)
        {
            TextBox textBox = sender as TextBox;
            if(!textBox.IsVisible)
                return;
            BindingExpression bindingExpression = textBox.GetBindingExpression(TextBox.TextProperty);

            if(bindingExpression != null)
            {
                bindingExpression.UpdateTarget();
                textBox.Visibility = Visibility.Collapsed;
            }
        }

        private void OnClose_Button_Click(object sender,RoutedEventArgs e)
        {
            Window window = (Window)((FrameworkElement)sender).TemplatedParent;
            window.Close();
        }

        private void OnMaximized_Button_Click(object sender,RoutedEventArgs e)
        {
            Window window = (Window)((FrameworkElement)sender).TemplatedParent;
            if(window.WindowState == WindowState.Maximized)
            {
                window.WindowState = WindowState.Normal;
            }
            else if(window.WindowState == WindowState.Normal)
            {
                window.WindowState = WindowState.Maximized;
            }
        }

        private void OnMinimized_Button_Click(object sender,RoutedEventArgs e)
        {
            Window window = (Window)((FrameworkElement)sender).TemplatedParent;
            window.WindowState = WindowState.Minimized;
        }
    }
}
