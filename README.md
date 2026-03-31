# Arduino HID Controller

Use Arduino Leonardo (or any USB-native Arduino) as a virtual keyboard/mouse over USB — controlled from your Mac via serial commands.

**Use case:** Control any PC (including headless/target machine) by plugging the Leonardo into the *target PC* and controlling it from your Mac over serial.

---

## Hardware

- **Arduino Leonardo** (or Micro/Pro Micro) — native USB, no separate USB-serial chip
- Connected to your **Mac** now (for flashing + testing)
- Will be moved to **target PC** for actual HID control

---

## Flash the Sketch (Arduino IDE)

### 1. Open the sketch
Open `arduino_hid_controller/arduino_hid_controller.ino` in Arduino IDE.

### 2. Select board
Tools → Board → **Arduino Leonardo** (or "Arduino Leonardo (Virtual Com Port)" on Windows)

### 3. Select port
Tools → Port → The Leonardo port (may appear as `/dev/cu.usbmodemHIDFG1` or similar after first flash)

### 4. Upload
Click **Upload** (→ icon). The sketch will compile and flash.

### 5. Verify it's running
After upload completes, open **Tools → Serial Monitor** (baud: 115200). You should see:
```
Arduino HID Controller Ready
Type HELP for commands, PING to test
```
Type `PING` — you should get `PONG`.

---

## Install Python Dependencies (Mac)

```bash
pip3 install pyserial --break-system-packages
```

---

## Usage

### Auto-detect and send single command
```bash
python3 arduino-hid.py KEY:Hello
python3 arduino-hid.py KEY:ENTER
python3 arduino-hid.py MOUSE:50,0
python3 arduino-hid.py CLICK:left
python3 arduino-hid.py "KEY:SHIFT:hello"   # uppercase
python3 arduino-hid.py "KEY:CTRL:c"       # Ctrl+C
```

### Specify port manually
```bash
python3 arduino-hid.py --port /dev/cu.usbmodem31201 KEY:Hello
```

### Interactive mode
```bash
python3 arduino-hid.py --interactive
> KEY:Hello
> MOUSE:100,50
> CLICK:right
> PING
> exit
```

### Batch mode (commands from stdin)
```bash
cat commands.txt | python3 arduino-hid.py --batch
```

### List available devices
```bash
python3 arduino-hid.py --list
```

---

## Command Reference

### Keyboard
| Command | Action |
|---|---|
| `KEY:Hello` | Type text |
| `KEY:ENTER` | Press Enter |
| `KEY:SPACE` | Press Space |
| `KEY:TAB` | Press Tab |
| `KEY:BACKSPACE` | Backspace |
| `KEY:ESCAPE` | Escape |
| `KEY:UP/DOWN/LEFT/RIGHT` | Arrow keys |
| `KEY:SHIFT:Hello` | Type uppercase |
| `KEY:CTRL:c` | Ctrl+key |
| `KEY:ALT:f` | Alt+key |
| `KEY:META:d` | Cmd+key (Mac) |
| `KEYCODE:36` | Raw keycode |

### Mouse
| Command | Action |
|---|---|
| `MOUSE:10,20` | Move relative |
| `MOUSE:ABS:16000,9000` | Move absolute |
| `CLICK:left` | Left click |
| `CLICK:right` | Right click |
| `CLICK:middle` | Middle click |
| `PRESS:left` | Hold left button |
| `RELEASE:left` | Release left button |
| `SCROLL:0,-3` | Scroll up |

### System
| Command | Action |
|---|---|
| `PING` | Connection test |
| `HELP` | Show all commands |
| `RELEASEALL` | Release all keys/buttons |
| `SCREEN:WIDTH` | Get screen dimensions |
| `LIST` | List serial devices |

---

## Example Workflows

### Open Notepad and type on target PC
```bash
python3 arduino-hid.py KEY:WIN+r
python3 arduino-hid.py KEY:ENTER         # Wait for Run dialog
python3 arduino-hid.py KEY:BACKSPACE    # Clear
python3 arduino-hid.py KEY:notepad
python3 arduino-hid.py KEY:ENTER         # Opens Notepad
python3 arduino-hid.py KEY:Hello
```

### Drag a file (hold + move)
```bash
python3 arduino-hid.py MOUSE:200,200      # Move to icon
python3 arduino-hid.py CLICK:left        # Click to select
python3 arduino-hid.py MOUSE:500,300      # Move with held click
```

### Open Chrome on target PC
```bash
python3 arduino-hid.py KEY:WIN:r
python3 arduino-hid.py KEY:ENTER         # Run dialog
python3 arduino-hid.py KEY:BACKSPACE
python3 arduino-hid.py "KEY:SHIFT:chrome"
python3 arduino-hid.py KEY:ENTER
```

---

## Notes

- Leonardo shows up as `/dev/cu.usbmodem31201` on your Mac
- The generic USB-serial adapter is `/dev/cu.usbserial-3130` (not for HID)
- The sketch auto-releases all keys after each command to avoid stuck keys
- 115200 baud, 8N1, no flow control
- On target PC, no drivers needed (Leonardo is HID class)

## Files

```
arduino-hid/
├── arduino_hid_controller/
│   └── arduino_hid_controller.ino   # Arduino sketch (flash this)
├── arduino-hid.py                    # Mac-side controller CLI
├── WindowsGUI/                        # Windows WPF GUI
│   ├── ArduinoHID.csproj
│   ├── MainWindow.xaml(.cs)
│   ├── ViewModels/MainViewModel.cs
│   ├── Services/SerialService.cs
│   └── Converters/BoolConverters.cs
└── README.md
```

---

## Windows GUI (WPF)

A Windows-only graphical interface for controlling the Arduino HID device.

### Requirements
- Windows 10/11
- .NET 8.0 Runtime
- Arduino Leonardo (or compatible) connected to target PC

### Build from Source

```powershell
cd arduino-hid/WindowsGUI
dotnet restore
dotnet add package CommunityToolkit.Mvvm
dotnet add package System.IO.Ports
dotnet build
```

### Run
```powershell
dotnet run
# or
.\bin\Debug\net8.0-windows\ArduinoHID.exe
```

### Publish (Self-contained)

```powershell
dotnet publish -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true
```

### Features
- Serial port connection management (auto-detect COM ports)
- Keyboard shortcut buttons (Win, R, Tab, Esc, Enter, arrows, F1-F4, etc.)
- Modifier keys (Shift, Ctrl, Alt, Meta)
- Text input with modifier support
- Mouse click buttons (left, right, middle)
- Scroll wheel control
- Custom command input
- Real-time command/response log

### UI Layout
```
┌─────────────────────────────────────────────────────────┐
│  Arduino HID Controller                    [COM3 ▼] [↻] [Connect] │
├─────────────────────────────────────────────────────────┤
│  Keyboard                              Mouse            │
│  [WIN][R][TAB][ESC][ENTER][SPACE][BS]  [LEFT][RIGHT][MID] │
│  [↑][↓][←][→][HOME][END][PGUP][PGDN]   [↑SCROLL][↓SCROLL] │
│  [F1][F2][F3][F4][CAPS][DEL]                            │
│  ☐ Shift ☐ Ctrl ☐ Alt ☐ Meta  [RELEASE ALL]            │
├─────────────────────────────────────────────────────────┤
│  Text Input                    │  Quick Commands        │
│  [________________] [Send]     │  Custom: [_______]    │
├─────────────────────────────────────────────────────────┤
│  Command Log                                           │
│  > PING                                                │
│  ← PONG                                                │
│  > KEY:Hello                                          │
│  ← OK:KEY:Hello                                       │
├─────────────────────────────────────────────────────────┤
│  Port: COM3 | Baud: 115200                            │
└─────────────────────────────────────────────────────────┘
```

### Integration with OCR/Vision Automation

The GUI sends commands over serial to the Arduino. To integrate with screen capture and OCR:

1. Arduino plugs into **target PC** (as HID keyboard/mouse)
2. Serial cable connects **target PC** to **host PC** running this GUI
3. USB capture card sends target PC screen to host
4. Host runs OCR → finds UI elements → calculates coordinates
5. Host sends commands via this GUI to control target PC

See the BIOS/Windows automation workflow in the Discord discussion about VT3 stress testing.
