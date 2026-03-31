using System.IO.Ports;

namespace ArduinoHID.Services;

public class SerialService : IDisposable
{
    private SerialPort? _port;
    private readonly object _lock = new();

    public event Action<string>? DataReceived;
    public event Action<bool>? ConnectionChanged;

    public bool IsConnected => _port?.IsOpen ?? false;
    public string? CurrentPort { get; private set; }

    public string[] GetAvailablePorts()
    {
        return SerialPort.GetPortNames();
    }

    public bool Connect(string portName, int baudRate = 115200)
    {
        Disconnect();

        try
        {
            _port = new SerialPort(portName, baudRate)
            {
                DataBits = 8,
                Parity = Parity.None,
                StopBits = StopBits.One,
                ReadTimeout = 3000,
                WriteTimeout = 1000,
                NewLine = "\n"
            };

            _port.DataReceived += (s, e) =>
            {
                try
                {
                    var line = _port.ReadLine();
                    DataReceived?.Invoke(line.Trim());
                }
                catch { }
            };

            _port.Open();
            CurrentPort = portName;
            ConnectionChanged?.Invoke(true);
            return true;
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Serial connect error: {ex.Message}");
            return false;
        }
    }

    public void Disconnect()
    {
        lock (_lock)
        {
            if (_port != null)
            {
                try
                {
                    if (_port.IsOpen)
                        _port.Close();
                    _port.Dispose();
                }
                catch { }
                _port = null;
                CurrentPort = null;
                ConnectionChanged?.Invoke(false);
            }
        }
    }

    public async Task<string> SendCommandAsync(string cmd, CancellationToken ct = default, double timeoutSeconds = 0.5)
    {
        if (_port == null || !_port.IsOpen)
            return "ERR:Not connected";

        try
        {
            _port.WriteLine(cmd);
            return await ReadLineAsync(ct, timeoutSeconds);
        }
        catch (Exception ex)
        {
            return $"ERR:{ex.Message}";
        }
    }

    public string SendCommand(string cmd)
    {
        if (_port == null || !_port.IsOpen)
            return "ERR:Not connected";

        try
        {
            _port.WriteLine(cmd);
            return _port.ReadLine()?.Trim() ?? "";
        }
        catch (Exception ex)
        {
            return $"ERR:{ex.Message}";
        }
    }

    private async Task<string> ReadLineAsync(CancellationToken ct, double timeoutSeconds = 0.5)
    {
        var buffer = new List<char>();
        var start = DateTime.Now;

        while (true)
        {
            // Check timeout (only if timeoutSeconds > 0)
            if (timeoutSeconds > 0 && (DateTime.Now - start).TotalSeconds >= timeoutSeconds)
                break;

            ct.ThrowIfCancellationRequested();

            lock (_lock)
            {
                if (_port?.BytesToRead > 0)
                {
                    int b = _port.ReadByte();
                    if (b == '\n' || b == '\r')
                    {
                        if (buffer.Count > 0)
                            break;
                    }
                    else
                    {
                        buffer.Add((char)b);
                    }
                }
            }

            await Task.Delay(10, ct);
        }

        return new string(buffer.ToArray()).Trim();
    }

    public void Dispose()
    {
        Disconnect();
    }
}
