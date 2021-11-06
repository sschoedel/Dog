void kinematics(float x, float y, float z, int legSide, int legFrontBack) // front left: 1,1   front right: 2,1    back left: 1,2    back right: 2,2
{
  if (legSide == 1) // left
  {
    float x_L = x + hipOffsetLen;
    
    // Triangle 1
    float theta1_L;
//    if (legFrontBack == 2)
//      theta1_L = atan2(x_L, z) * 180 / M_PI;
//    else
      theta1_L = atan2(x_L, z) * 180 / M_PI - hipRotationExtraOffset;
      
    float z2_R = sqrt(sq(z) + sq(x_L) - sq(hipOffsetLen));
  
    // Triangle 2
    float beta2_L = atan2(y, z2_R) * 180 / M_PI; // angle due to y displacement
    float z1_L = sqrt(sq(z2_R) + sq(y)); // shared hypotenuse for trianges 2 and 3
    
    // Triangle 3
    float theta3_L = acos((sq(thighLen) + sq(shinLen) - sq(z1_L)) / (2 * thighLen * shinLen)) * 180 / M_PI; // angles due to z displacement
    float theta2_L = (180 - theta3_L) / 2;

    if (legFrontBack == 1)      // left (1) front (1)
    {
      motorRotations[0] = theta1_L + offsets[0];
      motorRotations[1] = theta2_L + beta2_L + offsets[1];
      motorRotations[2] = theta3_L +         offsets[2] - 90;
    }
    else if (legFrontBack == 2) // left (1) back (2)
    {
      motorRotations[6] = theta1_L + 180 - offsets[6]; 
      motorRotations[7] = theta2_L + beta2_L + offsets[7];
      motorRotations[8] = theta3_L +         offsets[8] - 90;     
    }
  }
  else if (legSide == 2) // right
  {
    float x_R = x - hipOffsetLen;

    y = -y; // v2 (comment for v1)
    
    // Triangle 1
    float theta1_R;
//    if (legFrontBack == 1)
//      theta1_R = -atan2(x_R, z) * 180 / M_PI;
//    else
      theta1_R = -atan2(x_R, z) * 180 / M_PI - hipRotationExtraOffset;
    float z2_R = sqrt(sq(z) + sq(x_R) - sq(hipOffsetLen));
    
    // Triangle 2
    float beta2_R = atan2(y, z2_R) * 180 / M_PI; // angle due to y displacement
    float z1_R = sqrt(sq(z2_R) + sq(y)); // shared hypotenuse for trianges 2 and 3
  
    // Triangle 3
    float theta3_R = acos((sq(thighLen) + sq(shinLen) - sq(z1_R)) / (2 * thighLen * shinLen)) * 180 / M_PI; // angles due to z displacement
    float theta2_R = (180 - theta3_R) / 2;
    
    if (legFrontBack == 1)      // right (2) front (1)
    {
      motorRotations[3] = theta1_R + 180 - offsets[3];
//      motorRotations[4] = theta2_R + beta2_R + offsets[4]; // v1
      motorRotations[4] = -theta2_R + beta2_R + offsets[4]; // v2
      motorRotations[5] = theta3_R +         offsets[5] - 90;
    }
    else if (legFrontBack == 2) // right (2) back (2)
    {
      motorRotations[9] = theta1_R + offsets[9];    
//      motorRotations[10] = theta2_R + beta2_R + offsets[10]; // v1
      motorRotations[10] = -theta2_R + beta2_R + offsets[10]; // v2
      motorRotations[11] = theta3_R +         offsets[11] - 90;
    }
  }
}



// Sets motor angles to achieve desired x,y,z,p,r,y offsets
void wholeDogKinematics(float x, float y, float z, float pitch, float roll, float yaw)
{  
  // Contribution to z and y due to pitch
  float z_pitch = dogLength/2 * sin(pitch*M_PI/180);

  // Contribution to z and x due to roll
  float z_roll = dogWidth/2 * tan(roll * M_PI/180);

  // Nominal yaw distance and theta
  float d = 134.78;
  float theta_n = asin((dogWidth/2 + hipOffsetLen)/d) * 180/M_PI;
  
  // Contribution to x and y due to yaw
  float x_yaw = d*(sin(theta_n * M_PI/180 + yaw * M_PI/180) - sin(theta_n * M_PI/180));
  float y_yaw = d*(cos(theta_n * M_PI/180) - cos(theta_n * M_PI/180 + yaw * M_PI/180));

  // Modify the height of each leg's 1st triangle
//  float z_FL = z + z_pitch + z_roll; // v1
//  float z_BL = z - z_pitch + z_roll; // v1
  float z_FL = z - z_pitch + z_roll; // v2
  float z_BL = z + z_pitch + z_roll; // v2
//  float z_FR = z + z_pitch - z_roll; // v1
//  float z_BR = z - z_pitch - z_roll; // v1
  float z_FR = z - z_pitch - z_roll; // v2
  float z_BR = z + z_pitch - z_roll; // v2
  
  // Nominal X value to set feet in vertical position at zero robot x displacement (hipOffsetLen) and x displacement for desired yaw (x_yaw)
  float x_FL = x + hipOffsetLen + x_yaw;
  float x_BL = x + hipOffsetLen - x_yaw;
  float x_FR = x - hipOffsetLen + x_yaw;
  float x_BR = x - hipOffsetLen - x_yaw;

  // Modify Y values for each leg based on y displacement for desired yaw (y_yaw)
  float y_FL = y + y_yaw;
  float y_BL = y + y_yaw;
//  float y_FR = y - y_yaw; // v1
//  float y_BR = y - y_yaw; // v1
  float y_FR = -(y - y_yaw); // v2
  float y_BR = -(y - y_yaw); // v2

  Serial.print("x_yaw, y_yaw: "); Serial.print(x_yaw); Serial.print(" "), Serial.println(y_yaw);
 

  // Triangle 1
  // Front left leg
//  float theta1_FL = atan2(x_FL, z_FL) * 180 / M_PI - hipRotationExtraOffset;
  float theta1_FL = atan2(x_FL, z_FL) * 180 / M_PI;
  float z2_FL = sqrt(sq(z_FL) + sq(x_FL) - sq(hipOffsetLen));
  // Back left leg
//  float theta1_BL = atan2(x_BL, z_BL) * 180 / M_PI - hipRotationExtraOffset; // v1
  float theta1_BL = atan2(x_BL, z_BL) * 180 / M_PI; // v2
  float z2_BL = sqrt(sq(z_BL) + sq(x_BL) - sq(hipOffsetLen));
  // Front right leg
//  float theta1_FR = -atan2(x_FR, z_FR) * 180 / M_PI - hipRotationExtraOffset; // v1
  float theta1_FR = -atan2(x_FR, z_FR) * 180 / M_PI; // v2
  float z2_FR = sqrt(sq(z_FR) + sq(x_FR) - sq(hipOffsetLen));
  // Back right leg
//  float theta1_BR = -atan2(x_BR, z_BR) * 180 / M_PI - hipRotationExtraOffset;
  float theta1_BR = -atan2(x_BR, z_BR) * 180 / M_PI;
  float z2_BR = sqrt(sq(z_BR) + sq(x_BR) - sq(hipOffsetLen));
  

  // Triangle 2
  // Front left leg
  float beta2_FL = atan2(y_FL, z2_FL) * 180 / M_PI; // angle due to y displacement
  float z1_FL = sqrt(sq(z2_FL) + sq(y)); // shared hypotenuse for triangles 2 and 3
  // Back left leg
  float beta2_BL = atan2(y_BL, z2_BL) * 180 / M_PI; // angle due to y displacement
  float z1_BL = sqrt(sq(z2_BL) + sq(y)); // shared hypotenuse for triangles 2 and 3
  // Front right leg
  float beta2_FR = atan2(y_FR, z2_FR) * 180 / M_PI; // angle due to y displacement
  float z1_FR = sqrt(sq(z2_FR) + sq(y)); // shared hypotenuse for triangles 2 and 3
  // Back right leg
  float beta2_BR = atan2(y_BR, z2_BR) * 180 / M_PI; // angle due to y displacement
  float z1_BR = sqrt(sq(z2_BR) + sq(y)); // shared hypotenuse for triangles 2 and 3

  
  // Triangle 3
  // Front left leg
  float theta3_FL = acos((sq(thighLen) + sq(shinLen) - sq(z1_FL)) / (2 * thighLen * shinLen)) * 180 / M_PI; // angles due to z displacement
  float theta2_FL = (180 - theta3_FL) / 2;
  // Back left leg
  float theta3_BL = acos((sq(thighLen) + sq(shinLen) - sq(z1_BL)) / (2 * thighLen * shinLen)) * 180 / M_PI; // angles due to z displacement
  float theta2_BL = (180 - theta3_BL) / 2;
  // Front right leg
  float theta3_FR = acos((sq(thighLen) + sq(shinLen) - sq(z1_FR)) / (2 * thighLen * shinLen)) * 180 / M_PI; // angles due to z displacement
  float theta2_FR = (180 - theta3_FR) / 2;
  // Back right leg
  float theta3_BR = acos((sq(thighLen) + sq(shinLen) - sq(z1_BR)) / (2 * thighLen * shinLen)) * 180 / M_PI; // angles due to z displacement
  float theta2_BR = (180 - theta3_BR) / 2;
  
  

  motorRotations[0] = theta1_FL + offsets[0] - roll;
//  motorRotations[3] = theta1_FR + 180 - offsets[3] + roll;
//  motorRotations[6] = theta1_BL + 180 - offsets[6] - roll;
  motorRotations[3] = theta1_FR - offsets[3] + roll;
  motorRotations[6] = theta1_BL - offsets[6] - roll;
  motorRotations[9] = theta1_BR + offsets[9] + roll;

  // front left leg
//  motorRotations[1] = theta2_FL + beta2_FL + offsets[1] + pitch; // v1
  motorRotations[1] = theta2_FL + beta2_FL + offsets[1] - pitch; // v2
  motorRotations[2] = theta3_FL +         offsets[2] - 90;

  // front right leg
//  motorRotations[4] = theta2_FR + beta2_FR + offsets[4] + pitch; // v1
//  motorRotations[4] = -theta2_FR + beta2_FR + offsets[4] + pitch; // v2
  motorRotations[4] = -theta2_FR + beta2_FR + offsets[4] + pitch; // v2
  motorRotations[5] = theta3_FR +         offsets[5] - 90;

  // back left leg
//  motorRotations[7] = theta2_BL + beta2_BL + offsets[7] + pitch; // v1
//  motorRotations[7] = theta2_BL + beta2_BL + offsets[7] - pitch; // v2
  motorRotations[7] = theta2_BL + beta2_BL + offsets[7] - pitch; // v2
  motorRotations[8] = theta3_BL +         offsets[8] - 90;

  // back right leg
//  motorRotations[10] = theta2_BR + beta2_BR + offsets[10] + pitch; // v1
  motorRotations[10] = -theta2_BR + beta2_BR + offsets[10] + pitch; // v2
  motorRotations[11] = theta3_BR +         offsets[11] - 90;

  if (debuggingRotations) {
    // print pwm outputs
    Serial.print("Motor rotations\n");
    for (int i = 0; i < 12; i++)
    {
      Serial.print(i); Serial.print(" : "); Serial.print(motorRotations[i]); Serial.print('\n');
    }
    Serial.print("\n"); 
  }
}
