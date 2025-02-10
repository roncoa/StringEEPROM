/**************************
   StringEEPROM.h
   v 1.0.2
   by roncoa@gmail.com
   06/02/2025
 **************************/

#ifndef STRING_EEPROM_H
#define STRING_EEPROM_H

#include <Arduino.h>

#define STRING_EEPROM_MAX_INPUT 64

class StringEEPROM {
  public:
    StringEEPROM() : debugEnabled(false), inputIndex(0), maxStrings(-1) {}

    void setDebug(bool enable) {
      debugEnabled = enable;
    }
    bool isDebugEnabled() const {
      return debugEnabled;
    }

    void setMaxStrings(int max) {
      maxStrings = max;
    }
    int getMaxStrings() const {
      return maxStrings;
    }

    bool writeString(uint8_t n, const char* data);
    int readString(uint8_t n, char* buffer, int maxLength);
    int check();
    void init();
    void begin(uint32_t baudRate = 115200);
    void handleSerial();
    void showAllStrings();
    void printHelp();

  private:
    void debugPrint(const __FlashStringHelper* message);
    void debugPrintln(const __FlashStringHelper* message);
    void debugPrintValue(const __FlashStringHelper* message, int value);
    void debugPrintlnValue(const __FlashStringHelper* message, int value);
    void debugPrintChar(const char* message);
    void debugPrintlnChar(const char* message);

    static const uint8_t TERMINATOR = 255;
    char inputBuffer[STRING_EEPROM_MAX_INPUT];
    int inputIndex;
    bool debugEnabled;
    int maxStrings;
};

#endif
