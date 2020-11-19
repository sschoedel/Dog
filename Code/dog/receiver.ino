void receiveFromController(bool receiveJS, bool receiveIMU, int &JSy, int &JSx, bool &buttonPress, int buttonTapped, int &initialRoll, int &initialPitch, int &initialYaw, int &roll, int &pitch, int &yaw, unsigned int &receiverWatchDog)
{
  if(!firstButtonTapped && buttonTapped)
  {
    firstButtonTapped = true;
    initialRoll = roll;
    initialPitch = pitch;
    initialYaw = yaw;
  }
  
  if (receiveJS && !receiveIMU)
  {
    if (radio.available())
    {
      // Data packet: JSy (4 chars) JSx (4 chars) buttonPressed (1 char)
      char textReceive[9] = "";
      radio.read(&textReceive, sizeof(textReceive));
      
      if (debugging)
      { Serial.print("Received: "); Serial.println(textReceive); }
      
      // Turn inputs into joystick values
      int JSy_digit1 = textReceive[0] - '0';
      int JSy_digit2 = textReceive[1] - '0';
      int JSy_digit3 = textReceive[2] - '0';
      int JSy_digit4 = textReceive[3] - '0';
      JSy = JSy_digit1 * 1000 + JSy_digit2 * 100 + JSy_digit3 * 10 + JSy_digit4;
      
      int JSx_digit1 = textReceive[4] - '0';
      int JSx_digit2 = textReceive[5] - '0';
      int JSx_digit3 = textReceive[6] - '0';
      int JSx_digit4 = textReceive[7] - '0';
      JSx = JSx_digit1 * 1000 + JSx_digit2 * 100 + JSx_digit3 * 10 + JSx_digit4;
  
      buttonPress = textReceive[8] - '0';
      
      checkJoystickControl();

      // Reset receiver watchdog
      receiverWatchDog = millis();
    }
  }
  else if (receiveJS && receiveIMU)
  {
        // Read incoming wifi data
    if (radio.available())
    {
      // Data packet: JSy (4 chars) JSx (4 chars) buttonPressed (1 char)
      char textReceive[10] = "";
      radio.read(&textReceive, sizeof(textReceive));
      receivingByte = textReceive[9];
      
      if (debugging)
      { Serial.print("Received: "); Serial.println(textReceive); }
      
      if (receivingByte == 1)
      {
        // Turn inputs into joystick values
        int JSy_digit1 = textReceive[0] - '0';
        int JSy_digit2 = textReceive[1] - '0';
        int JSy_digit3 = textReceive[2] - '0';
        int JSy_digit4 = textReceive[3] - '0';
        JSy = JSy_digit1 * 1000 + JSy_digit2 * 100 + JSy_digit3 * 10 + JSy_digit4;
        
        int JSx_digit1 = textReceive[4] - '0';
        int JSx_digit2 = textReceive[5] - '0';
        int JSx_digit3 = textReceive[6] - '0';
        int JSx_digit4 = textReceive[7] - '0';
        JSx = JSx_digit1 * 1000 + JSx_digit2 * 100 + JSx_digit3 * 10 + JSx_digit4;
    
        buttonPress = textReceive[8] - '0';
        
  //      Serial.print("Byte 1 received: "); Serial.println(textReceive);
  //      Serial.print("Joystick X, Joystick Y: "); Serial.print(JSx); Serial.print(", "); Serial.println(JSy);
      }
      else if (receivingByte == 2)
      {
        int yaw_digit1 = textReceive[0];
        int yaw_digit2 = textReceive[1];
        int yaw_digit3 = textReceive[2];
        yaw = yaw_digit1 * 100 + yaw_digit2 * 10 + yaw_digit3;
        yaw = map(yaw, 0, 360, -180, 180) - initialYaw;
        
        int pitch_digit1 = textReceive[3];
        int pitch_digit2 = textReceive[4];
        int pitch_digit3 = textReceive[5];
        pitch = pitch_digit1 * 100 + pitch_digit2 * 10 + pitch_digit3;
        pitch = map(pitch, 0, 360, -180, 180) - initialPitch;
        
        int roll_digit1 = textReceive[6];
        int roll_digit2 = textReceive[7];
        int roll_digit3 = textReceive[8];
        roll = roll_digit1 * 100 + roll_digit2 * 10 + roll_digit3;
        roll = map(roll, 0, 360, -180, 180) - initialRoll;
        
  //      Serial.print("Byte 2 received: "); Serial.println(textReceive);
      }
  
      if (firstButtonTapped) // Only start control when IMU rotations have been zeroed
      {
        controllerInputs();
      }
    }
  }
}
