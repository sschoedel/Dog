#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(5,6); // CE, CSN
const byte address[6] = "000001";
void setup() {
  Serial.begin(9600);
  radio.begin();
//  radio.setChannel(0x55);
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}
void loop() {
  const char text[] = "Hello World";
  radio.write(&text, sizeof(text));
  Serial.print("Sent: "); Serial.println(text);
  delay(1000);
}
