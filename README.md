# mechanical-7-segment-clock
A large mechanical 7-segment clock using cheap stepper motors.  

![](/Images/Main_photo.jpg)
##
Each digit requires its own controller board. The seconds digit acts as the master keeping time using the DS3231 RTC and sending I2C commands out to the other digits. The rest of the digits act as slaves recieving numbers via I2C and displaying the number. The minute tens digit needs I2c adress #3 hours digit adress #2 and hours tens adress #1. 

## BOM
- 28x		28BYJ-48 5V stepper motors  
- 28x		ULN2003 darlington arrays
- 16x		7HCT 4094 8-bit shift registors 
- 28x		608 skateboard bearings
- 4x		Arduino nano 
- 4x		4 pole screw terminals 
- 1x 		Adafruit ds3231 RTC module
- 56x		M4x10 bolts (for anchoring stepper motors to the housing)
- 56x 		M4x25 botls (for attaching the stpper casing to the backboard)

### Printed parts 
- 28x		Stepper housings 
- 28x		Levers

### Additional parts
- 1x		Backplate
- 28x		Digit faces

