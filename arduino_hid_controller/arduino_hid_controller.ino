/*
 * Arduino HID Controller - Leonardo Edition
 * Receives commands over Hardware Serial (UART RX/TX) and executes HID actions
 * 
 * HARDWARE: Connect USB-to-Serial adapter to Leonardo RX(0)/TX(1) pins
 * - USB-Serial RX  → Leonardo TX (pin 1)
 * - USB-Serial TX  → Leonardo RX (pin 0)
 * - GND           → GND
 * 
 * Command Protocol:
 *   KEY:<text>           - Type text (letters, numbers, symbols)
 *   KEYCODE:<code>       - Press keycode (e.g., KEYCODE:36 for Enter)
 *   KEY:ENTER            - Press Enter
 *   KEY:SPACE            - Press Space
 *   KEY:BACKSPACE        - Press Backspace
 *   KEY:TAB              - Press Tab
 *   KEY:ESCAPE           - Press Escape
 *   KEY:UP               - Arrow keys
 *   KEY:DOWN
 *   KEY:LEFT
 *   KEY:RIGHT
 *   KEY:SHIFT:<text>     - Type with Shift held (for uppercase/symbols)
 *   KEY:CTRL:<text>      - Type with Ctrl held (e.g., CTRL:C for copy)
 *   KEY:ALT:<text>       - Type with Alt held
 *   KEY:META:<text>      - Type with Cmd held (Mac)
 *   MODIFIER:<modifiers> - Set modifier keys (SHIFT|CTRL|ALT|META)
 *   RELEASEALL           - Release all pressed keys
 *   MOUSE:<dx>,<dy>     - Move mouse relative
 *   MOUSE:ABS:<x>,<y>   - Move mouse absolute (0-32767 range)
 *   CLICK:left           - Left click
 *   CLICK:right          - Right click
 *   CLICK:middle         - Middle click
 *   PRESS:left           - Press and hold left button
 *   PRESS:right          - Press and hold right button
 *   RELEASE:left         - Release left button
 *   RELEASE:right        - Release right button
 *   SCROLL:<dx>,<dy>    - Scroll wheel
 *   SCREEN:WIDTH         - Report screen dimensions
 *   PING                 - Respond with PONG
 *   HELP                 - List available commands
 */

#include <Keyboard.h>
#include <Mouse.h>

// Screen dimensions for absolute positioning
#define SCREEN_W 1920
#define SCREEN_H 1080

// Command buffer
#define BUF_SIZE 128
char buffer[BUF_SIZE];
int bufPos = 0;

// Modifier keys
bool shiftHeld = false;
bool ctrlHeld = false;
bool altHeld = false;
bool metaHeld = false;  // Command key on Mac

void setup() {
  // Use Hardware Serial (UART) - pins 0 (RX) and 1 (TX)
  Serial1.begin(115200);
  Keyboard.begin();
  Mouse.begin();
  
  // Wait for serial connection
  while (!Serial1) {
    ; // Wait for serial1 port to connect
  }
  
  // Indicate ready
  Serial1.println("Arduino HID Controller Ready");
  Serial1.println("Type HELP for commands, PING to test");
}

void loop() {
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    
    if (c == '\n' || c == '\r') {
      if (bufPos > 0) {
        buffer[bufPos] = '\0';
        processCommand(buffer);
        bufPos = 0;
      }
    } else {
      if (bufPos < BUF_SIZE - 1) {
        buffer[bufPos++] = c;
      }
    }
  }
}

void processCommand(const char* cmd) {
  // Skip empty commands
  if (cmd[0] == '\0') return;
  
  // Parse command type
  if (strncmp(cmd, "PING", 4) == 0) {
    Serial1.println("PONG");
  }
  else if (strncmp(cmd, "HELP", 4) == 0) {
    printHelp();
  }
  else if (strncmp(cmd, "KEY:", 4) == 0) {
    handleKeyCommand(cmd + 4);
  }
  else if (strncmp(cmd, "KEYCODE:", 8) == 0) {
    handleKeycodeCommand(cmd + 8);
  }
  else if (strncmp(cmd, "MODIFIER:", 9) == 0) {
    handleModifierCommand(cmd + 9);
  }
  else if (strncmp(cmd, "RELEASEALL", 10) == 0) {
    Keyboard.releaseAll();
    Mouse.release(MOUSE_LEFT);
    Mouse.release(MOUSE_RIGHT);
    Mouse.release(MOUSE_MIDDLE);
    Serial1.println("OK:RELEASEALL");
  }
  else if (strncmp(cmd, "MOUSE:", 6) == 0) {
    handleMouseCommand(cmd + 6);
  }
  else if (strncmp(cmd, "CLICK:", 6) == 0) {
    handleClickCommand(cmd + 6);
  }
  else if (strncmp(cmd, "PRESS:", 6) == 0) {
    handlePressCommand(cmd + 6);
  }
  else if (strncmp(cmd, "RELEASE:", 8) == 0) {
    handleReleaseCommand(cmd + 8);
  }
  else if (strncmp(cmd, "SCROLL:", 7) == 0) {
    handleScrollCommand(cmd + 7);
  }
  else if (strncmp(cmd, "SCREEN:WIDTH", 12) == 0) {
    Serial1.print("SCREEN:");
    Serial1.print(SCREEN_W);
    Serial1.print(":");
    Serial1.println(SCREEN_H);
  }
  else {
    Serial1.print("ERR:Unknown command: ");
    Serial1.println(cmd);
  }
  
  // Ensure all keys are released after each command
  Keyboard.releaseAll();
}

void printHelp() {
  Serial1.println("=== Arduino HID Controller Commands ===");
  Serial1.println("KEY:<text>           - Type text");
  Serial1.println("KEY:ENTER/SPACE/TAB  - Special keys");
  Serial1.println("KEY:UP/DOWN/LEFT/RIGHT- Arrow keys");
  Serial1.println("KEY:BACKSPACE        - Backspace");
  Serial1.println("KEY:ESCAPE           - Escape");
  Serial1.println("KEY:SHIFT:<text>     - Type with Shift");
  Serial1.println("KEY:CTRL:<text>      - Type with Ctrl");
  Serial1.println("KEY:ALT:<text>       - Type with Alt");
  Serial1.println("KEY:META:<text>      - Type with Cmd");
  Serial1.println("KEYCODE:<n>          - Press keycode n");
  Serial1.println("MODIFIER:SHIFT|CTRL|ALT|META - Set/Clear modifiers");
  Serial1.println("RELEASEALL          - Release all keys/buttons");
  Serial1.println("MOUSE:<dx>,<dy>      - Relative mouse move");
  Serial1.println("MOUSE:ABS:<x>,<y>   - Absolute mouse (0-32767)");
  Serial1.println("CLICK:left/right/middle - Single click");
  Serial1.println("PRESS:left/right    - Hold mouse button");
  Serial1.println("RELEASE:left/right  - Release mouse button");
  Serial1.println("SCROLL:<dx>,<dy>     - Scroll wheel");
  Serial1.println("SCREEN:WIDTH         - Get screen size");
  Serial1.println("PING                 - Connection test");
  Serial1.println("HELP                 - This help text");
}

void handleKeyCommand(const char* arg) {
  // Check for modifier prefixes
  if (strncmp(arg, "SHIFT:", 6) == 0) {
    Keyboard.press(KEY_LEFT_SHIFT);
    typeString(arg + 6);
    Keyboard.release(KEY_LEFT_SHIFT);
    Serial1.println("OK:KEY:SHIFT");
    return;
  }
  if (strncmp(arg, "CTRL:", 5) == 0) {
    Keyboard.press(KEY_LEFT_CTRL);
    typeString(arg + 5);
    Keyboard.release(KEY_LEFT_CTRL);
    Serial1.println("OK:KEY:CTRL");
    return;
  }
  if (strncmp(arg, "ALT:", 4) == 0) {
    Keyboard.press(KEY_LEFT_ALT);
    typeString(arg + 4);
    Keyboard.release(KEY_LEFT_ALT);
    Serial1.println("OK:KEY:ALT");
    return;
  }
  if (strncmp(arg, "META:", 5) == 0) {
    Keyboard.press(KEY_LEFT_GUI);  // Command key on Mac
    typeString(arg + 5);
    Keyboard.release(KEY_LEFT_GUI);
    Serial1.println("OK:KEY:META");
    return;
  }
  
  // Special keys
  if (strcmp(arg, "ENTER") == 0) {
    Keyboard.press(KEY_RETURN);
    Keyboard.release(KEY_RETURN);
    Serial1.println("OK:KEY:ENTER");
  }
  else if (strcmp(arg, "SPACE") == 0) {
    Keyboard.write(' ');
    Serial1.println("OK:KEY:SPACE");
  }
  else if (strcmp(arg, "TAB") == 0) {
    Keyboard.press(KEY_TAB);
    Keyboard.release(KEY_TAB);
    Serial1.println("OK:KEY:TAB");
  }
  else if (strcmp(arg, "BACKSPACE") == 0) {
    Keyboard.press(KEY_BACKSPACE);
    Keyboard.release(KEY_BACKSPACE);
    Serial1.println("OK:KEY:BACKSPACE");
  }
  else if (strcmp(arg, "DELETE") == 0) {
    Keyboard.press(KEY_DELETE);
    Keyboard.release(KEY_DELETE);
    Serial1.println("OK:KEY:DELETE");
  }
  else if (strcmp(arg, "ESCAPE") == 0) {
    Keyboard.press(KEY_ESC);
    Keyboard.release(KEY_ESC);
    Serial1.println("OK:KEY:ESCAPE");
  }
  else if (strcmp(arg, "UP") == 0) {
    Keyboard.press(KEY_UP_ARROW);
    Keyboard.release(KEY_UP_ARROW);
    Serial1.println("OK:KEY:UP");
  }
  else if (strcmp(arg, "DOWN") == 0) {
    Keyboard.press(KEY_DOWN_ARROW);
    Keyboard.release(KEY_DOWN_ARROW);
    Serial1.println("OK:KEY:DOWN");
  }
  else if (strcmp(arg, "LEFT") == 0) {
    Keyboard.press(KEY_LEFT_ARROW);
    Keyboard.release(KEY_LEFT_ARROW);
    Serial1.println("OK:KEY:LEFT");
  }
  else if (strcmp(arg, "RIGHT") == 0) {
    Keyboard.press(KEY_RIGHT_ARROW);
    Keyboard.release(KEY_RIGHT_ARROW);
    Serial1.println("OK:KEY:RIGHT");
  }
  else if (strcmp(arg, "HOME") == 0) {
    Keyboard.press(KEY_HOME);
    Keyboard.release(KEY_HOME);
    Serial1.println("OK:KEY:HOME");
  }
  else if (strcmp(arg, "END") == 0) {
    Keyboard.press(KEY_END);
    Keyboard.release(KEY_END);
    Serial1.println("OK:KEY:END");
  }
  else if (strcmp(arg, "PAGEUP") == 0) {
    Keyboard.press(KEY_PAGE_UP);
    Keyboard.release(KEY_PAGE_UP);
    Serial1.println("OK:KEY:PAGEUP");
  }
  else if (strcmp(arg, "PAGEDOWN") == 0) {
    Keyboard.press(KEY_PAGE_DOWN);
    Keyboard.release(KEY_PAGE_DOWN);
    Serial1.println("OK:KEY:PAGEDOWN");
  }
  else if (strcmp(arg, "CAPS_LOCK") == 0 || strcmp(arg, "CAPSLOCK") == 0) {
    Keyboard.press(KEY_CAPS_LOCK);
    Keyboard.release(KEY_CAPS_LOCK);
    Serial1.println("OK:KEY:CAPSLOCK");
  }
  else if (strcmp(arg, "F1") == 0) {
    Keyboard.press(KEY_F1);
    Keyboard.release(KEY_F1);
    Serial1.println("OK:KEY:F1");
  }
  else if (strcmp(arg, "F2") == 0) {
    Keyboard.press(KEY_F2);
    Keyboard.release(KEY_F2);
    Serial1.println("OK:KEY:F2");
  }
  else if (strcmp(arg, "F3") == 0) {
    Keyboard.press(KEY_F3);
    Keyboard.release(KEY_F3);
    Serial1.println("OK:KEY:F3");
  }
  else if (strcmp(arg, "F4") == 0) {
    Keyboard.press(KEY_F4);
    Keyboard.release(KEY_F4);
    Serial1.println("OK:KEY:F4");
  }
  else {
    // Regular text - type as-is
    typeString(arg);
    Serial1.print("OK:KEY:");
    Serial1.println(arg);
  }
}

void typeString(const char* str) {
  // The Keyboard library handles ASCII->keycode mapping
  // and Shift/Caps automatically. Just write each character.
  while (*str) {
    Keyboard.write(*str);
    str++;
    delay(10);  // Small delay between characters
  }
}

void handleKeycodeCommand(const char* arg) {
  int code = atoi(arg);
  if (code > 0 && code <= 255) {
    Keyboard.press(code);
    Keyboard.release(code);
    Serial1.print("OK:KEYCODE:");
    Serial1.println(code);
  } else {
    Serial1.print("ERR:Invalid keycode: ");
    Serial1.println(arg);
  }
}

void handleModifierCommand(const char* arg) {
  // Parse modifier string (comma-separated)
  char modArg[64];
  strncpy(modArg, arg, sizeof(modArg) - 1);
  modArg[sizeof(modArg) - 1] = '\0';
  
  char* token = strtok(modArg, "|,");
  while (token != NULL) {
    if (strcmp(token, "SHIFT") == 0) {
      shiftHeld = !shiftHeld;
      if (shiftHeld) Keyboard.press(KEY_LEFT_SHIFT);
      else Keyboard.release(KEY_LEFT_SHIFT);
    }
    else if (strcmp(token, "CTRL") == 0 || strcmp(token, "CONTROL") == 0) {
      ctrlHeld = !ctrlHeld;
      if (ctrlHeld) Keyboard.press(KEY_LEFT_CTRL);
      else Keyboard.release(KEY_LEFT_CTRL);
    }
    else if (strcmp(token, "ALT") == 0) {
      altHeld = !altHeld;
      if (altHeld) Keyboard.press(KEY_LEFT_ALT);
      else Keyboard.release(KEY_LEFT_ALT);
    }
    else if (strcmp(token, "META") == 0 || strcmp(token, "GUI") == 0 || strcmp(token, "CMD") == 0) {
      metaHeld = !metaHeld;
      if (metaHeld) Keyboard.press(KEY_LEFT_GUI);
      else Keyboard.release(KEY_LEFT_GUI);
    }
    else if (strcmp(token, "CLEAR") == 0) {
      shiftHeld = ctrlHeld = altHeld = metaHeld = false;
      Keyboard.release(KEY_LEFT_SHIFT);
      Keyboard.release(KEY_LEFT_CTRL);
      Keyboard.release(KEY_LEFT_ALT);
      Keyboard.release(KEY_LEFT_GUI);
    }
    token = strtok(NULL, "|,");
  }
  
  Serial1.println("OK:MODIFIER");
}

void handleMouseCommand(const char* arg) {
  if (strncmp(arg, "ABS:", 4) == 0) {
    // Absolute positioning (0-32767 range used by HID)
    int x, y;
    if (sscanf(arg + 4, "%d,%d", &x, &y) == 2) {
      Mouse.move(x, y, 0);
      Serial1.print("OK:MOUSE:ABS:");
      Serial1.print(x);
      Serial1.print(",");
      Serial1.println(y);
    } else {
      Serial1.println("ERR:Invalid MOUSE:ABS format (use MOUSE:ABS:x,y)");
    }
  } else {
    // Relative movement
    int dx, dy;
    if (sscanf(arg, "%d,%d", &dx, &dy) == 2) {
      Mouse.move(dx, dy, 0);
      Serial1.print("OK:MOUSE:");
      Serial1.print(dx);
      Serial1.print(",");
      Serial1.println(dy);
    } else {
      Serial1.println("ERR:Invalid MOUSE format (use MOUSE:dx,dy or MOUSE:ABS:x,y)");
    }
  }
}

void handleClickCommand(const char* arg) {
  // Case-insensitive comparison
  char upper[16];
  int i = 0;
  while (arg[i] && i < 15) { upper[i] = (char)((arg[i] >= 'a' && arg[i] <= 'z') ? arg[i] - 32 : arg[i]); i++; }
  upper[i] = '\0';
  
  if (strcmp(upper, "LEFT") == 0) {
    Mouse.click(MOUSE_LEFT);
    Serial1.println("OK:CLICK:LEFT");
  }
  else if (strcmp(upper, "RIGHT") == 0) {
    Mouse.click(MOUSE_RIGHT);
    Serial1.println("OK:CLICK:RIGHT");
  }
  else if (strcmp(upper, "MIDDLE") == 0) {
    Mouse.click(MOUSE_MIDDLE);
    Serial1.println("OK:CLICK:MIDDLE");
  }
  else {
    Serial1.print("ERR:Unknown click button: ");
    Serial1.println(arg);
  }
}

void handlePressCommand(const char* arg) {
  char upper[16]; int i = 0;
  while (arg[i] && i < 15) { upper[i] = (char)((arg[i] >= 'a' && arg[i] <= 'z') ? arg[i] - 32 : arg[i]); i++; }
  upper[i] = '\0';
  
  if (strcmp(upper, "LEFT") == 0) {
    Mouse.press(MOUSE_LEFT);
    Serial1.println("OK:PRESS:LEFT");
  }
  else if (strcmp(upper, "RIGHT") == 0) {
    Mouse.press(MOUSE_RIGHT);
    Serial1.println("OK:PRESS:RIGHT");
  }
  else if (strcmp(upper, "MIDDLE") == 0) {
    Mouse.press(MOUSE_MIDDLE);
    Serial1.println("OK:PRESS:MIDDLE");
  }
  else {
    Serial1.print("ERR:Unknown press button: ");
    Serial1.println(arg);
  }
}

void handleReleaseCommand(const char* arg) {
  char upper[16]; int i = 0;
  while (arg[i] && i < 15) { upper[i] = (char)((arg[i] >= 'a' && arg[i] <= 'z') ? arg[i] - 32 : arg[i]); i++; }
  upper[i] = '\0';
  
  if (strcmp(upper, "LEFT") == 0) {
    Mouse.release(MOUSE_LEFT);
    Serial1.println("OK:RELEASE:LEFT");
  }
  else if (strcmp(upper, "RIGHT") == 0) {
    Mouse.release(MOUSE_RIGHT);
    Serial1.println("OK:RELEASE:RIGHT");
  }
  else if (strcmp(upper, "MIDDLE") == 0) {
    Mouse.release(MOUSE_MIDDLE);
    Serial1.println("OK:RELEASE:MIDDLE");
  }
  else {
    Serial1.print("ERR:Unknown release button: ");
    Serial1.println(arg);
  }
}

void handleScrollCommand(const char* arg) {
  int dx, dy;
  if (sscanf(arg, "%d,%d", &dx, &dy) == 2) {
    Mouse.move(0, 0, dy);  // Vertical scroll (positive = down, negative = up)
    Serial1.print("OK:SCROLL:");
    Serial1.print(dx);
    Serial1.print(",");
    Serial1.println(dy);
  } else {
    Serial1.println("ERR:Invalid SCROLL format (use SCROLL:dx,dy)");
  }
}
