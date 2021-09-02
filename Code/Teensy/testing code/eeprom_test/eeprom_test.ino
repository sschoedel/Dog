#include <EEPROM.h>

int calAccelAddress = 0;

bool calAccel = true;
byte calAccelEEPROM;
void setup() {
  Serial.begin(115200);
  
  calAccelEEPROM = EEPROM.read(calAccelAddress);
  Serial.println(calAccelEEPROM);
  if (calAccel || calAccelEEPROM != 1)
  {
    // Calibrate accel
    calAccelEEPROM = 3;
    EEPROM.write(calAccelAddress, calAccelEEPROM);
  }
}

void loop()
{
  
}
