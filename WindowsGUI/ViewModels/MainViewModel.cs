using System.Collections.ObjectModel;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using ArduinoHID.Services;

namespace ArduinoHID.ViewModels;

public partial class MainViewModel : ObservableObject, IDisposable
{
    private readonly SerialService _serial;
    private CancellationTokenSource _cts = new();

    [ObservableProperty]
    private string _commandLog = "";

    [ObservableProperty]
    private string _textInput = "";

    [ObservableProperty]
    private string _customCommand = "";

    [ObservableProperty]
    private string _selectedPort = "";

    [ObservableProperty]
    private bool _isConnected;

    [ObservableProperty]
    private bool _shiftModifier;

    [ObservableProperty]
    private bool _ctrlModifier;

    [ObservableProperty]
    private bool _altModifier;

    [ObservableProperty]
    private bool _metaModifier;

    [ObservableProperty]
    private int _mouseX;

    [ObservableProperty]
    private int _mouseY;

    [ObservableProperty]
    private bool _noTimeout;

    public ObservableCollection<string> AvailablePorts { get; } = new();

    public MainViewModel()
    {
        _serial = new SerialService();
        _serial.DataReceived += OnDataReceived;
        _serial.ConnectionChanged += connected => IsConnected = connected;
        RefreshPorts();
    }

    partial void OnIsConnectedChanged(bool value)
    {
        if (value)
        {
            // Verify connection with PING
            _ = SendCommandAsync("PING");
        }
    }

    private void OnDataReceived(string data)
    {
        System.Windows.Application.Current?.Dispatcher.Invoke(() =>
        {
            AppendLog($"← {data}");
        });
    }

    [RelayCommand]
    private void RefreshPorts()
    {
        AvailablePorts.Clear();
        foreach (var port in _serial.GetAvailablePorts())
        {
            AvailablePorts.Add(port);
        }

        if (SelectedPort == "" && AvailablePorts.Count > 0)
            SelectedPort = AvailablePorts[0];
        else if (!AvailablePorts.Contains(SelectedPort))
            SelectedPort = AvailablePorts.FirstOrDefault() ?? "";
    }

    [RelayCommand]
    private async Task ConnectAsync()
    {
        if (string.IsNullOrEmpty(SelectedPort))
        {
            AppendLog("ERR: No port selected");
            return;
        }

        AppendLog($"Connecting to {SelectedPort}...");
        if (_serial.Connect(SelectedPort))
        {
            IsConnected = true;
            AppendLog($"Connected to {SelectedPort}");
        }
        else
        {
            AppendLog($"ERR: Failed to connect to {SelectedPort}");
        }
    }

    [RelayCommand]
    private void Disconnect()
    {
        _serial.Disconnect();
        AppendLog("Disconnected");
    }

    [RelayCommand]
    private async Task SendKeyAsync(string key)
    {
        await SendCommandAsync($"KEY:{key}");
    }

    [RelayCommand]
    private async Task SendTextAsync()
    {
        if (string.IsNullOrWhiteSpace(TextInput))
            return;

        var cmd = BuildModifierCommand($"KEY:{TextInput}");
        await SendCommandAsync(cmd);
        TextInput = "";
    }

    [RelayCommand]
    private async Task SendCustomCommandAsync()
    {
        if (string.IsNullOrWhiteSpace(CustomCommand))
            return;

        await SendCommandAsync(CustomCommand);
        CustomCommand = "";
    }

    [RelayCommand]
    private async Task MouseMoveRelativeAsync()
    {
        await SendCommandAsync($"MOUSE:{MouseX},{MouseY}");
    }

    [RelayCommand]
    private async Task MouseMoveAbsoluteAsync()
    {
        // HID absolute range is 0-32767, need to scale
        int absX = (int)(MouseX / 1920.0 * 32767);
        int absY = (int)(MouseY / 1080.0 * 32767);
        await SendCommandAsync($"MOUSE:ABS:{absX},{absY}");
    }

    [RelayCommand]
    private async Task ClickAsync(string button)
    {
        await SendCommandAsync($"CLICK:{button}");
    }

    [RelayCommand]
    private async Task PressAsync(string button)
    {
        await SendCommandAsync($"PRESS:{button}");
    }

    [RelayCommand]
    private async Task ReleaseAsync(string button)
    {
        await SendCommandAsync($"RELEASE:{button}");
    }

    [RelayCommand]
    private async Task ScrollAsync(string direction)
    {
        int dy = direction.ToLower() == "up" ? -3 : 3;
        await SendCommandAsync($"SCROLL:0,{dy}");
    }

    [RelayCommand]
    private async Task ReleaseAllAsync()
    {
        await SendCommandAsync("RELEASEALL");
    }

    [RelayCommand]
    private void ClearLog()
    {
        CommandLog = "";
    }

    private async Task SendCommandAsync(string cmd)
    {
        if (!_serial.IsConnected)
        {
            AppendLog("ERR: Not connected");
            return;
        }

        AppendLog($"> {cmd}");
        var timeout = NoTimeout ? 0 : 0.5;
        var response = await _serial.SendCommandAsync(cmd, _cts.Token, timeout);
        if (!string.IsNullOrEmpty(response))
        {
            AppendLog($"← {response}");
        }
    }

    private string BuildModifierCommand(string baseCmd)
    {
        var mods = new List<string>();
        if (ShiftModifier) mods.Add("SHIFT");
        if (CtrlModifier) mods.Add("CTRL");
        if (AltModifier) mods.Add("ALT");
        if (MetaModifier) mods.Add("META");

        if (mods.Count == 0)
            return baseCmd;

        // Extract the KEY: part and prepend modifiers
        if (baseCmd.StartsWith("KEY:"))
        {
            return $"KEY:{string.Join(":", mods)}:{baseCmd[4..]}";
        }
        return baseCmd;
    }

    private void AppendLog(string line)
    {
        CommandLog += line + Environment.NewLine;
    }

    public void Dispose()
    {
        _cts.Cancel();
        _cts.Dispose();
        _serial.Dispose();
    }
}
