/*
 * Arduino HID Controller - Leonardo Edition
 * Letters, numbers, and common symbols
 * 
 * HARDWARE: Connect USB-to-Serial adapter to Leonardo RX(0)/TX(1) pins
 * - USB-Serial RX  → Leonardo TX (pin 1)
 * - USB-Serial TX  → Leonardo RX (pin 0)
 * - GND           → GND
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
    Keyboard.press(KEY_LEFT_SHIFT);
    typeText(arg + 6);
    Keyboard.release(KEY_LEFT_SHIFT);
    Serial1.println("OK");
    return;
  }
  if (strncmp(arg, "CTRL:", 5) == 0) {
    Keyboard.press(KEY_LEFT_CTRL);
    typeText(arg + 5);
    Keyboard.release(KEY_LEFT_CTRL);
    Serial1.println("OK");
    return;
  }
  if (strncmp(arg, "WIN:", 4) == 0) {
    Keyboard.press(KEY_LEFT_GUI);
    typeText(arg + 4);
    Keyboard.release(KEY_LEFT_GUI);
    Serial1.println("OK");
    return;
  }
  if (strcmp(arg, "WIN") == 0) {
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.release(KEY_LEFT_GUI);
    Serial1.println("OK");
    return;
  }
  
  // Special keys
  if (strcmp(arg, "ENTER") == 0) { Keyboard.press(KEY_RETURN); Keyboard.release(KEY_RETURN); Serial1.println("OK"); return; }
  if (strcmp(arg, "SPACE") == 0) { Keyboard.write(' '); Serial1.println("OK"); return; }
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
  
  // F1-F4
  if (strcmp(arg, "F1") == 0) { Keyboard.press(KEY_F1); Keyboard.release(KEY_F1); Serial1.println("OK"); return; }
  if (strcmp(arg, "F2") == 0) { Keyboard.press(KEY_F2); Keyboard.release(KEY_F2); Serial1.println("OK"); return; }
  if (strcmp(arg, "F3") == 0) { Keyboard.press(KEY_F3); Keyboard.release(KEY_F3); Serial1.println("OK"); return; }
  if (strcmp(arg, "F4") == 0) { Keyboard.press(KEY_F4); Keyboard.release(KEY_F4); Serial1.println("OK"); return; }
  // F5-F12
  if (strcmp(arg, "F5") == 0) { Keyboard.press(KEY_F5); Keyboard.release(KEY_F5); Serial1.println("OK"); return; }
  if (strcmp(arg, "F6") == 0) { Keyboard.press(KEY_F6); Keyboard.release(KEY_F6); Serial1.println("OK"); return; }
  if (strcmp(arg, "F7") == 0) { Keyboard.press(KEY_F7); Keyboard.release(KEY_F7); Serial1.println("OK"); return; }
  if (strcmp(arg, "F8") == 0) { Keyboard.press(KEY_F8); Keyboard.release(KEY_F8); Serial1.println("OK"); return; }
  if (strcmp(arg, "F9") == 0) { Keyboard.press(KEY_F9); Keyboard.release(KEY_F9); Serial1.println("OK"); return; }
  if (strcmp(arg, "F10") == 0) { Keyboard.press(KEY_F10); Keyboard.release(KEY_F10); Serial1.println("OK"); return; }
  if (strcmp(arg, "F11") == 0) { Keyboard.press(KEY_F11); Keyboard.release(KEY_F11); Serial1.println("OK"); return; }
  if (strcmp(arg, "F12") == 0) { Keyboard.press(KEY_F12); Keyboard.release(KEY_F12); Serial1.println("OK"); return; }
  
  // Regular text
  typeText(arg);
  Serial1.println("OK");
}

// Use Keyboard.write() for reliable ASCII mapping
void typeText(const char* str) {
  while (*str) {
    Keyboard.write(*str);
    str++;
    delay(10);
  }
}
