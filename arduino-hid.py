#!/usr/bin/env python3
"""
Arduino HID Controller - Host Side (Mac)
Sends commands to Arduino Leonardo over USB serial to emulate keyboard/mouse HID.

Usage:
    python3 arduino-hid.py KEY:Hello
    python3 arduino-hid.py MOUSE:100,0
    python3 arduino-hid.py CLICK:left
    python3 arduino-hid.py --interactive
"""

import argparse
import sys
import time
import serial
import serial.tools.list_ports

# Default serial settings
DEFAULT_BAUD = 115200
DEFAULT_PORT_LEONARDO = "/dev/cu.usbmodemHIDFG1"  # Current Leonardo port
DEFAULT_PORT_SERIAL = "/dev/cu.usbserial-3130"

def find_arduino_port():
    """Find the first available Arduino Leonardo or USB serial device."""
    ports = serial.tools.list_ports.comports()
    
    # Priority: Leonardo (usbmodem) first, then generic usbserial
    leonardo = None
    generic = None
    
    for port in ports:
        desc = port.description.lower()
        if 'leonardo' in desc or 'usbmodem' in port.device:
            leonardo = port.device
        elif 'usbserial' in port.device or 'usb-serial' in desc:
            generic = port.device
    
    return leonardo or generic

def connect(port=None, baud=DEFAULT_BAUD, timeout=3.0):
    """Connect to Arduino serial port."""
    if port is None:
        port = find_arduino_port()
        if port is None:
            print("Error: No Arduino HID device found. Is it connected?", file=sys.stderr)
            sys.exit(1)
    
    try:
        ser = serial.Serial(port, baud, timeout=timeout)
        time.sleep(0.1)  # Let connection settle
        
        # Drain any stale data
        ser.reset_input_buffer()
        
        # Send initial ping to verify connection
        ser.write(b'PING\n')
        ser.flush()
        time.sleep(0.1)
        
        response = ser.readline().decode('utf-8', errors='ignore').strip()
        if response == 'PONG':
            print(f"Connected to {port} (baud={baud})", file=sys.stderr)
        else:
            print(f"Connected to {port} (unexpected greeting: {response})", file=sys.stderr)
        
        return ser
    except serial.SerialException as e:
        print(f"Error connecting to {port}: {e}", file=sys.stderr)
        sys.exit(1)

def send_command(ser, cmd, wait_response=True, timeout=5.0):
    """Send a command and optionally wait for response."""
    ser.write(f"{cmd}\n".encode('utf-8'))
    ser.flush()
    
    if wait_response:
        start = time.time()
        while time.time() - start < timeout:
            if ser.in_waiting > 0:
                response = ser.readline().decode('utf-8', errors='ignore').strip()
                return response
            time.sleep(0.01)
        return ""  # Timeout
    return None

def send_batch(ser, commands):
    """Send multiple commands in sequence."""
    for cmd in commands:
        resp = send_command(ser, cmd)
        print(f"  {cmd} → {resp}" if resp else f"  {cmd} → OK")

def list_devices():
    """List all connected USB serial devices."""
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("No USB serial devices found.")
        return
    
    print("Available USB serial devices:")
    for p in ports:
        print(f"  {p.device}: {p.description}")

def interactive_mode(port=None):
    """Interactive command mode."""
    ser = connect(port)
    print("\nArduino HID Controller - Interactive Mode")
    print("Type commands (e.g., KEY:Hello, MOUSE:10,20, CLICK:left)")
    print("Type 'help' for command list, 'exit' to quit.\n")
    
    while True:
        try:
            cmd = input("> ").strip()
            if not cmd:
                continue
            if cmd.lower() in ('exit', 'quit', 'q'):
                break
            if cmd.lower() == 'help':
                print_help()
                continue
            
            resp = send_command(ser, cmd)
            if resp:
                print(f"  → {resp}")
            else:
                print("  → (timeout)")
        except (EOFError, KeyboardInterrupt):
            print("\nExiting.")
            break
    
    ser.close()

def print_help():
    """Print command reference."""
    print("""
=== Command Reference ===

Keyboard:
  KEY:<text>           Type text (e.g., KEY:Hello World)
  KEY:ENTER           Press Enter
  KEY:SPACE           Press Space
  KEY:TAB             Press Tab
  KEY:BACKSPACE       Press Backspace
  KEY:DELETE          Press Delete
  KEY:ESCAPE          Press Escape
  KEY:UP/DOWN/LEFT/RIGHT  Arrow keys
  KEY:HOME/END        Home/End keys
  KEY:PAGEUP/PAGEDOWN Page Up/Down
  KEY:CAPSLOCK        Caps Lock
  KEY:F1-F4           Function keys
  KEY:SHIFT:<text>    Type with Shift held
  KEY:CTRL:<text>     Type with Ctrl held
  KEY:ALT:<text>      Type with Alt held
  KEY:META:<text>     Type with Cmd held (Mac)
  KEYCODE:<n>         Press raw keycode

Mouse:
  MOUSE:<dx>,<dy>     Move mouse relative (e.g., MOUSE:10,0)
  MOUSE:ABS:<x>,<y>   Move mouse absolute (0-32767)
  CLICK:left          Left click
  CLICK:right         Right click
  CLICK:middle        Middle click
  PRESS:left          Hold left button
  RELEASE:left        Release left button
  SCROLL:<dx>,<dy>    Scroll wheel

System:
  PING                Test connection
  HELP                Show this help
  SCREEN:WIDTH        Get screen dimensions
  RELEASEALL          Release all keys/mouse
  LIST                List available serial devices

Examples:
  python3 arduino-hid.py KEY:Hello
  python3 arduino-hid.py "KEY:SHIFT:hello"
  python3 arduino-hid.py MOUSE:100,50
  python3 arduino-hid.py CLICK:right
  python3 arduino-hid.py --interactive
""")

def main():
    parser = argparse.ArgumentParser(description='Arduino HID Controller')
    parser.add_argument('command', nargs='?', help='Command to send (e.g., KEY:Hello)')
    parser.add_argument('--port', '-p', help=f'Serial port (auto-detect if omitted)')
    parser.add_argument('--baud', '-b', type=int, default=DEFAULT_BAUD, help=f'Baud rate (default: {DEFAULT_BAUD})')
    parser.add_argument('--no-wait', '-n', action='store_true', help="Don't wait for response")
    parser.add_argument('--list', '-l', action='store_true', help='List available serial devices')
    parser.add_argument('--interactive', '-i', action='store_true', help='Interactive mode')
    parser.add_argument('--batch', action='store_true', help='Batch mode (one arg per command)')
    
    args = parser.parse_args()
    
    # List devices mode
    if args.list:
        list_devices()
        return
    
    # Interactive mode
    if args.interactive:
        interactive_mode(args.port)
        return
    
    # Batch or single command mode
    if args.batch:
        # Read commands from stdin (one per line)
        ser = connect(args.port, args.baud)
        for line in sys.stdin:
            cmd = line.strip()
            if cmd and not cmd.startswith('#'):
                resp = send_command(ser, cmd)
                print(resp if resp else "OK")
        ser.close()
        return
    
    # Single command mode
    if not args.command:
        parser.print_help()
        print("\nExamples:")
        print("  python3 arduino-hid.py KEY:Hello")
        print("  python3 arduino-hid.py MOUSE:10,20")
        print("  python3 arduino-hid.py --interactive")
        sys.exit(1)
    
    ser = connect(args.port, args.baud)
    resp = send_command(ser, args.command, wait_response=not args.no_wait)
    if resp:
        print(resp)
    ser.close()

if __name__ == "__main__":
    main()
