#include <Servo.h>

Servo servo;



void setup() {
  Serial.begin(9600);
  servo.attach(3);
}
//const int servoPins[12] = {6, 7, 3, 5, 2, 4,    17, 16, 15, 20, 14, 21}; // Servo pins for v2

void loop() {
  if(Serial.available() > 0)
  {
    int ang = Serial.readStringUntil('\n').toInt();
    servo.write(ang);
    Serial.print("Current pwm: ");Serial.println(ang);
  }
}
