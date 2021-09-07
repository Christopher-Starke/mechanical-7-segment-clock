#include <SPI.h>
#include <Wire.h>
uint8_t newNumber = 0;
uint8_t oldNumber = 8;
uint8_t segmentOrder[11] = {0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011, 0b01011011, 0b01011111, 0b01110000, 0b01111111, 0b01110011, 0b00000000}; //order of the segments for numbers 0-9
const uint32_t stepOrder[9] = {0b00000000,0b00000001, 0b00000011, 0b00000010, 0b00000110, 0b00000100,0b00001100,0b00001000,0b00001001}; //binary order for stepper movement
int8_t currentStep[7] = {1, 1, 1, 1, 1, 1, 1};
uint32_t newBitOut = 0b10000000000000000000000000000000;
uint32_t stepsPerMove = 1100; /// set correctly 4096/rev
uint32_t stepRate = 1;
uint32_t mask;
const int slaveSelectPin = 12;
uint32_t timer;
 int i = 0;

// slave select 10 connected to RCLCK - strobe STR
// SCK 13 to SRCLCK CP
// MOSI 11 to SER in D
// 5V to OE


void setup() {
  pinMode(slaveSelectPin, OUTPUT);
  SPI.beginTransaction(SPISettings(500000, LSBFIRST, SPI_MODE0)); 
  SPI.begin();
  Serial.begin(9600);
  moveStepper( -1 , segmentOrder[oldNumber] );
  delay(1000);
}

void loop() {
  if (newNumber==10){
    newNumber = 0;
  } 

    Serial.println(newNumber);
     Serial.println(segmentOrder[newNumber],BIN );
    // move all segments to open position
    delay(500);
    moveStepper( 1 , segmentOrder[newNumber]);  // move segments to new position
    delay(500);    
    oldNumber = newNumber;
    moveStepper( -1 , segmentOrder[newNumber] ); 

   newNumber ++;
}

void receiveEvent(int howMany)
{
  newNumber = Wire.read();    // receive byte as an integer
  //Serial.println(newNumber);         // print the integer
  if (newNumber != oldNumber) {
    moveStepper( -1 , segmentOrder[oldNumber] ); // move all segments to open position
    moveStepper( 1 , segmentOrder[newNumber]);  // move segments to new position
    oldNumber = newNumber;
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
        newBitOut = ~mask & newBitOut;   //overwrite the 4 bit piece we want to rewrite
        newBitOut = newBitOut | stepOrder[currentStep[segment]] << 4 * segment; 
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
