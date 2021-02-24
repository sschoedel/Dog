//#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//RF24 radio(5, 6); // CE, CSN
RF24 radio(9,8); // CE, CSN
const byte address[6] = "000001";
void setup() {
  Serial.begin(115200);
  radio.begin();
//  radio.setChannel(0x55);
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}
void loop() {
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.print("RECEIVED: ");Serial.println(text);
  }
}
