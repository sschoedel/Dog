
int SW = 2;

int SW_state = 0;

void setup() {
  Serial.begin(9600); 
  
  pinMode(SW, INPUT); 
  
}

void loop() {
  SW_state = digitalRead(SW);
  
  Serial.print(" | Button: ");
  Serial.println(SW_state);

  delay(100);
  
}
