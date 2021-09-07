#include <SPI.h>
#include <Wire.h>
uint8_t newNumber = 10;
uint8_t oldNumber = 8;
uint8_t segmentOrder[11] = {0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011, 0b01011011, 0b01011111, 0b01110000, 0b01111111, 0b01111011, 0b00000001}; //
const uint32_t stepOrder[9] = {0b00000000,0b00000001, 0b00000011, 0b00000010, 0b00000110, 0b00000100,0b00001100,0b00001000,0b00001001}; //binary order for stepper movement
int8_t currentStep[7] = {1, 1, 1, 1, 1, 1, 1};
uint32_t newBitOut = 0b10000000000000000000000000000000;
uint32_t stepsPerMove = 1100; /// set correctly 4096/rev
uint32_t stepRate = 1;
uint32_t mask;
const int slaveSelectPin = 12;
uint32_t timer;


byte Minutes       = 0;
byte minuteDigit   = 0;
byte minuteTens    = 0;
byte Hours         = 0;
byte hourDigit     = 0;
byte hourTens      = 0;

// slave select 10 connected to RCLCK - strobe STR
// SCK 13 to SRCLCK CP
// MOSI 11 to SER in D
// 5V to OE

#include "RTClib.h"

RTC_DS3231 rtc;
#define CLOCK_INTERRUPT_PIN 2


void setup() {
  pinMode(slaveSelectPin, OUTPUT);
  SPI.beginTransaction(SPISettings(500000, LSBFIRST, SPI_MODE0)); 
  SPI.begin();
  Serial.begin(9600);
 
  moveStepper( -1 , segmentOrder[oldNumber] ); //Reset the clock to open position
  oldNumber = newNumber;
  moveStepper( 1, segmentOrder[newNumber]);
  
  
    if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

   /* if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(0,0,0, 14, 30,00));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }*/     

   rtc.adjust(DateTime(0,0,0,15, 16,00)); // sets the time on reset 

    //we don't need the 32K Pin, so disable it
    rtc.disable32K();
    
    // Making it so, that the alarm will trigger an interrupt
    pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onAlarm, FALLING);
    
    // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
    // if not done, this easily leads to problems, as both register aren't reset on reboot/recompile
    rtc.clearAlarm(1);
    rtc.clearAlarm(2);
    
    // stop oscillating signals at SQW Pin
    // otherwise setAlarm1 will fail
    rtc.writeSqwPinMode(DS3231_OFF);
    
    // turn off alarm 2 (in case it isn't off already)
    // again, this isn't done at reboot, so a previously set alarm could easily go overlooked
    rtc.disableAlarm(2);
    
    // schedule an alarm at the next minute seconds in the future
    rtc.setAlarm1(DateTime(0,0,0,0,0,00),DS3231_A1_Second);
  
}

void loop() {

  if(rtc.alarmFired(1)){

    DateTime now = rtc.now();
    Serial.println(now.minute(),DEC);
    rtc.clearAlarm(1);
    rtc.setAlarm1(DateTime(0,0,0,0,0,00),  DS3231_A1_Second );

     Minutes      = now.minute();
  minuteTens  = Minutes / 10;
  minuteDigit = Minutes - minuteTens * 10;

  Hours       = now.hour();
  hourTens    = Hours / 10;
  hourDigit   = Hours - hourTens * 10;

  newNumber = minuteDigit;

  Wire.beginTransmission(3); // transmit to device #3
  Wire.write(minuteTens);              // sends one byte
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(2); // transmit to device #2
  Wire.write(hourDigit);              // sends one byte
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(1); // transmit to device #1
  Wire.write(hourTens);              // sends one byte
  Wire.endTransmission();    // stop transmitting

    Serial.println(newNumber);
  Serial.println(segmentOrder[newNumber],BIN ); 
  moveStepper( -1 , segmentOrder[oldNumber]);  // move segments to new position  
  oldNumber = newNumber;
  moveStepper( 1 , segmentOrder[newNumber] ); 
 
    // move all segments to open position
    
    
  }
   

    

  
}



void moveStepper(int dir, int segments) {

  int stepsToGo = stepsPerMove;
  while (stepsToGo > 0 ) {
    int segment = 0;
    while ( segment < 8 ) {
    //Serial.println("step");
    
      if (bitRead(segments, segment) == 1) {
        currentStep[segment] = currentStep[segment] + dir; //increment in desired direction
        if (currentStep[segment] == 9)  {
          currentStep[segment] = 1; //rollover
        }
        if (currentStep[segment] == 0) {
          currentStep[segment] = 8; //rollover
        }
        
        uint32_t mask = 0b1111ul <<  4*segment;        
        newBitOut = ~mask & newBitOut; //overwrite the 4 bit piece we want to rewrite
        newBitOut = newBitOut | stepOrder[currentStep[segment]] << 4 * segment; 
        //Serial.println(newBitOut,BIN);
      }
      
      segment++;
      
    }

        digitalWrite (slaveSelectPin,  HIGH);
        SPI.transfer(newBitOut);
        SPI.transfer(newBitOut>>8);  
        SPI.transfer(newBitOut>>16); 
        SPI.transfer(newBitOut>>24);     
        digitalWrite (slaveSelectPin, LOW);

    stepsToGo = stepsToGo - 1;
    while (millis() <= timer + stepRate){
      ;
    }
    timer = millis();
  }

}

void onAlarm() { //minuteAlarm
    Serial.println("Alarm occured!");
    
}
