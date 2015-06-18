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

// per experience, this is the max speed backward and forward
#define MAX_BACKWARD 77
#define MAX_FORWARD 177
#define MIDDLE_POINT 127
#define CONTROLLER_RATIO 656

#define MAX_CONTROLLER 32767
#define MIN_CONTROLLER -32768

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
          set_motor(MIDDLE_POINT, MIDDLE_POINT);
          Xbox.setLedBlink(ALL, i);
          Serial.print(F("Battery: "));
          Serial.print(Xbox.getBatteryLevel(i)); // The battery level in the range 0-3
          return;
        }

        if (Xbox.getButtonPress(L2, i) && Xbox.getButtonPress(R2, i)) {
          set_motor(MAX_FORWARD, MAX_FORWARD);
          return;
        }
        if (Xbox.getButtonPress(L2, i)) {
          set_motor(MAX_FORWARD, MAX_BACKWARD);
          return;
        }
        if (Xbox.getButtonPress(R2, i)) {
          set_motor(MAX_BACKWARD, MAX_FORWARD);
          return;
        }
        
        long vertical = long(Xbox.getAnalogHat(LeftHatY, i));
        long horizontal = long(Xbox.getAnalogHat(LeftHatX, i));

        if (vertical > 0) { // forwards with skew left/right
          if (horizontal > 0) { // right
            if (vertical+horizontal >= MAX_CONTROLLER) {
              int16_t right = int(MAX_CONTROLLER-horizontal);
              set_motor(c2m_scale(MAX_CONTROLLER), c2m_scale(right));
              return;
            }
          } else { //left
            if (vertical-horizontal >= MAX_CONTROLLER) {
              int16_t left = int(MAX_CONTROLLER+horizontal);
              set_motor(c2m_scale(left), c2m_scale(MAX_CONTROLLER));
              return;
            }
          }
          set_motor(c2m_scale(vertical+horizontal), c2m_scale(vertical-horizontal));
          return;
        }

        // vertical < 0 (backwards)

        if (horizontal < 0) { // joystick left 
          if (vertical+horizontal <= MIN_CONTROLLER) {
            int16_t left = MIN_CONTROLLER-horizontal;
            set_motor(c2m_scale(left), c2m_scale(MIN_CONTROLLER));
            return;
          }
        } else { // right
          if (vertical-horizontal <= MIN_CONTROLLER) {
            int16_t right = int(MIN_CONTROLLER+horizontal);
            set_motor(c2m_scale(MIN_CONTROLLER), c2m_scale(right));
            return;
          }          
        }
        set_motor(c2m_scale(vertical-horizontal), c2m_scale(vertical+horizontal));
        return;
      }
    }
}

// dead zone threshold for the xbox joystick
#define DEAD_ZONE_THRESHOLD 6000

uint8_t c2m_scale(int16_t stick) {
  // establish a deadzone
  if (abs(stick) < DEAD_ZONE_THRESHOLD) {
    stick = 0;
  }

  // make it range from 0 to 250 or so
  uint8_t ret = stick/CONTROLLER_RATIO + MIDDLE_POINT;

  return ret;
}

void set_motor(uint8_t left, uint8_t right) {
  Serial.print(F("L: "));
  Serial.print(left);
  Serial.print(F("\t\tR: "));
  Serial.print(right);
  Serial.println();
  analogWrite(2, left);
  analogWrite(3, right);
  delay(50);
}


