/**************************
   StringEEPROM.cpp
   v 1.0.3
   by roncoa@gmail.com
   11/02/2025
 **************************/

#include "StringEEPROM.h"
#include <EEPROM.h>

void StringEEPROM::debugPrint(const __FlashStringHelper* message) {
  if (!debugEnabled || !Serial) return;
  Serial.print(message);
}

void StringEEPROM::debugPrintln(const __FlashStringHelper* message) {
  if (!debugEnabled || !Serial) return;
  Serial.println(message);
}

void StringEEPROM::debugPrintValue(const __FlashStringHelper* message, int value) {
  if (!debugEnabled || !Serial) return;
  Serial.print(message);
  Serial.print(value);
}

void StringEEPROM::debugPrintlnValue(const __FlashStringHelper* message, int value) {
  if (!debugEnabled || !Serial) return;
  Serial.print(message);
  Serial.println(value);
}

void StringEEPROM::debugPrintChar(const char* message) {
  if (!debugEnabled || !Serial) return;
  Serial.print(message);
}

void StringEEPROM::debugPrintlnChar(const char* message) {
  if (!debugEnabled || !Serial) return;
  Serial.println(message);
}

void StringEEPROM::begin(uint32_t baudRate) {
  Serial.begin(baudRate);
  delay(100);
  int numStrings = check();
  if (numStrings >= 0) {
    debugPrintlnValue(F("Found strings: "), numStrings);
    debugPrintlnValue(F("EEPROM size: "), EEPROM.length());
    if (maxStrings > 0) {
      debugPrintlnValue(F("Max strings limit: "), maxStrings);
    }
  } else {
    debugPrintln(F("Invalid EEPROM, initializing..."));
    init();
  }
}

bool StringEEPROM::writeString(uint8_t n, const char* data) {
  if (n < 1) return false;
  int currentStrings = check();
  if (maxStrings > 0) {
    if (n > currentStrings && currentStrings >= maxStrings) {
      debugPrintln(F("Max strings limit reached"));
      return false;
    }
    if (n > maxStrings) {
      debugPrintln(F("Position exceeds max strings limit"));
      return false;
    }
  }
  debugPrintlnValue(F("Writing at position: "), n);
  uint16_t pos = 0;
  uint8_t count = 1;
  uint8_t newLen = data ? strlen(data) : 0;
  debugPrintlnValue(F("New string length: "), newLen);
  uint16_t targetPos = 0;
  uint16_t terminatorPos = 0;
  uint8_t oldLen = 0;
  while (count < n) {
    uint8_t len = EEPROM.read(pos);
    if (len == TERMINATOR) {
      EEPROM.write(pos, 0);
      EEPROM.write(pos + 1, TERMINATOR);
      pos += 1;
      count++;
      continue;
    }
    pos += len + 1;
    count++;
  }
  targetPos = pos;
  oldLen = EEPROM.read(targetPos);
  debugPrintlnValue(F("Target position: "), targetPos);
  debugPrintlnValue(F("Old length: "), oldLen);
  pos = targetPos;
  while (pos < EEPROM.length()) {
    uint8_t len = EEPROM.read(pos);
    if (len == TERMINATOR) {
      terminatorPos = pos;
      break;
    }
    pos += len + 1;
  }
  if (targetPos + 1 + newLen >= EEPROM.length()) {
    debugPrintln(F("Not enough space in EEPROM"));
    return false;
  }
  if (targetPos == terminatorPos) {
    EEPROM.write(targetPos, newLen);
    for (uint8_t i = 0; i < newLen; i++) {
      EEPROM.write(targetPos + 1 + i, data[i]);
    }
    EEPROM.write(targetPos + 1 + newLen, TERMINATOR);
    return true;
  }
  int shift = newLen - oldLen;
  debugPrintlnValue(F("Shift amount: "), shift);
  if (shift != 0) {
    if (terminatorPos + shift >= EEPROM.length()) {
      debugPrintln(F("Not enough space after shift"));
      return false;
    }
    if (shift > 0) {
      for (int i = terminatorPos; i >= targetPos + 1 + oldLen; i--) {
        EEPROM.write(i + shift, EEPROM.read(i));
      }
    } else {
      for (int i = targetPos + 1 + oldLen; i <= terminatorPos; i++) {
        EEPROM.write(i + shift, EEPROM.read(i));
      }
    }
  }
  EEPROM.write(targetPos, newLen);
  for (uint8_t i = 0; i < newLen; i++) {
    EEPROM.write(targetPos + 1 + i, data[i]);
  }
  debugPrintln(F("Write successful"));
  return true;
}

int StringEEPROM::readString(uint8_t n, char* buffer, int maxLength) {
  if (n < 1) {
    buffer[0] = '\0';
    return -1;
  }
  if (maxStrings > 0 && n > maxStrings) {
    debugPrintln(F("Position exceeds max strings limit"));
    buffer[0] = '\0';
    return -1;
  }
  uint16_t pos = 0;
  uint8_t count = 1;
  while (count < n) {
    uint8_t len = EEPROM.read(pos);
    if (len == TERMINATOR) {
      buffer[0] = '\0';
      debugPrintln(F("String not found"));
      return -1;
    }
    pos += len + 1;
    count++;
  }
  uint8_t len = EEPROM.read(pos);
  if (len == TERMINATOR) {
    buffer[0] = '\0';
    debugPrintln(F("String not found"));
    return -1;
  }
  int copyLen = min((int)len, maxLength - 1);
  for (int i = 0; i < copyLen; i++) {
    buffer[i] = EEPROM.read(pos + 1 + i);
  }
  buffer[copyLen] = '\0';
  return len;
}

int StringEEPROM::check() {
  uint16_t pos = 0;
  int count = 0;
  while (pos < EEPROM.length()) {
    uint8_t len = EEPROM.read(pos);
    if (len == TERMINATOR) return count;
    if (pos + 1 + len >= EEPROM.length()) return -1;
    pos += len + 1;
    count++;
  }
  return -1;
}

void StringEEPROM::init() {
  debugPrintln(F("Initializing EEPROM..."));
  EEPROM.write(0, TERMINATOR);
  debugPrintln(F("EEPROM initialized"));
}

void StringEEPROM::showAllStrings() {
  int numStrings = check();
  if (numStrings < 0) {
    debugPrintln(F("Invalid EEPROM!"));
    return;
  }
  debugPrintlnValue(F("Number of strings: "), numStrings);
  char buf[STRING_EEPROM_MAX_INPUT];
  uint16_t pos = 0;
  for (uint8_t n = 1; n <= numStrings; n++) {
    int len = readString(n, buf, sizeof(buf));
    if (len >= 0) {
      debugPrintValue(F("String read from position "), n);
      debugPrint(F("="));
      debugPrintlnChar(buf);
    }
    uint8_t currentLen = EEPROM.read(pos);
    pos += currentLen + 1;
  }
}

void StringEEPROM::printHelp() {
  debugPrintln(F("Available commands:"));
  debugPrintln(F("N=string   - Write string at position N"));
  debugPrintln(F("?          - Show all strings"));
  debugPrintln(F("#          - Show number of strings"));
  debugPrintln(F("!          - Initialize EEPROM"));
  debugPrintln(F("h          - Show help"));
}

void StringEEPROM::handleSerial() {
  if (!Serial) return;
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      inputBuffer[inputIndex] = '\0';

      if (inputIndex == 0) {
        // Skip empty lines
      }
      else if (inputBuffer[0] == '?') {
        showAllStrings();
      }
      else if (inputBuffer[0] == '#') {
        debugPrintlnValue(F("Number of strings: "), check());
      }
      else if (inputBuffer[0] == '!') {
        debugPrintln(F("Are you sure? (y/n)"));
        while (!Serial.available());
        if (Serial.read() == 'y') {
          init();
        } else {
          debugPrintln(F("Cancelled"));
        }
      }
      else if (inputBuffer[0] == 'h') {
        printHelp();
      }
      else {
        char* equalSign = strchr(inputBuffer, '=');
        if (equalSign != NULL) {
          *equalSign = '\0';
          uint8_t num = atoi(inputBuffer);
          char* str = equalSign + 1;
          bool ok = writeString(num, str);
          //          if (ok) {
          //            debugPrintln(F("Write successful"));
          //            showAllStrings();
          //          } else {
          //            debugPrintln(F("Write failed"));
          //          }
        } else {
          debugPrintln(F("Invalid command. 'h' for help"));
        }
      }
      inputIndex = 0;
    }
    else if (inputIndex < STRING_EEPROM_MAX_INPUT - 1) {
      inputBuffer[inputIndex++] = c;
    }
  }
}
