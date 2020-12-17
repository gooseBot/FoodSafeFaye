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
QwiicButton button;
uint8_t brightness = 100;
                            
void setup()
{
  Serial.begin(9600);
  myDelay(8000);
  Serial.println("setup");

  Wire.begin();
  lcd.begin(Wire); //Set up the LCD for I2C communication
  //lcd.setBacklight(132, 245, 66); //green
  lcd.setBacklight(245, 66, 66); //red
  lcd.setContrast(10); //Set contrast. Lower to 0 for higher contrast.
  lcd.clear(); //Clear the display - this moves the cursor to home position as well  

  //check if button will acknowledge over I2C
  if (button.begin(0x5B) == false) {
    Serial.println("Button did not acknowledge! Freezing.");
  }  
  Serial.println("Button acknowledged.");  
  button.LEDoff();  
  
  EEPROM_readAnything(0, _myConfig);
  Serial.println(_myConfig.elapsedLockMinutes);
  Serial.println(_myConfig.lockPeriodMinutes);
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
  if (button.isPressed() == true) {
    button.LEDon(brightness);
    while(button.isPressed() == true)
      delay(10);  //wait for user to stop pressing
    button.LEDoff();
    int min2unlock = calcMin2UnlockTime();
    printCurrentTime(min2unlock);
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
  myservo.attach(_doorServo, 544, 2400);
  if (openLock) {
    Serial.println("open door");
    myservo.write(_open);
  } else {
    Serial.println("close door");
    myservo.write(_close);
  }
  myDelay(300);
  myservo.detach();
}

void printCurrentTime(int min2unlock) {
  //Serial.begin(9600);
  Serial.print(min2unlock, DEC);
  Serial.println();
  //Serial.end();
}

void lockDoorForDuration(int numMinutes) {
  Serial.println("lockDoorForDuration");
  _myConfig.lockPeriodMinutes = numMinutes;
  EEPROM_writeAnything(0, _myConfig);
  openDoor(false);   //lock door
  for (byte i = 0; i < numMinutes; ++i) {
    Serial.println(i);
    _myConfig.elapsedLockMinutes = i;
    EEPROM_writeAnything(0, _myConfig);
    displayCountDown(numMinutes - i);
    printCurrentTime(numMinutes - i);
    //myDelay(60000);
    myDelay(600);
  }
  displayCountDown(0);
  openDoor(true);    //open door
}

int calcMin2UnlockTime() {
  _myConfig.lockPeriodMinutes = 120;
  _myConfig.elapsedLockMinutes = 0;
  return _myConfig.lockPeriodMinutes - _myConfig.elapsedLockMinutes;
}

void displayCountDown(int minutesLeft) {
  //serLCD lcd(3);
  lcd.clear();
  //lcd.setBrightness(10);
  lcd.print(minutesLeft);
  if (minutesLeft > 1) {
    lcd.print(" more minutes");
  } else if (minutesLeft == 1) {
    lcd.print(" more minute!");
  } else {
    lcd.clear();
    lcd.print("OPEN!");
  }
}
