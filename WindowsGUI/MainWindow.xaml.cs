using System.Windows;
using ArduinoHID.ViewModels;

namespace ArduinoHID;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        DataContext = new MainViewModel();
        Closing += (s, e) => (DataContext as MainViewModel)?.Dispose();
    }
}
