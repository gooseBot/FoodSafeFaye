#include <Servo.h>
#include <Wire.h>
#include <SerLCD.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"
#include <SparkFun_Qwiic_Button.h>
struct appConfig
{
  int lockPeriodMinutes;
  int elapsedLockMinutes;
};
typedef struct appConfig AppConfig;

AppConfig _myConfig;
SerLCD lcd; // Initialize the library with default I2C address 0x72
QwiicButton buttonGreen;
QwiicButton buttonRed;

int lockDurations[] = {2*60, 4*60, 8*60, 24*60};
int lockChoiceIndex = 0;
int durationMinutes = lockDurations[0];

void setup()
{
  //Serial.begin(9600);
  //myDelay(5000);  //needed to allow time to open serial monitor

  Wire.begin();
  
  lcd.begin(Wire); 
  lcd.disableSplash(); //This will supress any splash from being displayed at power on
  lcd.clear(); //Clear the display - this moves the cursor to home position as well  
  lcd.setContrast(2); //Set contrast. Lower to 0 for higher contrast.

  //check if button will acknowledge over I2C
  if (buttonGreen.begin(0x6E) == false) {
    //Serial.println("Button green not acknowledge!");
  }  
  if (buttonRed.begin() == false) {
    //Serial.println("Button red not acknowledge!");
  }  
  
  EEPROM_readAnything(0, _myConfig);
  if (_myConfig.elapsedLockMinutes >= _myConfig.lockPeriodMinutes) {
    openDoor(true);
    displayCountDown(0);
  } else {
    int min2unlock = _myConfig.lockPeriodMinutes - _myConfig.elapsedLockMinutes;
    lockDoorForDuration(min2unlock);
  }
}

void loop()
{
  //if button pushed then lock the door and wait for the lock duration to end
  //check if button is pressed, and tell us if it is!

  
  if (buttonGreen.isPressed() == true) {
    while(buttonGreen.isPressed() == true)
      delay(10);  //wait for user to stop pressing
    lockChoiceIndex++; if (lockChoiceIndex>3) {lockChoiceIndex=0;}
    durationMinutes = lockDurations[lockChoiceIndex];
    displayDurationChoice(durationMinutes);
  }

  if (buttonRed.isPressed() == true) {
    while(buttonRed.isPressed() == true)
      delay(10);  //wait for user to stop pressing
    int min2unlock = calcMin2UnlockTime(durationMinutes);
    lockDoorForDuration(min2unlock);          
  }
}

void myDelay(unsigned long mseconds) {
  // this delay keeps the arduino working, built in delay stops most activity
  //   this type of delay seems to be needed to make the servo libarary work
  //   not sure why.
  unsigned long starttime = millis();   //going to count for a fixed time
  unsigned long endtime = starttime;
  while ((endtime - starttime) <= mseconds) // do the loop
  {
    endtime = millis();                  //keep the arduino awake.
  }
}

void openDoor(boolean openLock) {

  static Servo myservo;  // create servo object to control a servo
  byte _doorServo = 7;
  byte _open = 0;
  byte _close = 100;
  uint8_t brightness = 50;
  myservo.attach(_doorServo);
  if (openLock) {
    myservo.write(_open);
    lcd.setFastBacklight(0,128,0); //green
    buttonGreen.LEDon(brightness);
    buttonRed.LEDoff();
  } else {
    Serial.println("close door");
    myservo.write(_close);
    lcd.setFastBacklight(255,0,0); //red
    buttonRed.LEDon(brightness);
    buttonGreen.LEDoff();
  }
  myDelay(300);
  myservo.detach();
}

void lockDoorForDuration(int numMinutes) {
  Serial.println("lockDoorForDuration");
  _myConfig.lockPeriodMinutes = numMinutes;
  EEPROM_writeAnything(0, _myConfig);
  openDoor(false);   //lock door
  for (int i = 0; i < numMinutes; ++i) {
    Serial.println(i);
    _myConfig.elapsedLockMinutes = i;
    EEPROM_writeAnything(0, _myConfig);
    displayCountDown(numMinutes - i);
    //myDelay(1000*60);
    myDelay(60000);
  }
  displayCountDown(0);
  openDoor(true);    //open door
}

int calcMin2UnlockTime(int numMinutes) {
  _myConfig.lockPeriodMinutes = numMinutes;
  _myConfig.elapsedLockMinutes = 0;
  return _myConfig.lockPeriodMinutes - _myConfig.elapsedLockMinutes;
}

void displayCountDown(int minutesLeft) {
  int hours, minutes;
  hours = minutesLeft / 60;
  minutes = minutesLeft % 60;  
  lcd.clear();
  lcd.print("LOCKED!");
  lcd.setCursor(0, 1);
  
  if (minutesLeft > 1 & minutesLeft < 60) {
    lcd.print(minutesLeft);
    lcd.print(" more minutes");
  } else if (minutesLeft >= 60) {
    lcd.print(hours);
    lcd.print(":");
    lcd.print(minutes);
    lcd.print(" remaining");
  } else if (minutesLeft == 1) {
    lcd.print(" more minute!");
  } else {
    displayDurationChoice(durationMinutes);
  }
}

void displayDurationChoice(int minutes) {
    lcd.clear();
    lcd.print("OPEN!");
    lcd.setCursor(0, 1);
    lcd.print("Lockout = ");
    lcd.print(durationMinutes/60);
    lcd.print(" hrs");
}
