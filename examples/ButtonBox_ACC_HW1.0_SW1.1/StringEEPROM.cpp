#include "StringEEPROM.h"
#include <EEPROM.h>

void StringEEPROM::debugPrint(const __FlashStringHelper* message) {
  if (!debugEnabled || !Serial) return;
  Serial.print(F("Debug: "));
  Serial.println(message);
}

void StringEEPROM::debugPrintValue(const __FlashStringHelper* message, int value) {
  if (!debugEnabled || !Serial) return;
  Serial.print(F("Debug: "));
  Serial.print(message);
  Serial.println(value);
}

void StringEEPROM::begin(uint32_t baudRate) {
  Serial.begin(baudRate);
  
  int numStrings = check();
  if (numStrings >= 0) {
    if (debugEnabled) {
      debugPrintValue(F("Found strings:"), numStrings);
    }
  } else {
    debugPrint(F("Invalid EEPROM, initializing..."));
    init();
  }
  
  printHelp();
}

bool StringEEPROM::writeString(uint8_t n, const char* data) {
  if (n < 1) return false;

  // Check maxStrings limit
  if (maxStrings > 0) {
    int numStrings = check();
    if (numStrings >= maxStrings && n > numStrings) {
      debugPrint(F("Max strings limit reached"));
      return false;
    }
  }

  debugPrintValue(F("Writing at position"), n);
  
  uint16_t pos = 0;
  uint8_t count = 1;
  uint8_t newLen = data ? strlen(data) : 0;
  
  debugPrintValue(F("New string length"), newLen);

  // Calculate target position
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

  debugPrintValue(F("Target position"), targetPos);
  debugPrintValue(F("Old length"), oldLen);

  // Find terminator
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
    debugPrint(F("Not enough space in EEPROM"));
    return false;
  }

  // Write at terminator position
  if (targetPos == terminatorPos) {
    EEPROM.write(targetPos, newLen);
    for (uint8_t i = 0; i < newLen; i++) {
      EEPROM.write(targetPos + 1 + i, data[i]);
    }
    EEPROM.write(targetPos + 1 + newLen, TERMINATOR);
    return true;
  }

  // Calculate and check shift
  int shift = newLen - oldLen;
  debugPrintValue(F("Shift amount"), shift);

  if (shift != 0) {
    if (terminatorPos + shift >= EEPROM.length()) {
      debugPrint(F("Not enough space after shift"));
      return false;
    }

    // Move data
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

  // Write new string
  EEPROM.write(targetPos, newLen);
  for (uint8_t i = 0; i < newLen; i++) {
    EEPROM.write(targetPos + 1 + i, data[i]);
  }

  return true;
}

int StringEEPROM::readString(uint8_t n, char* buffer, int maxLength) {
  if (n < 1) {
    buffer[0] = '\0';
    return -1;
  }

  uint16_t pos = 0;
  uint8_t count = 1;

  while (count < n) {
    uint8_t len = EEPROM.read(pos);
    if (len == TERMINATOR) {
      buffer[0] = '\0';
      return -1;
    }
    pos += len + 1;
    count++;
  }

  uint8_t len = EEPROM.read(pos);
  if (len == TERMINATOR) {
    buffer[0] = '\0';
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
  EEPROM.write(0, TERMINATOR);
}

void StringEEPROM::showAllStrings() {
  int numStrings = check();
  if (numStrings < 0) {
    Serial.println(F("Invalid EEPROM!"));
    return;
  }

  Serial.print(F("Found "));
  Serial.print(numStrings);
  Serial.println(F(" strings:"));

  char buf[STRING_EEPROM_MAX_INPUT];
  uint16_t pos = 0;

  for (uint8_t n = 1; n <= numStrings; n++) {
    int len = readString(n, buf, sizeof(buf));
    Serial.print(n);
    Serial.print(F(" [pos="));
    Serial.print(pos);
    Serial.print(F(" len="));
    Serial.print(len);
    Serial.print(F("]: '"));
    Serial.print(buf);
    Serial.println(F("'"));

    uint8_t currentLen = EEPROM.read(pos);
    pos += currentLen + 1;
  }
}

void StringEEPROM::printHelp() {
  Serial.println(F("Available commands:"));
  Serial.println(F("N=string   - Write string at position N"));
  Serial.println(F("?          - Show all strings"));
  Serial.println(F("#          - Show number of strings"));
  Serial.println(F("!          - Initialize EEPROM"));
  Serial.println(F("h          - Show help"));
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
        int n = check();
        Serial.print(F("Number of strings: "));
        Serial.println(n);
      }
      else if (inputBuffer[0] == '!') {
        Serial.println(F("Are you sure? (y/n)"));
        while (!Serial.available());
        if (Serial.read() == 'y') {
          init();
          Serial.println(F("EEPROM initialized"));
        } else {
          Serial.println(F("Cancelled"));
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
          Serial.print(F("Write at position "));
          Serial.print(num);
          Serial.print(F(": "));
          Serial.println(ok ? F("OK") : F("ERROR"));

          if (ok && debugEnabled) {
            showAllStrings();
          }
        } else {
          Serial.println(F("Invalid command. 'h' for help"));
        }
      }

      inputIndex = 0;
    }
    else if (inputIndex < STRING_EEPROM_MAX_INPUT - 1) {
      inputBuffer[inputIndex++] = c;
    }
  }
}
