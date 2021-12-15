#include "MPU9250.h"
#include "SensorFusion.h"
#include <EEPROM.h>

MPU9250 IMU(Wire2,0x71);

SF fusion;

// Accel and mag calibration values
float accelBias[3];
float accelScaleFactor[3];
float magBias[3];
float magScaleFactor[3];


// IMU output
float IMUaccel[3];
float IMUgyro[3];
float IMUmag[3];
float IMUtemp;

// Sensor fusion
float pitch, roll, yaw;
float deltat;

// EEPROM variables
bool calibrateAccel = false; // Change these to true to force IMU calibration
bool calibrateMag = false;
byte calAccelAddress = 0;
byte calMagAddress = 1;
byte calAccelEEPROM;
byte calMagEEPROM;

void setup() {
  Serial.begin(115200);

  // Retrieve calibration statuses from EEPROM
  calAccelEEPROM = EEPROM.read(calAccelAddress);
  calMagEEPROM = EEPROM.read(calMagAddress);
  
  int status;
  
  status = IMU.begin();
  
  if (status > 0)
  { Serial.println("IMU begin success."); }
  else
  { Serial.println("IMU begin failure."); }
  
  status = IMU.setAccelRange(MPU9250::ACCEL_RANGE_8G);
  
  if (status > 0)
  { Serial.println("IMU accel range success."); }
  else
  { Serial.println("IMU accel range failure."); }

  status = IMU.setGyroRange(MPU9250::GYRO_RANGE_1000DPS);
  
  if (status > 0)
  { Serial.println("IMU gyro range success."); }
  else
  { Serial.println("IMU gyro range failure."); }
  
  status = IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_92HZ);
  
  if (status > 0)
  { Serial.println("IMU DLPF success."); }
  else
  { Serial.println("IMU DLPF failure."); }

  status = IMU.setSrd(3); // 1000/ (1 + 3) = 250 Hz (must be larger than 2 * DLPF bandwidth and smaller than data read rate)
  
  if (status > 0)
  { Serial.println("IMU sample rate divider success."); }
  else
  { Serial.println("IMU sample rate divider failure."); }

  status = IMU.disableDataReadyInterrupt();
  
  if (status > 0)
  { Serial.println("IMU disable data ready interrupt success."); }
  else
  { Serial.println("IMU disable data ready interrupt failure."); }

  // TODO: recalibrate gyro if not stable when it should be (this would occur when dog is started while in motion)
  // Dog should lay down here to keep IMU still
  status = IMU.calibrateGyro();
  
  if (status > 0)
  { 
    Serial.println("IMU gyro recalibration success."); 
    Serial.print("\tGyro bias X, Y, Z (degrees): ");
    Serial.print(IMU.getGyroBiasX_rads() * 180/M_PI);Serial.print(", ");
    Serial.print(IMU.getGyroBiasY_rads() * 180/M_PI);Serial.print(", ");
    Serial.println(IMU.getGyroBiasZ_rads() * 180/M_PI);
  }
  else
  { Serial.println("IMU gyro recalibration failure."); }

  
  // Calibrate accelerometer if it has not already been done or the user wishes it to be calibrated again
  if (calAccelEEPROM != 1 || calibrateAccel)
  { 
    // Calibration routine
    Serial.println("Calibrating accelerometer.");
    Serial.println("You will place the IMU in six different orientations. Three seconds are allotted for movement per orientation.");
    delay(6000);
    for (int i=1; i<7; i++)
    {
      Serial.print("Place the IMU in orientation ");Serial.print(i);Serial.println('.');
      Serial.println("...");
      delay(1000);
      Serial.println("...");
      delay(1000);
      Serial.println("...");
      delay(1000);
      Serial.print("--- Calibrating orientation ");Serial.print(i);Serial.println("---");
      status = IMU.calibrateAccel();
      if (status > 0)
      {
        if (i < 6)
        {
          Serial.println("Done.");
        }
      }
      else
      {
        Serial.print("Calibration for side ");Serial.print(i);Serial.println(" failed.");
      }
    }
    Serial.println("\nDone calibrating.\n");
    
    // Collect biases and scale factors
    accelBias[0] = IMU.getAccelBiasX_mss();
    accelBias[1] = IMU.getAccelBiasY_mss();
    accelBias[2] = IMU.getAccelBiasZ_mss();
    accelScaleFactor[0] = IMU.getAccelScaleFactorX();
    accelScaleFactor[1] = IMU.getAccelScaleFactorY();
    accelScaleFactor[2] = IMU.getAccelScaleFactorZ();

    // Write acc biases and scale factors to EEPROM
    IMU.setAccelCalX(accelBias[0], accelScaleFactor[0]);
    IMU.setAccelCalY(accelBias[1], accelScaleFactor[1]);
    IMU.setAccelCalZ(accelBias[2], accelScaleFactor[2]);

    // Set calibration status to 1 in EEPROM (to indicate previous calibration) 
    calAccelEEPROM = 1;
    EEPROM.write(calAccelAddress, calAccelEEPROM);
  }
  else
  {
    accelBias[0] = IMU.getAccelBiasX_mss_EEPROM();
    accelBias[1] = IMU.getAccelBiasY_mss_EEPROM();
    accelBias[2] = IMU.getAccelBiasZ_mss_EEPROM();
    accelScaleFactor[0] = IMU.getAccelScaleFactorX_EEPROM();
    accelScaleFactor[1] = IMU.getAccelScaleFactorY_EEPROM();
    accelScaleFactor[2] = IMU.getAccelScaleFactorZ_EEPROM();
    
    Serial.println("printing acc bias xyz: ");
    Serial.println(accelBias[0]);
    Serial.println(accelBias[1]);
    Serial.println(accelBias[2]);
    Serial.println("printing acc scale xyz: ");
    Serial.println(accelScaleFactor[0]);
    Serial.println(accelScaleFactor[1]);
    Serial.println(accelScaleFactor[2]);
  }


  // Calibrate magnetometer if it has not already been done
  if (calMagEEPROM != 1 || calibrateMag)
  {
    Serial.println("Calibrating magnetometer.");
    Serial.println("Wave IMU in a figure 8 until calibration has finished.");
    status = IMU.calibrateMag();
    if (status > 0)
    { Serial.println("Finished. IMU magnetometer calibration success."); }
    else
    { Serial.println("IMU magnetometer calibration failure."); }
    
    // Collect biases and scale factors
    magBias[0] = IMU.getMagBiasX_uT();
    magBias[1] = IMU.getMagBiasY_uT();
    magBias[2] = IMU.getMagBiasZ_uT();
    magScaleFactor[0] = IMU.getMagScaleFactorX();
    magScaleFactor[1] = IMU.getMagScaleFactorY();
    magScaleFactor[2] = IMU.getMagScaleFactorZ();

    // Write acc biases and scale factors to EEPROM
    IMU.setMagCalX(magBias[0], magScaleFactor[0]);
    IMU.setMagCalY(magBias[1], magScaleFactor[1]);
    IMU.setMagCalZ(magBias[2], magScaleFactor[2]);
    
    // Set calibration status to 1 in EEPROM (to indicate previous calibration) 
    calMagEEPROM = 1;
    EEPROM.write(calMagAddress, calMagEEPROM);
  }
  else
  {    
    magBias[0] = IMU.getMagBiasX_uT_EEPROM();
    magBias[1] = IMU.getMagBiasY_uT_EEPROM();
    magBias[2] = IMU.getMagBiasZ_uT_EEPROM();
    magScaleFactor[0] = IMU.getMagScaleFactorX_EEPROM();
    magScaleFactor[1] = IMU.getMagScaleFactorY_EEPROM();
    magScaleFactor[2] = IMU.getMagScaleFactorZ_EEPROM();
    
    Serial.println("printing mag bias xyz: ");
    Serial.println(magBias[0]);
    Serial.println(magBias[1]);
    Serial.println(magBias[2]);
    Serial.println("printing mag scale xyz: ");
    Serial.println(magScaleFactor[0]);
    Serial.println(magScaleFactor[1]);
    Serial.println(magScaleFactor[2]);
  }


  // TODO: Add low power mode
  
}

void loop() {
  update_IMU_data();
  update_euler_angles();
  print_data();
}

void update_IMU_data()
{
  IMU.readSensor();
  
  IMUaccel[0] = IMU.getAccelX_mss();
  IMUaccel[1] = IMU.getAccelY_mss();
  IMUaccel[2] = IMU.getAccelZ_mss();

  IMUgyro[0] = IMU.getGyroX_rads();
  IMUgyro[1] = IMU.getGyroY_rads();
  IMUgyro[2] = IMU.getGyroZ_rads();

  IMUmag[0] = IMU.getMagX_uT();
  IMUmag[1] = IMU.getMagY_uT();
  IMUmag[2] = IMU.getMagZ_uT();

  IMUtemp = IMU.getTemperature_C();
}

void update_euler_angles()
{
  deltat = fusion.deltatUpdate();

//  fusion.MahonyUpdate(IMUgyro[0], IMUgyro[1], IMUgyro[2], IMUaccel[0], IMUaccel[1], IMUaccel[2], deltat);  //mahony is suggested if there isn't the mag and the mcu is slow
  fusion.MadgwickUpdate(IMUgyro[0], IMUgyro[1], IMUgyro[2], IMUaccel[0], IMUaccel[1], IMUaccel[2], deltat);  //else use the magwick, it is slower but more accurate

  pitch = fusion.getPitch(); // In degrees
  roll = fusion.getRoll(); 
  yaw = fusion.getYaw();
}

void print_data()
{
  Serial.print("A: ");
  Serial.print(IMUaccel[0]);Serial.print(", ");
  Serial.print(IMUaccel[1]);Serial.print(", ");
  Serial.print(IMUaccel[2]);
  Serial.print("\tG: ");
  Serial.print(IMUgyro[0]);Serial.print(", ");
  Serial.print(IMUgyro[1]);Serial.print(", ");
  Serial.print(IMUgyro[2]);
//  Serial.print("\tM: ");
//  Serial.print(IMUmag[0]);Serial.print(", ");
//  Serial.print(IMUmag[1]);Serial.print(", ");
//  Serial.print(IMUmag[2]);
  Serial.print("\tT: ");
  Serial.print(IMUtemp);
  Serial.print("\t P: ");
  Serial.print(pitch);
  Serial.print("\tR: ");
  Serial.print(roll);
  Serial.print("\tY: ");
  Serial.println(yaw);
}
