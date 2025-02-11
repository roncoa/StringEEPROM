# StringEEPROM Library

[![GitHub release](https://img.shields.io/github/release/roncoa/StringEEPROM.svg)](https://github.com/roncoa/StringEEPROM/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/roncoa/StringEEPROM.svg?style=social&label=Star)](https://github.com/roncoa/StringEEPROM/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/roncoa/StringEEPROM.svg?style=social&label=Fork)](https://github.com/roncoa/StringEEPROM/network)

If you like **StringEEPROM Library**... buy me a beer  [![donate](https://img.shields.io/badge/donate-PayPal-blue.svg)](https://paypal.me/roncoa)

A library for efficiently storing and managing multiple strings in EEPROM memory for Arduino and ESP32 platforms.

## Features

- Store multiple strings in EEPROM with dynamic allocation
- Command interface through Serial for interactive use
- Configurable debug output
- Configurable maximum number of strings
- Memory efficient implementation using flash memory for strings
- Compatible with both Arduino and ESP32 platforms

## Installation

### Using Arduino Library Manager (Recommended)
1. Open Arduino IDE
2. Go to Tools > Manage Libraries...
3. Search for "StringEEPROM"
4. Click Install

### Manual Installation
1. Download latest release as ZIP
2. In Arduino IDE: Sketch > Include Library > Add .ZIP Library
3. Select downloaded ZIP file
4. Restart Arduino IDE

## Storage Format

The library uses the following format to store strings in EEPROM:
```
[len1][data1][len2][data2]...[255]
```
where:
- `len` = length byte (0-254)
- `data` = string content
- `255` = terminator marking the end of all data

## Class Reference

### Constructor

```cpp
StringEEPROM()
```
Creates a new StringEEPROM instance with debug disabled and no limit on number of strings.

### Configuration Methods

```cpp
void setDebug(bool enable)
```
Enable or disable debug messages over Serial.

```cpp
bool isDebugEnabled() const
```
Returns current debug status.

```cpp
void setMaxStrings(int max)
```
Set maximum number of strings that can be stored. Use -1 for unlimited (default).

```cpp
int getMaxStrings() const
```
Get current maximum strings limit.

### Core Methods

```cpp
void begin(uint32_t baudRate = 115200)
```
Initialize the library and Serial communication.
- `baudRate`: Optional baud rate for Serial (default 115200)

```cpp
bool writeString(uint8_t n, const char* data)
```
Write a string to the specified position.
- `n`: Position (1-based index)
- `data`: String to write
- Returns: true if successful, false otherwise

```cpp
int readString(uint8_t n, char* buffer, int maxLength)
```
Read a string from the specified position.
- `n`: Position to read (1-based index)
- `buffer`: Buffer to store the string
- `maxLength`: Maximum length of the buffer
- Returns: Length of string or -1 if not found

```cpp
int check()
```
Check EEPROM content validity.
- Returns: Number of strings stored, or -1 if EEPROM content is invalid

```cpp
void init()
```
Initialize EEPROM by writing terminator at start.

### Debug Methods

```cpp
void debugPrint(const __FlashStringHelper* message)
void debugPrintln(const __FlashStringHelper* message)
```
Print debug message from flash memory, with or without newline.

```cpp
void debugPrintValue(const __FlashStringHelper* message, int value)
void debugPrintlnValue(const __FlashStringHelper* message, int value)
```
Print debug message with integer value, with or without newline.

```cpp
void debugPrintChar(const char* message)
void debugPrintlnChar(const char* message)
```
Print debug message from RAM, with or without newline.

### Serial Interface Methods

```cpp
void handleSerial()
```
Process Serial commands. Should be called in loop().

```cpp
void showAllStrings()
```
Display all stored strings with their positions.

```cpp
void printHelp()
```
Display available Serial commands.

## Serial Commands

The library provides an interactive interface through Serial with the following commands:

- `N=string` - Write "string" at position N (e.g., "1=Hello")
- `?` - Show all strings
- `#` - Show number of strings
- `!` - Initialize EEPROM (requires confirmation)
- `h` - Show help message

## Example Usage

Basic usage:
```cpp
#include <StringEEPROM.h>

StringEEPROM eeprom;

void setup() {
  // Optional: set maximum number of strings
  eeprom.setMaxStrings(10);
  
  // Optional: enable debug messages
  eeprom.setDebug(true);
  
  // Initialize with default baud rate (115200)
  eeprom.begin();
}

void loop() {
  // Handle Serial commands
  eeprom.handleSerial();
}
```

## Memory Considerations

- Each string requires length + 1 bytes of EEPROM space (length byte + string content)
- The library uses flash memory for constant strings to save RAM
- Maximum string length is 254 bytes due to length byte limitations
- EEPROM space is used efficiently with dynamic allocation

## Limitations

- Maximum string length is 254 bytes
- String positions are 1-based
- String content cannot contain byte value 255 (used as terminator)
- ESP32 virtual EEPROM requires commit() after changes

#### Platform Support
- AVR architecture
- ESP32 virtual EEPROM
- Generic Arduino
- Memory abstraction
- Platform-specific optimizations

## Contributing
1. Fork repository
2. Create feature branch
3. Submit pull request

## Support
- GitHub Issues: Technical issues, bugs
- GitHub Discussions: Questions, ideas
- Email: your.email@example.com

## License
MIT License - See [LICENSE](LICENSE) file

## Version History

### 1.0.3
- Bug fix

### 1.0.0
- Initial release
- Basic string storage
- Serial interface
- Debug system
- ESP32 support

## Author
roncoa@gmail.com
