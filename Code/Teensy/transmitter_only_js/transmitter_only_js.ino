#include <RF24.h>

RF24 radio(5,6);

//const byte address[6] = "000001"; // Address for v1
const byte address[6] = "000002"; // Address for v2

float sendTimer = 25;
float prevTime = millis();

const int horizontalJS = A0;
const int verticalJS = A1;
const int button = 7;

int xJS;
int yJS;
bool buttonPress;

bool debugging = true;

void setup() {
  Serial.begin(115200);
  radio.begin();

  pinMode(button, INPUT_PULLUP);
}

void loop() {
  yJS = 1024 - analogRead(verticalJS);
  xJS = 1024 - analogRead(horizontalJS);
  buttonPress = digitalRead(button);

  // Read incoming data from teensy if available
  if (radio.available())
  {
    char textReceive[32] = "";
    radio.read(&textReceive, sizeof(textReceive));
    Serial.print("Received: "); Serial.println(textReceive);
  }

  // Send something every sendTimer milliseconds
  if (millis() > prevTime + sendTimer)
  {
    prevTime = millis();
    
    // Send something to teensy
    radio.stopListening();
    radio.openWritingPipe(address);

    char textSend[10] = "";
    sprintf(textSend, "%04i%04i%01i", yJS, xJS, buttonPress);
    radio.write(&textSend, sizeof(textSend));
    Serial.print("Wrote: "); Serial.println(textSend);
  
    radio.openReadingPipe(1, address);
    radio.startListening();
  }
}

  
