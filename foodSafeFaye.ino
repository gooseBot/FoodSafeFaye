#include <Servo.h> 
#include <serLCD.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"
struct appConfig
{
  int lockPeriodMinutes;
  int elapsedLockMinutes;
};
typedef struct appConfig AppConfig;
AppConfig _myConfig;
byte _lockButton=2;
  
void setup() 
{ 
  Serial.begin(9600);
  myDelay(3000);
  Serial.println("setup");
  EEPROM_readAnything(0, _myConfig);
  Serial.println(_myConfig.elapsedLockMinutes);
  Serial.println(_myConfig.lockPeriodMinutes);
  if (_myConfig.elapsedLockMinutes >= _myConfig.lockPeriodMinutes){    
    openDoor(true);
    displayCountDown(0);
  } else {        
    int min2unlock = _myConfig.lockPeriodMinutes - _myConfig.elapsedLockMinutes;
    lockDoorForDuration(min2unlock);    
  }
  pinMode(_lockButton, INPUT_PULLUP);  //pull signal for button high
} 
 
void loop() 
{ 
  //if button pushed then lock the door and wait for the lock duration to end
  int sensorVal = digitalRead(_lockButton);
  if (sensorVal == LOW) {
    int min2unlock=calcMin2UnlockTime();
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
  byte _doorServo=7;
  byte _open=0;
  byte _close=100;
  myservo.attach(_doorServo,544,2400);  
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
  openDoor(false);   //lock door
  for (byte i = 0; i < numMinutes; ++i){
    Serial.println(i); 
    _myConfig.elapsedLockMinutes = numMinutes-i;
    EEPROM_writeAnything(0, _myConfig);
    displayCountDown(numMinutes-i);
    printCurrentTime(numMinutes-i);  
    myDelay(60000);
  }
  displayCountDown(0);
  openDoor(true);    //open door
}

int calcMin2UnlockTime() {  
  _myConfig.lockPeriodMinutes = 120;
  _myConfig.elapsedLockMinutes = 0;
  EEPROM_writeAnything(0, _myConfig);
  return _myConfig.lockPeriodMinutes - _myConfig.elapsedLockMinutes;
}

void displayCountDown(int minutesLeft) {
  serLCD lcd(3);  
  lcd.clear();
  lcd.setBrightness(10);
  lcd.print(minutesLeft);
  if (minutesLeft>1) {
    lcd.print(" more minutes");
  } else if (minutesLeft==1) {
    lcd.print(" more minute!");
  } else {
    lcd.clear();
    lcd.print("OPEN!");
  }
}
