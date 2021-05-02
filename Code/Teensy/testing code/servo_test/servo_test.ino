#include <Servo.h>

Servo testServo1;
Servo testServo2;

#define MAX_MESSAGE_BUFFER 64
#define MAX_CMD_BUFFER 64

String message;
int messageIndex = 0;

String cmd[MAX_CMD_BUFFER] = {};
int cmdIndex = 0;

bool debugging = true;

void setup() {
  Serial.begin(9600);
  testServo1.attach(5);
  testServo2.attach(6);

}

void loop() {
  if(Serial.available() > 0)
  {
    readInput();
    parseCmd();
  }
}

void parseCmd()
{
//  if(cmd[0] == "1")
//  {
//    testServo1.write(cmd[1].toInt());
//  }
//  else if(cmd[0] == "2")
//  {
//    testServo2.write(cmd[1].toInt());
//  }
testServo2.write(cmd[0].toInt());
}


/*
 * readInput and parseIntoCmd are for making cmd string
 */
void readInput()
{
  // Fill message array with incoming chars
  while (Serial.available() > 0)
  {
    message = Serial.readStringUntil('\n');
  }
  Serial.println(message);
  parseIntoCmd();
}

void parseIntoCmd()
{
  cmdIndex = 0;
  int lasti = 0; // Saves most recent place after last string split
//  String messageStr(message); // Char array into string so can use substring
  int msgLngth = message.length();
  for(int i=0; i<msgLngth; i++)
  {
    if(message[i] == ',') // Delineates command word
    {
      cmd[cmdIndex] = message.substring(lasti, i);
      cmdIndex++;
      lasti = i+1;
    }
    else if (i == msgLngth - 1) // Last command (no ending comma)
    {
      if(message[msgLngth] != ',') // Ignore any stray ending commas
      {
        cmd[cmdIndex] = message.substring(lasti, i+1);
      }
    }
  }

  if (debugging)
  {
    Serial.print("Received command: ");
    for (int i=0; i<=cmdIndex; i++)
    {
      Serial.print(cmd[i]);
      Serial.print(" ");
    }
    Serial.print('\n');
  }
}

//// Clear char messagea and string cmd arrays
//void resetMessage(){
//  memset(message, 0, sizeof(message));
//}

void resetCmd(){
  for(int i=0; i<cmdIndex; i++)
  {
    cmd[i] = "";
  }
}
