/*
 * Arduino HID Controller - Leonardo Edition
 * Simple version: letters and numbers only
 * 
 * HARDWARE: Connect USB-to-Serial adapter to Leonardo RX(0)/TX(1) pins
 * - USB-Serial RX  → Leonardo TX (pin 1)
 * - USB-Serial TX  → Leonardo RX (pin 0)
 * - GND           → GND
 * 
 * Command Protocol:
 *   KEY:<text>           - Type text (letters + numbers only)
 *   KEY:ENTER/SPACE/TAB/ESCAPE/BACKSPACE
 *   KEY:UP/DOWN/LEFT/RIGHT
 *   KEY:F1-F4
 *   KEY:SHIFT:<text>    - Type uppercase
 *   KEY:CTRL:<text>     - Type with Ctrl
 *   CLICK:left/right/middle
 *   MOUSE:<dx>,<dy>
 *   PING
 *   HELP
 */

#include <Keyboard.h>
#include <Mouse.h>

#define BUF_SIZE 64
char buffer[BUF_SIZE];
int bufPos = 0;

void setup() {
  Serial1.begin(115200);
  Keyboard.begin();
  Mouse.begin();
  while (!Serial1) { }
  Serial1.println("Arduino HID Ready");
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
    } else if (bufPos < BUF_SIZE - 1) {
      buffer[bufPos++] = c;
    }
  }
}

void processCommand(const char* cmd) {
  if (cmd[0] == '\0') return;
  
  if (strcmp(cmd, "PING") == 0) {
    Serial1.println("PONG");
  }
  else if (strcmp(cmd, "HELP") == 0) {
    Serial1.println("KEY:<text> CLICK:left/right MOUSE:dx,dy PING HELP");
  }
  else if (strncmp(cmd, "KEY:", 4) == 0) {
    handleKey(cmd + 4);
  }
  else if (strcmp(cmd, "RELEASEALL") == 0) {
    Keyboard.releaseAll();
    Mouse.release(MOUSE_LEFT);
    Mouse.release(MOUSE_RIGHT);
    Mouse.release(MOUSE_MIDDLE);
    Serial1.println("OK");
  }
  else if (strncmp(cmd, "MOUSE:", 6) == 0) {
    int dx, dy;
    if (sscanf(cmd + 6, "%d,%d", &dx, &dy) == 2) {
      Mouse.move(dx, dy, 0);
      Serial1.println("OK");
    }
  }
  else if (strncmp(cmd, "CLICK:", 6) == 0) {
    if (strcmp(cmd + 6, "left") == 0) Mouse.click(MOUSE_LEFT);
    else if (strcmp(cmd + 6, "right") == 0) Mouse.click(MOUSE_RIGHT);
    else if (strcmp(cmd + 6, "middle") == 0) Mouse.click(MOUSE_MIDDLE);
    Serial1.println("OK");
  }
  else {
    Serial1.println("ERR");
  }
  
  Keyboard.releaseAll();
}

void handleKey(const char* arg) {
  // Modifier prefixes
  if (strncmp(arg, "SHIFT:", 6) == 0) {
    typeString(arg + 6, true);
    Serial1.println("OK");
    return;
  }
  if (strncmp(arg, "CTRL:", 5) == 0) {
    typeStringMod(arg + 5, KEY_LEFT_CTRL);
    Serial1.println("OK");
    return;
  }
  
  // Special keys
  if (strcmp(arg, "ENTER") == 0) { Keyboard.press(KEY_RETURN); Keyboard.release(KEY_RETURN); Serial1.println("OK"); return; }
  if (strcmp(arg, "SPACE") == 0) { Keyboard.press(0x2C); Keyboard.release(0x2C); Serial1.println("OK"); return; }
  if (strcmp(arg, "TAB") == 0) { Keyboard.press(KEY_TAB); Keyboard.release(KEY_TAB); Serial1.println("OK"); return; }
  if (strcmp(arg, "ESCAPE") == 0 || strcmp(arg, "ESC") == 0) { Keyboard.press(KEY_ESC); Keyboard.release(KEY_ESC); Serial1.println("OK"); return; }
  if (strcmp(arg, "BACKSPACE") == 0 || strcmp(arg, "BS") == 0) { Keyboard.press(KEY_BACKSPACE); Keyboard.release(KEY_BACKSPACE); Serial1.println("OK"); return; }
  if (strcmp(arg, "DELETE") == 0) { Keyboard.press(KEY_DELETE); Keyboard.release(KEY_DELETE); Serial1.println("OK"); return; }
  if (strcmp(arg, "UP") == 0) { Keyboard.press(KEY_UP_ARROW); Keyboard.release(KEY_UP_ARROW); Serial1.println("OK"); return; }
  if (strcmp(arg, "DOWN") == 0) { Keyboard.press(KEY_DOWN_ARROW); Keyboard.release(KEY_DOWN_ARROW); Serial1.println("OK"); return; }
  if (strcmp(arg, "LEFT") == 0) { Keyboard.press(KEY_LEFT_ARROW); Keyboard.release(KEY_LEFT_ARROW); Serial1.println("OK"); return; }
  if (strcmp(arg, "RIGHT") == 0) { Keyboard.press(KEY_RIGHT_ARROW); Keyboard.release(KEY_RIGHT_ARROW); Serial1.println("OK"); return; }
  if (strcmp(arg, "HOME") == 0) { Keyboard.press(KEY_HOME); Keyboard.release(KEY_HOME); Serial1.println("OK"); return; }
  if (strcmp(arg, "END") == 0) { Keyboard.press(KEY_END); Keyboard.release(KEY_END); Serial1.println("OK"); return; }
  if (strcmp(arg, "PAGEUP") == 0) { Keyboard.press(KEY_PAGE_UP); Keyboard.release(KEY_PAGE_UP); Serial1.println("OK"); return; }
  if (strcmp(arg, "PAGEDOWN") == 0) { Keyboard.press(KEY_PAGE_DOWN); Keyboard.release(KEY_PAGE_DOWN); Serial1.println("OK"); return; }
  if (strcmp(arg, "CAPSLOCK") == 0) { Keyboard.press(KEY_CAPS_LOCK); Keyboard.release(KEY_CAPS_LOCK); Serial1.println("OK"); return; }
  if (strcmp(arg, "F1") == 0) { Keyboard.press(KEY_F1); Keyboard.release(KEY_F1); Serial1.println("OK"); return; }
  if (strcmp(arg, "F2") == 0) { Keyboard.press(KEY_F2); Keyboard.release(KEY_F2); Serial1.println("OK"); return; }
  if (strcmp(arg, "F3") == 0) { Keyboard.press(KEY_F3); Keyboard.release(KEY_F3); Serial1.println("OK"); return; }
  if (strcmp(arg, "F4") == 0) { Keyboard.press(KEY_F4); Keyboard.release(KEY_F4); Serial1.println("OK"); return; }
  
  // Regular text: letters and numbers only
  typeString(arg, false);
  Serial1.println("OK");
}

void typeString(const char* str, bool upper) {
  while (*str) {
    char c = *str;
    uint8_t k = 0;
    
    if (c >= 'a' && c <= 'z') k = 0x04 + (c - 'a');
    else if (c >= 'A' && c <= 'Z') k = 0x04 + (c - 'A');
    else if (c >= '0' && c <= '9') k = (c == '0') ? 0x27 : 0x1E + (c - '1');
    else if (c == ' ') k = 0x2C;
    
    if (k) {
      if (upper) Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press(k);
      delay(1);
      Keyboard.releaseAll();
    }
    str++;
    delay(10);
  }
}

void typeStringMod(const char* str, uint8_t mod) {
  Keyboard.press(mod);
  while (*str) {
    char c = *str;
    uint8_t k = 0;
    
    if (c >= 'a' && c <= 'z') k = 0x04 + (c - 'a');
    else if (c >= 'A' && c <= 'Z') k = 0x04 + (c - 'A');
    else if (c >= '0' && c <= '9') k = (c == '0') ? 0x27 : 0x1E + (c - '1');
    
    if (k) {
      Keyboard.press(k);
      delay(1);
      Keyboard.releaseAll();
    }
    str++;
    delay(10);
  }
  Keyboard.release(mod);
}
