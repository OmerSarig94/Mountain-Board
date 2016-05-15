/*
 * (CC BY-NC-SA 4.0) 
 * http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * WARNING WARNING WARNING: attaching motors to a skateboard is 
 * a terribly dangerous thing to do.  This software is totally
 * for amusement and/or educational purposes.  Don't obtain or
 * make a wiiceiver (see below for instructions and parts), 
 * don't attach it to a skateboard, and CERTAINLY don't use it
 * to zip around with just a tiny, ergonomic nunchuck instead
 * of a bulky R/C controller.
 *
 * This software is made freely available.  If you wish to 
 * sell it, don't.  If you wish to modify it, DO! (and please
 * let me know).  Much of the code is derived from others out
 * there, I've made attributuions where appropriate.
 *
 * http://austindavid.com/wiiceiver
 *  
 * latest software: https://github.com/jaustindavid/wiiceiver
 * schematic & parts: http://www.digikey.com/schemeit#t9g
 *
 * Enjoy!  Be safe! 
 * 
 * (CC BY-NC-SA 4.0) Austin David, austin@austindavid.com
 * 12 May 2014
 *
 */
 
#include <Arduino.h>

#include <avr/wdt.h> 
#include <Wire.h>
#include <Servo.h>
#include <EEPROM.h>

#define WIICEIVER_VERSION "2.0 beta4"

// define ALLOW_HELI_MODE to unlock the settable mode & functions
// #define ALLOW_HELI_MODE
#undef ALLOW_HELI_MODE
// leaving it undefined removes all the CODE at compile-time

// addys for vars stored in EEPROM
#define EEPROM_Y_ADDY 0
#define EEPROM_AUTOCRUISE_ADDY 1
#define EEPROM_WDC_ADDY 2
#define EEPROM_DRAGBRAKE_ADDY 3
#define EEPROM_MINTHROTTLE_ADDY 4          // no longer in use
#define EEPROM_MAXTHROTTLE_ADDY 5
#define EEPROM_ACCELPROFILE_ADDY 6
#define EEPROM_HELI_MODE_ADDY 7

#define DEBUGGING

#include "watchdog.h"

// #define DEBUGGING_PINS

#include "utils.h"


#include "Blinker.h"

// #define DEBUGGING_CHUCK
// #define DEBUGGING_CHUCK_ACTIVITY
#define WII_ACTIVITY_COUNTER 100  // once per 20ms; 50 per second
#include "Chuck.h"

// #define DEBUGGING_ESC
// to infrequently insert a small change in the servo pulse, to keep an ESC interested
// disabled by default
// #define ESC_JITTER
#include "ElectronicSpeedController.h"

// #define DEBUGGING_SMOOVER
#include "Smoover.h"

#define THROTTLE_MIN 0.05                      // the lowest throttle to send the ESC

// #define DEBUGGING_CRUISER
#include "Cruiser.h"

#define DEBUGGING_THROTTLE
#include "Throttle.h"


// global objects

Chuck chuck;
ElectronicSpeedController ESC;
Blinker green, red;
Throttle throttle;

#define DEBUGGING_TUNA
#include "Tuna.h" // needs access to the global objects 



// an unambiguous startup display
void splashScreen() {
  int i;
  digitalWrite(pinLocation(GREEN_LED_ID), HIGH);
  digitalWrite(pinLocation(RED_LED_ID), HIGH);
  delay(250);
  for (i = 0; i < 5; i++) {
    digitalWrite(pinLocation(GREEN_LED_ID), HIGH);
    digitalWrite(pinLocation(RED_LED_ID), LOW);
    delay(50);
    digitalWrite(pinLocation(GREEN_LED_ID), LOW);
    digitalWrite(pinLocation(RED_LED_ID), HIGH);
    delay(50);
  }
  digitalWrite(pinLocation(GREEN_LED_ID), HIGH);
  digitalWrite(pinLocation(RED_LED_ID), HIGH);
  delay(250);
  digitalWrite(pinLocation(GREEN_LED_ID), LOW);    
  digitalWrite(pinLocation(RED_LED_ID), LOW);  
} // void splashScreen(void)


// flash the LEDs to indicate throttle position
void updateLEDs(float throttlePos) {
  if (abs(throttlePos) < THROTTLE_MIN) {
    green.update(1);
    red.update(1);
  } else {
    int bps = constrain(abs(int(throttlePos * 20)), 1, 20);
    if (throttlePos > 0) {
      green.update(bps);
      red.update(1);
    } else {
      green.update(1);
      red.update(bps);
    }
  }
} // updateLEDs(float throttle)


// the nunchuck appears to be static: we lost connection!
// go "dead" for up to 5s, but keep checking the chuck to see if
// it comes back
void freakOut(void) {
  unsigned long targetMS = millis() + 5000;
  bool redOn = false;
  byte blinkCtr = 0;

#ifdef DEBUGGING
    Serial.print(millis());
    Serial.println(": freaking out");
#endif

  red.stop();
  green.stop();
  while (!chuck.isActive() && targetMS > millis()) {
//  while (targetMS > millis()) {
    if (blinkCtr >= 4) {
      blinkCtr = 0;
      if (redOn) {
        red.high();
        green.low();
        redOn = false;
      } 
      else {
        red.low();
        green.high();
        redOn = true;
      }
    }
    blinkCtr ++;
    chuck.update();
    delay(20);
    wdt_reset();
  }
  green.start(1);
  red.start(1);
} // void freakOut(void)



void setup_pins() {
  /*
  pinMode(ESC_GROUND, OUTPUT);
  digitalWrite(ESC_GROUND, LOW);
  pinMode(WII_GROUND, OUTPUT);
  digitalWrite(WII_GROUND, LOW);
  */
  pinMode(pinLocation(WII_POWER_ID), OUTPUT);
  digitalWrite(pinLocation(WII_POWER_ID), HIGH);
  
  pinMode(pinLocation(WII_SCL_ID), INPUT_PULLUP);
  pinMode(pinLocation(WII_SDA_ID), INPUT_PULLUP);

} // setup_pins()


// wait up to 1s for something to happen
bool waitForActivity(void) {
  unsigned long timer = millis() + 1000;
#ifdef DEBUGGING
    Serial.print(millis());
    Serial.print(F(" Waiting for activity ... "));
#endif
  
  chuck.update();
  while (! chuck.isActive() && timer > millis()) {
    wdt_reset();
    delay(20);
    chuck.update();
  }
#ifdef DEBUGGING
    Serial.print(millis());
    Serial.println(chuck.isActive() ? F(": active!") : F(": not active :("));
#endif
  
  return chuck.isActive();
} // bool waitForActivity()


// returns true if the chuck appears "active"
// will retry 5 times, waiting 1s each
bool startChuck() {
  int tries = 0;
  
  while (tries < 10) {
#ifdef DEBUGGING
    Serial.print(F("(Re)starting the nunchuck: #"));
    Serial.println(tries);
#endif
    wdt_reset();
    chuck.setup();
    chuck.readEEPROM();
    tries ++;
    if (waitForActivity()) {
      return true;
    }
  }
  return false;
} // startChuck()


// pretty much what it sounds like
void handleInactivity() {
  watchdog_setup(WDTO_8S);
#ifdef DEBUGGING
  Serial.print(millis());
  Serial.println(F(": handling inactivity"));
#endif
  // lastThrottle = 0; // kills cruise control
  // smoother.zero();  // kills throttle history
  throttle.zero();
  ESC.setLevel(0);
  
  // this loop: try to restart 5 times in 5s; repeat until active
  do {    
    freakOut();
    if (! chuck.isActive()) {
      // stopChuck();
      // delay(250);
      startChuck();
    }
  } while (! chuck.isActive());
  
  // active -- now wait for zero
#ifdef DEBUGGING
  Serial.print(millis());
  Serial.println(F("Waiting for 0"));
#endif  
  while (chuck.Y > 0.1 || chuck.Y < -0.1) {
    chuck.update();
    wdt_reset();
    delay(20);
  }
  
#ifdef DEBUGGING
  Serial.print(millis());
  Serial.println(F(": finished inactivity -- chuck is active"));
#endif
  watchdog_setup(WDTO_250MS);
} // handleInactivity()




void setup() {
  wdt_disable();
  Serial.begin(115200);

  Serial.print(F("Wiiceiver v "));
  Serial.print(F(WIICEIVER_VERSION));
  Serial.print(F(" (compiled "));
  Serial.print(F(__DATE__));
  Serial.print(F(" "));
  Serial.print(F(__TIME__));
  Serial.println(F(")"));
  display_WDC();  
  readSettings();
  
  green.init(pinLocation(GREEN_LED_ID));
  red.init(pinLocation(RED_LED_ID));

  setup_pins();
  ESC.init(pinLocation(ESC_PPM_ID), pinLocation(ESC2_PPM_ID));
  
  splashScreen();
  showTunaSettings();

#ifdef DEBUGGING
  Serial.println(F("Starting the nunchuck ..."));
#endif
  green.high();
  red.high();
  if (! startChuck()) {
 /*   maybeCalibrate();
  } else {*/
    handleInactivity();
  }
#ifdef DEBUGGING
  Serial.println(F("Nunchuck is active!"));
#endif

  green.start(10);
  red.start(10);
  
  green.update(1);
  red.update(1);
  watchdog_setup(WDTO_250MS);
  throttle.init();
} // setup()



void loop() {
  static float lastThrottleValue = 0;
  unsigned long startMS = millis();
  wdt_reset();
  green.run();
  red.run();
  chuck.update();

  // check for the tuning UI
  tuna();
  
  // for forcing a watchdog timeout (testing)
  #undef SUICIDAL_Z
  #ifdef SUICIDAL_Z
    if (chuck.Z) {
      Serial.println(F("sleepin' to reset"));
      delay(9000);
    } // suicide!
  #endif 

  if (!chuck.isActive()) {
    #ifdef DEBUGGING
      Serial.println(F("INACTIVE!!"));
    #endif
    handleInactivity();
  } else {
    float throttleValue = throttle.update(chuck);
    ESC.setLevel(throttleValue);
    if (throttleValue != lastThrottleValue) {
      updateLEDs(throttle.getThrottle());
      #ifdef DEBUGGING
        Serial.print(F("y="));
        Serial.print(chuck.Y, 4);
        Serial.print(F(", "));
        Serial.print(F("c="));
        Serial.print(chuck.C);      
        Serial.print(F(", z="));
        Serial.print(chuck.Z);
        Serial.print(F(", "));
        Serial.println(throttleValue, 4); 
      #endif
      lastThrottleValue = throttleValue;
    } // if (throttleValue != lastThrottleValue)
    
    int delayMS = constrain(startMS + 21 - millis(), 5, 20);
    #ifdef DEBUGGING_INTERVALS
      Serial.print(F("sleeping ")); 
      Serial.println(delayMS);
    #endif
    delay(delayMS);
  } // if (!chuck.isActive()) - else
} // loop()

