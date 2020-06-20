#include <Servo.h>

Servo servo;



void setup() {
  Serial.begin(9600);
  servo.attach(20);
}

void loop() {
  if(Serial.available() > 0)
  {
    int ang = Serial.readStringUntil('\n').toInt();
    servo.write(ang);
  }
}
