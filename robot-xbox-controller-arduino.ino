/*
 * Robot controller with the Xbox Wireless Receiver library on Arduino
 */

#include <XBOXRECV.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

USB Usb;
XBOXRECV Xbox(&Usb);

uint8_t controlling = 0;

void setup() {
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  int done = 0;

  Serial.print(F("\r\nWelcome to the robot controller software!"));

  while (!done) {
    if (Usb.Init() == -1) {
      Serial.print(F("\r\nOSC did not start"));
      delay(2000);
    } else {
      done = 1;
    }
  }
  //Xbox.setLedBlink(ALL, 0);
  Serial.print(F("\r\nPress the Start button to start controlling the robot"));
}

void loop() {
  Usb.Task();
  if (Xbox.XboxReceiverConnected) {
    process_buttons();
  }
}

void process_buttons() {
    for (uint8_t i = 0; i < 4; i++) {
      if (Xbox.Xbox360Connected[i]) {
        if (!controlling) {
          if (Xbox.getButtonClick(START, i)) {
            Xbox.setLedMode(ALTERNATING, i);
            controlling = 1;
          } else {
            return;
          }
        }
        
        if (Xbox.getButtonClick(BACK, i) || Xbox.getButtonClick(XBOX, i) || Xbox.getButtonClick(SYNC, i)) {
          controlling = 0;
          // stop everything
          set_motor(0, 0);
          Xbox.setLedBlink(ALL, i);
          Serial.print(F("Battery: "));
          Serial.print(Xbox.getBatteryLevel(i)); // The battery level in the range 0-3
          return;
        }

        int16_t lh = Xbox.getAnalogHat(LeftHatY, i);
        int16_t rh = Xbox.getAnalogHat(RightHatY, i);
        Serial.print(F("L: "));
        Serial.print(lh);
        Serial.print(F("\t\tR: "));
        Serial.print(rh);
        Serial.println();
      }
    }
}

void set_motor(int16_t left, int16_t right) {
  // FIXME -- do motor control here
}


