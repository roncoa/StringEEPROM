/***********************************
  ButtonBox_ACC_HW1.0_SW1.1
  Arduino pro micro 32U4 MCU
  Core Arduino AVR Boards 1.8.6
 ***********************************/

#include <KeySequence.h>              // libreria KeySequence alla versione 1.3.0
#include <SimRacingController.h>      // libreria SimRacingController alla versione 2.1.0
#include <StringEEPROM.h>             // libreria StringEEPROM alla versione 1.0.0
#include "Sequenze.h"                 // macro tastiera per ACC

// Debug configuration
#define DEBUG true

// Hardware configuration
// Matrix Configuration
#define MATRIX_ROWS 3
#define MATRIX_COLS 5
const int rowPins[MATRIX_ROWS] = {2, 3, 4};
const int colPins[MATRIX_COLS] = {5, 6, 7, 8, 9};

// Encoder Configuration
#define NUM_ENCODERS 4
const int encoderPinsA[NUM_ENCODERS] = {20, 18, 14, 10};
const int encoderPinsB[NUM_ENCODERS] = {21, 19, 15, 16};

// Create instances
KeySequence keys;
SimRacingController controller;
StringEEPROM eeprom;

// Error callback
bool onError(const ControllerError& error) {
  if (DEBUG) {
    Serial.print("Error: ");
    Serial.println(error.message);
  }
  return true;
}

// Matrix button callback
void onMatrixChange(int profile, int row, int col, bool state) {
  if (profile == 0) { // ACC profile
    if (state) {
      char buffer[64];
      int len;
      // Row 1
      if (row == 0) {
        switch (col) {
          case 0: keys.sendSequence(ACC_EngagePitLimiter); break;
          case 1: keys.sendSequence(ACC_CycleCarLightStages); break;
          case 2: keys.sendSequence(ACC_LeftDirectionalLight); break;
          case 3: keys.sendSequence(ACC_RightDirectionalLight); break;
          case 4: keys.sendSequence(ACC_CycleMultifunctionDisplay); break;
        }
      }
      // Row 2
      else if (row == 1) {
        switch (col) {
          case 0: keys.sendSequence(ACC_Starter); break;
          case 1: keys.sendSequence(ACC_EnableRainLights); break;
          case 2: keys.sendSequence(ACC_EnableFlashingLights); break;
          case 3: keys.sendSequence(ACC_CycleWiper); break;
          case 4: keys.sendSequence(ACC_Savereplay); break;
        }
      }
      // Row 3
      else if (row == 2) {
        switch (col) {
          case 0: keys.sendSequence(ACC_IngitionSequence); break;
          case 1: keys.sendSequence(ACC_DecreaseTCC); break;
          case 2: keys.sendSequence(ACC_IncreaseTCC); break;
          case 3:
            len = eeprom.readString(1, buffer, sizeof(buffer));
            if (len < 0) {
              strncpy(buffer, AUX1, sizeof(buffer));
              buffer[sizeof(buffer) - 1] = '\0';
            }
            keys.sendSequence(buffer);
            break;
          case 4:
            len = eeprom.readString(2, buffer, sizeof(buffer));
            if (len < 0) {
              strncpy(buffer, AUX2, sizeof(buffer));
              buffer[sizeof(buffer) - 1] = '\0';
            }
            keys.sendSequence(buffer);
            break;
        }
      }
    } else {
      keys.releaseAll();
    }
  }
}

// Encoder callback
void onEncoderChange(int profile, int encoder, int direction) {
  if (profile == 0) { // ACC profile
    switch (encoder) {
      case 0:  // Enc 1
        keys.sendSequence(direction > 0 ? ACC_IncreaseTC : ACC_DecreaseTC);
        break;
      case 1:  // Enc 2
        keys.sendSequence(direction > 0 ? ACC_IncreaseABS : ACC_DecreaseABS);
        break;
      case 2:  // Enc 3
        keys.sendSequence(direction > 0 ? ACC_IncreaseEngineMap : ACC_DecreaseEngineMap);
        break;
      case 3:  // Enc 4
        keys.sendSequence(direction > 0 ? ACC_IncreaseBrakeBias : ACC_DecreaseBrakeBIas);
        break;
    }
    keys.releaseAll();
  }
}

void setup() {
  if (DEBUG) {
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println(F("ButtonBox_ACC_HW1.0_SW1.1 Arduino pro micro 32U4 MCU"));
  }

  // Initialize StringEEPROM
  eeprom.setDebug(true);
  eeprom.setMaxStrings(2);
  eeprom.begin();


  if (eeprom.check() != eeprom.getMaxStrings())
  {
    if (DEBUG) {
      Serial.println("Initializing strings (AUX1, AUX2)");
    }
    eeprom.writeString(1, AUX1);
    eeprom.writeString(2, AUX2);
  }



  // Initialize KeySequence
  keys.setDebug(DEBUG);
  keys.begin();
  keys.setAutoRelease(false);
  keys.setDefaultDelay(150);
  // Configure controller
  controller.setMatrix(rowPins, MATRIX_ROWS, colPins, MATRIX_COLS);
  controller.setEncoders(encoderPinsA, encoderPinsB, NUM_ENCODERS);
  // Set callbacks
  controller.setErrorCallback(onError);
  controller.setMatrixCallback(onMatrixChange);
  controller.setEncoderCallback(onEncoderChange);
  // Set encoder sensitivity
  for (int i = 0; i < NUM_ENCODERS; i++) {
    controller.setEncoderDivisor(i, 4);
  }
  // Initialize controller
  if (!controller.begin()) {
    if (DEBUG) {
      Serial.println("Error: " + String(controller.getLastError().message));
    }
    while (1); // Stop if initialization fails
  }
  if (DEBUG) {
    Serial.println(F("Controller initialized!"));
  }
  delay(1000);
}

void loop() {
  controller.update();
  eeprom.handleSerial();
}
