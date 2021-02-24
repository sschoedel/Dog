#include <Trajectory.h>

#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

// nrf24 parameters
//RF24 radio(5, 6); // ce csn pins for v1
RF24 radio(9, 8); // ce csn pins for v2

//const byte address[6] = "000001"; // Address for v1
const byte address[6] = "000002"; // Address for v2

#define MAX_TRAJECTORY_SUBPOINTS 200
#define MAX_MESSAGE_BUFFER 64
#define MAX_CMD_BUFFER 64
#define NUM_SERVOS 12

//const int servoPins[12] = {15, 18, 19, 17, 20, 21, 14, 9, 7, 16, 10, 8}; // Servo pins for v1
const int servoPins[12] = {6, 7, 3, 5, 2, 4,    17, 16, 15, 20, 14, 21}; // Servo pins for v2

// Create servo objects
Servo flhr; // front left hip roll
Servo flhp; // front left hip pitch
Servo fls;  // front left shin

Servo frhr;
Servo frhp;
Servo frs;

Servo blhr;
Servo blhp;
Servo bls;

Servo brhr;
Servo brhp;
Servo brs;

// array of servos:
//Servo servos[12] = {flhr, flhp, fls, frhr, frhp, frs, blhr, blhp, bls, brhr, brhp, brs}; // Servo mapping for v1
Servo servos[12] = {flhr, flhp, fls, frhr, frhp, frs, blhr, blhp, bls, brhr, brhp, brs}; // Servo mapping for v2

// Create leg trajectory objects
Trajectory flTraj(MAX_TRAJECTORY_SUBPOINTS); // front left
Trajectory frTraj(MAX_TRAJECTORY_SUBPOINTS); // front right
Trajectory blTraj(MAX_TRAJECTORY_SUBPOINTS); // back left
Trajectory brTraj(MAX_TRAJECTORY_SUBPOINTS); // back right


//// Body geometry in mm for v1:
//float thighLen = 125;
//float shinLen = 125; // This value changes slightly based on incident angle with ground
//float hipOffsetLen = 36;
//float dogLength = 210;
//float dogWidth = 97;

// Body geometry in mm for v2:
float thighLen = 125;
float shinLen = 125; // This value changes slightly based on incident angle with ground
float hipOffsetLen = 36;
float dogLength = 215;
float dogWidth = 97;

// For serial communication
char message[MAX_MESSAGE_BUFFER] = {};
int messageIndex = 0;
String cmd[MAX_CMD_BUFFER] = {};
int cmdIndex = 0;

// PWM tracking arrays
int boundedPWMs[NUM_SERVOS];
int rawPWMs[NUM_SERVOS] = {0};
int servoPWMs[NUM_SERVOS];
//int minPWMs[NUM_SERVOS] = {0, 0, 10, 0, 0, 0, 0, 0, 15, 0, 0, 15};                                              // For v1
//int maxPWMs[NUM_SERVOS] = {180, 180, 150, 180, 180, 135, 180, 180, 155, 180, 180, 145};                         // For v1
//bool invertServo[NUM_SERVOS] = {false, false, true, true, true, false, true, false, true, false, true, false};  // For v1
int minPWMs[NUM_SERVOS] = {0, 0, 35, 0, 0, 25,     0, 0, 22, 0, 0, 27};                                         // For v2
int maxPWMs[NUM_SERVOS] = {180, 180, 175, 180, 180, 165,     180, 180, 168, 180, 180, 165};                     // For v2
bool invertServo[NUM_SERVOS] = {false, false, true, true, false, false, true, false, true, false, false, false};  // For v2

// Offsets to get theta = 0 for  each servo
//int offsets[NUM_SERVOS] = {69, 38, 117, 69, 41, 92, 62, 37, 93, 69, 52, 96}; // For v1
int offsets[NUM_SERVOS] = {90, 56, 80, 83, 131, 99,     83, 48, 90, 90, 135, 100}; // For v2
int hipRotationExtraOffset = 12; // degrees

// Flags for if commanded PWM value exceeds defined limits
bool pwmRaw_exceeded_bounds[NUM_SERVOS] = {false};

// Output shaft motor angles in degrees
float motorRotations[NUM_SERVOS] = {90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90};

bool debugging = false;
bool debuggingRotations = false;

// Local and global positioning variables
float hOffset, vOffset, fbOffset, yawOffset; // relative to global

// ===========================
// Variables for wifi commands
// ===========================

// Stationary rotations and translations
int JSx = 512, JSy = 512   ;
int buttonTapped;
int yaw, pitch, roll;

int receivingByte = 1;
bool receiveJS = true;
bool receiveIMU = false;
int initialRoll, initialPitch, initialYaw;
bool firstButtonTapped = false;

bool sending = true;

bool buttonPress;
bool prevButtonPress = false;
// Horizontal_vertical, frontback_pitch, yaw_roll
enum JScontrol_enum {H_V, FB_Y, P_R, TRANS_ONE_LEG, TRANS_TROT, ROTATE_ONE_LEG, ROTATE}; // determines which two axes the joystick controls. Cycled with the button
JScontrol_enum JScontrol = H_V;

// Heartbeat tracking and duration
float lastTime = millis();
float sendTime = 1000;

// Receiver watchdog time
unsigned int receiverWatchDog;
unsigned int receiverWatchDogTimer = 500; // ms
bool receiverWDTimerTripped = false;

/*
 * Locomotion
 */ 
enum gait_enum {trot_slow, trot, gallop, pronk};
gait_enum gait = trot;

enum trot_enum {up, front, middle, back, stop_trot};
trot_enum flTrot = up;
trot_enum frTrot = middle;
trot_enum blTrot = middle;
trot_enum brTrot = up;

float botFrontInitial = 150;

float desiredGaitVelY;
float desiredGaitVelX;
float desiredGaitRotationVel;

int stopThresholdFB = 1;
int stopThresholdSideways = 1;
int stopThresholdRot = 1;

Vector3 flCurrPoint;
Vector3 frCurrPoint;
Vector3 blCurrPoint;
Vector3 brCurrPoint;

double maxMotionDelay = 17;
double currentDelay = maxMotionDelay;
double prevStepUpdate = millis();

void setup() {
  // start gait as continuous trot for now
  Serial.begin(115200);

//  startIMU();

  // Start nrf by reading - switch to writing only when required
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  // Assign all servo objects to appropriate pins
  for (int i = 0; i < NUM_SERVOS; i++)
  {
    servos[i].attach(servoPins[i]);
  }

  Serial.println("moving to crouch position");
  
  wholeDogKinematics(0, 0, 150, 0, 0, 0);
  moveToRotations(); // Set to crouching position on startup
  
  flTraj.setCurrentTrajectoryPoint(0, 0, botFrontInitial);
  frTraj.setCurrentTrajectoryPoint(0, 0, botFrontInitial);
  blTraj.setCurrentTrajectoryPoint(0, 0, botFrontInitial);
  brTraj.setCurrentTrajectoryPoint(0, 0, botFrontInitial);
}

void loop() {

  // Check receiver watch dog to make sure data is still being received
  receiverWDCheck();
  
  // Read incoming serial data
  if (Serial.available() > 0)
  {
    readInput();
    parseCmd();

    // Print flags for debugging
    if (debugging)
    {
      displayFlags();
    }
  }

  // Read incoming radio data
  receiveFromController(receiveJS, receiveIMU, JSy, JSx, buttonPress, buttonTapped, initialRoll, initialPitch, initialYaw, roll, pitch, yaw, receiverWatchDog);


  /* 
   * ========================
   * ====  GAIT CONTROL  ====
   * ========================
   */
  if (!receiverWDTimerTripped)
  {
    // Use controller values
    controllerInputs();
    movementManager();
  }
  else
  {
    // Set to crouching position
    wholeDogKinematics(0, 0, 170, 0, 0, 0);
    moveToRotations();
  }

  // Send data over wifi
  if (sending)
  {
    // Always stop listening before opening writing pipe
    radio.stopListening();
    radio.openWritingPipe(address);

    char textSend[] = "heartbeat <3";
    radio.write(&textSend, sizeof(textSend));
//    Serial.print("Wrote: "); Serial.println(textSend);

    // Re-open reading pipe to receive more data
    radio.openReadingPipe(0, address);
    radio.startListening();
    sending = false;
  }

  // Send to controller every sendTime milliseconds
  if (millis() > lastTime + sendTime)
  {
    lastTime = millis();
    sending = true;
  }


  resetMessage();
  resetCmd();
  resetFlags();

}

/*
 * Primary function for controlling locomotion
 */
void movementManager()
{
  if (JScontrol != TRANS_ONE_LEG && JScontrol != TRANS_TROT && JScontrol != ROTATE_ONE_LEG && JScontrol != ROTATE) // Stationary rotations and translations
  {
    //                 H_V        FB_Y      H_V     P_R    P_R    FB_Y
    wholeDogKinematics(hOffset, fbOffset, vOffset, pitch, roll, yawOffset);
    moveToRotations();
  }
  else // Walk
  {
    currentDelay = maxMotionDelay - max(max(abs(desiredGaitVelX), abs(desiredGaitVelY)), abs(desiredGaitRotationVel));
    
    updateLegTrajectories();
    moveToPositions();
    
    if (millis() > prevStepUpdate + currentDelay) // Only step to the next leg position every currentDelay milliseconds
    {
      prevStepUpdate = millis();
      stepLegPositions();
    }
  }
}

void updateLegTrajectories()
{
  if (gait == trot)
  {
    /*
    * 4 intermediate positions for each step, adjacent legs offset by two
    * FRONT LEFT LEG:    front middle  back   up
    * FRONT RIGHT LEG:   back   up     front  middle
    * BACK LEFT LEG:     back   up     front  middle
    * BACK RIGHT LEG:    front middle  back   up
    */

    // Modify these parameters to change gait speeds
    const int subPts = 10;
    const int initialFwdThrow = 35;
    const int initialSideThrow = 25;
    const int initialRotThrow = 25;

    float botFront = vOffset+20;
    float botBack = vOffset+20;
    float top = vOffset-30;

    // Front legs, forward position offset
    float frontForwardOffset = 0;
    
    // Back legs, forward position offset
    float backForwardOffset = 18;

    // Set forward/backward gait throws
    static float forward = 0;

    // Handle controller deadzone for front/back motion
    if (desiredGaitVelY > stopThresholdFB)
    { 
      forward = initialFwdThrow + desiredGaitVelY; 
    }
    else if (desiredGaitVelY < -stopThresholdFB)
    { 
      forward = -initialFwdThrow + desiredGaitVelY;
    }
    else
    { 
      forward = 0; 
    }


    // Set side to side gait throws
    static float sideways = 0;

    // Handle controller deadzone for side to side motion (if in TRANS_TROT mode)
    if (desiredGaitVelX > stopThresholdSideways)
    { 
      sideways = initialSideThrow + desiredGaitVelX; 
    }
    else if (desiredGaitVelX < -stopThresholdSideways)
    { 
      sideways = -initialSideThrow + desiredGaitVelX; 
    }
    else
    { 
      sideways = 0; 
    }

    // Set rotation throw
    static float rotAdd = 0;

    // Handle controller deadzone for rotating motion (if in ROTATE mode)
    if (desiredGaitRotationVel > stopThresholdRot)
    { 
      rotAdd = initialRotThrow + desiredGaitRotationVel; 
    }
    else if (desiredGaitRotationVel < -stopThresholdRot)
    { 
      rotAdd = -initialRotThrow + desiredGaitRotationVel;
    }
    else
    { 
      rotAdd = 0; 
    }

    // Put legs in stationary position if no movement velocity is applied
    if (desiredGaitVelX < stopThresholdSideways && desiredGaitVelX > -stopThresholdSideways 
        && desiredGaitVelY < stopThresholdFB && desiredGaitVelY > -stopThresholdFB
        && desiredGaitRotationVel < stopThresholdRot && desiredGaitRotationVel > -stopThresholdRot)
    {
      
      // Create trajectories from most recent foot end points to standing positions
      // and put gait into stop state for each individual leg
      if (flTrot != stop_trot)
      {
        flTrot = stop_trot;
        flCurrPoint = flTraj.getCurrentTrajectoryPoint();
        flTraj.interpolate(flCurrPoint.x,0, flCurrPoint.y,frontForwardOffset, flCurrPoint.z,botFront, subPts);
      }
      if (frTrot != stop_trot)
      {
        frTrot = stop_trot;
        frCurrPoint = frTraj.getCurrentTrajectoryPoint();
        frTraj.interpolate(frCurrPoint.x,0, frCurrPoint.y,frontForwardOffset, frCurrPoint.z,botFront, subPts);
      }
      if (blTrot != stop_trot)
      {
        blTrot = stop_trot;
        blCurrPoint = blTraj.getCurrentTrajectoryPoint();
        blTraj.interpolate(blCurrPoint.x,0, blCurrPoint.y,backForwardOffset, blCurrPoint.z,botBack, subPts);
      }
      if (brTrot != stop_trot)
      {
        brTrot = stop_trot;
        brCurrPoint = brTraj.getCurrentTrajectoryPoint();
        brTraj.interpolate(brCurrPoint.x,0, brCurrPoint.y,backForwardOffset, brCurrPoint.z,botBack, subPts);
      }

      if (debugging)
      {
        if (flTraj.trajectoryComplete == false)
        { Serial.println("Stopping"); }
        else
        { Serial.println("Stopped"); }
      }
    }
    else // Put legs in starting position
    {
      // Create trajectories from most recent foot end points to gait start positions
      // and put each leg's gait enum into the appropriate state (up, middle, middle, up for starting position)
      if (flTrot == stop_trot)
      {
        flTrot = up;
        flCurrPoint = flTraj.getCurrentTrajectoryPoint();
        flTraj.interpolate(flCurrPoint.x,0, flCurrPoint.y,frontForwardOffset, flCurrPoint.z,top, subPts);
      }
      if (frTrot == stop_trot)
      {
        frTrot = middle;
        frCurrPoint = frTraj.getCurrentTrajectoryPoint();
        frTraj.interpolate(frCurrPoint.x,0, frCurrPoint.y,frontForwardOffset, frCurrPoint.z,botFront, subPts);
      }
      if (blTrot == stop_trot)
      {
        blTrot = middle;
        blCurrPoint = blTraj.getCurrentTrajectoryPoint();
        blTraj.interpolate(blCurrPoint.x,0, blCurrPoint.y,backForwardOffset, blCurrPoint.z,botBack, subPts);
      }
      if (brTrot == stop_trot)
      {
        brTrot = up;
        brCurrPoint = brTraj.getCurrentTrajectoryPoint();
        brTraj.interpolate(brCurrPoint.x,0, brCurrPoint.y,backForwardOffset, brCurrPoint.z,top, subPts);
      }        
    }
    
    if (flTraj.trajectoryComplete == true)
    {
      switch (flTrot)
      {
        case up:
          flTraj.interpolate(0,-sideways-rotAdd, 0,-forward-rotAdd, top,botFront, subPts);
          flTrot = front;
          break;
        case front:
          flTraj.interpolate(-sideways-rotAdd,0, -forward-rotAdd,0, botFront,botFront, subPts);
          flTrot = middle;
          break;
        case middle:
          flTraj.interpolate(0,sideways+rotAdd, 0,forward+rotAdd, botFront,botFront, subPts);
          flTrot = back;
          break;
        case back:
          flTraj.interpolate(sideways+rotAdd,0, forward+rotAdd,0, botFront,top, subPts);
          flTrot = up;
          break;
        case stop_trot:
          if (debugging)
          { Serial.println("Trot stopped"); }
      }
    }
    
    if (frTraj.trajectoryComplete == true)
    {
      switch (frTrot)
      {
        case up:
          frTraj.interpolate(0,-sideways-rotAdd, 0,-forward+rotAdd, top,botFront, subPts);
          frTrot = front;
          break;
        case front:
          frTraj.interpolate(-sideways-rotAdd,0, -forward+rotAdd, 0, botFront,botFront, subPts);
          frTrot = middle;
          break;
        case middle:
          frTraj.interpolate(0,sideways+rotAdd, 0,forward-rotAdd, botFront,botFront, subPts);
          frTrot = back;
          break;
        case back:
          frTraj.interpolate(sideways+rotAdd,0, forward-rotAdd,0, botFront,top, subPts);
          frTrot = up;
          break;
        case stop_trot:
          if (debugging)
          { Serial.println("Trot stopped"); }
      }
    }
    
    if (blTraj.trajectoryComplete == true)
    {
      switch (blTrot)
      {
        case up:
          blTraj.interpolate(0,-sideways+rotAdd, 0 + backForwardOffset,-forward-rotAdd + backForwardOffset, top,botFront, subPts);
          blTrot = front;
          break;
        case front:
          blTraj.interpolate(-sideways+rotAdd,0, -forward-rotAdd + backForwardOffset, 0 + backForwardOffset, botFront,botFront, subPts);
          blTrot = middle;
          break;
        case middle:
          blTraj.interpolate(0,sideways-rotAdd, 0 + backForwardOffset,forward+rotAdd + backForwardOffset, botFront,botFront, subPts);
          blTrot = back;
          break;
        case back:
          blTraj.interpolate(sideways-rotAdd,0, forward+rotAdd + backForwardOffset,0 + backForwardOffset, botFront,top, subPts);
          blTrot = up;
          break;
        case stop_trot:
          if (debugging)
          { Serial.println("Trot stopped"); }
      }
    }
    
    if (brTraj.trajectoryComplete == true)
    {
      switch (brTrot)
      {
        case up:
          brTraj.interpolate(0,-sideways+rotAdd, 0 + backForwardOffset,-forward+rotAdd + backForwardOffset, top,botFront, subPts);
          brTrot = front;
          break;
        case front:
          brTraj.interpolate(-sideways+rotAdd,0, -forward+rotAdd + backForwardOffset, 0 + backForwardOffset, botFront,botFront, subPts);
          brTrot = middle;
          break;
        case middle:
          brTraj.interpolate(0,sideways-rotAdd, 0 + backForwardOffset,forward-rotAdd + backForwardOffset, botFront,botFront, subPts);
          brTrot = back;
          break;
        case back:
          brTraj.interpolate(sideways-rotAdd,0, forward-rotAdd + backForwardOffset,0 + backForwardOffset, botFront,top, subPts);
          brTrot = up;
          break;
        case stop_trot:
          if (debugging)
          { Serial.println("Trot stopped"); }
      }
    }
  }
  else if (gait == trot_slow)
  {
    
  }
  else
  {
    Serial.println("Other gaits not programmed yet");
  }
}

void moveToPositions()
{
  flCurrPoint = flTraj.getCurrentTrajectoryPoint();
  frCurrPoint = frTraj.getCurrentTrajectoryPoint();
  blCurrPoint = blTraj.getCurrentTrajectoryPoint();
  brCurrPoint = brTraj.getCurrentTrajectoryPoint();
  
  kinematics(flCurrPoint.x, flCurrPoint.y, flCurrPoint.z, 1,1);
  kinematics(frCurrPoint.x, frCurrPoint.y, frCurrPoint.z, 2,1);
  kinematics(blCurrPoint.x, blCurrPoint.y, blCurrPoint.z, 1,2);
  kinematics(brCurrPoint.x, brCurrPoint.y, brCurrPoint.z, 2,2);
  
  moveToRotations();
}

void stepLegPositions()
{
  flTraj.step();
  frTraj.step();
  blTraj.step();
  brTraj.step();
}

/*
 * Cycle through control enum if button is pressed down
 */
void checkJoystickControl()
{
  if (prevButtonPress && !buttonPress)
  {
    buttonTapped = true;
  }
  else
  {
    buttonTapped = false;
  }
  
  prevButtonPress = buttonPress;
  
  if (buttonTapped)
  {
    switch(JScontrol)
    {
    case ROTATE:
      JScontrol = H_V; // Horizontal and vertical control
      break;
    case H_V:
      JScontrol = FB_Y; // Forward/back and yaw control
      break;
    case FB_Y:
      JScontrol = P_R; // Pitch and roll control
      break;
    case P_R:
      JScontrol = TRANS_ONE_LEG; // Switch to individual legged walking (translation in both directions)
      gait = trot_slow;
      break;
    case TRANS_ONE_LEG:
      JScontrol = TRANS_TROT; // Switch to simultaneous diagonal legged walking (translation in both directions)
      gait = trot;
      break;
    case TRANS_TROT:
      JScontrol = ROTATE_ONE_LEG; // Switch to individual legged walking (translation forward and back, rotation left and right)
      gait = trot_slow;
      break;
    case ROTATE_ONE_LEG:
      JScontrol = ROTATE; // Switch to simultaneous diagonal legged walking (translation forward and back, rotation left and right)
      gait = trot;
      break;
    }
  }
}

/*
 *  Setter for joystick reliant variables
 */
void controllerInputs()
{
  // Offsets added to absolute yaw when rotation and translation implemented
  if (JScontrol == H_V) // Control horizontal/vertical offset
  {
    hOffset = map(JSx, 0, 1024, -100,100);  // mm
    vOffset = map(JSy, 0, 1024, 40,200);  // mm
  }
  else if (JScontrol == FB_Y) // Control front-to-back offset and yaw
  {
    yawOffset = map(JSx, 0, 1024, -60, 60); // degrees
    fbOffset = map(JSy, 0, 1024, -150, 150); // mm
  }
  else if (JScontrol == P_R) // Control pitch and roll
  {
    roll = map(JSx, 0, 1024, -60, 60); // degrees
    pitch = map(JSy, 0, 1024, -45, 45); // degrees
  }
  else if (JScontrol == TRANS_ONE_LEG || JScontrol == TRANS_TROT)
  {
    // Forward/backward motion: positive is front of dog
    desiredGaitVelY = map(JSy, 0, 1024, -10,10);

    // Side to side motion: positive is right of dog
    desiredGaitVelX = map(JSx, 0, 1024, -10,10);
  }
  else if (JScontrol == ROTATE)
  {
    desiredGaitVelY = map(JSy, 0, 1024, -10,10);
    desiredGaitRotationVel = map(JSx, 0, 1024, -10,10);
  }

// IMU control
  //  int yawOffset = map(yaw, 0, 360, -40, 40);
  //  pitch = map(pitch, -180, 180, -40, 40);
  //  roll = map(roll, -180, 180, -40, 40);

//  Serial.print("yawOffset, pitch, roll: ");
//  Serial.print(yawOffset);Serial.print(", ");Serial.print(pitch);Serial.print(", ");Serial.println(roll);
}

/*
   test function for moving foot in circle in yz plane
*/
float yPos = 0;
float zPos = 0;
float xPos = 0;
float prevTime = millis();
float timeBetweenPoints = 10;  // update position 100 times a second (if rtos but its not so will be off)

void trotTest()
{
  static float t = 0; //degrees

  if (t < 180)
  {
    zPos = 170 + 45 * cos(2 * t * M_PI / 180);
  }
  else // t between 180 and 360
  {
    zPos = 215;
  }

  yPos = 40 +  40 * cos(t * M_PI / 180);

  t += 0.002;

  if (t > 360)
  {
    t = 0;
  }

  if (millis() > prevTime + timeBetweenPoints)
  {
    kinematics(xPos, yPos, zPos, 1, 1);
  }
}

/*
   parseCmd determines what actions to take based on input
*/
void parseCmd()
{
  if (cmd[0] == "gpwm")
  {
    updateGoalPWMs();
  }
  // leg position
  else if (cmd[0] == "lpos") // for only one leg right now - with all four legs xyz should command center position of robot
  {
    int legSide = cmd[1].toInt();
    int leg = cmd[2].toInt();
    float x = cmd[3].toInt();
    float y = cmd[4].toInt();
    float z = cmd[5].toInt();
    kinematics(x, y, z, legSide, leg);
    moveToRotations();
  }
  else if (cmd[0] == "theta")
  {
    int servo = cmd[1].toInt();
    float inputAngle = cmd[2].toFloat();
    motorRotations[servo] = offsets[servo] + inputAngle;
    moveToRotations();
  }
  // dog position
  else if (cmd[0] == "dpos")
  {
    float x = cmd[1].toInt();
    float y = cmd[2].toInt();
    float z = cmd[3].toInt();
    wholeDogKinematics(x, y, z, 0, 0, 0); // TODO: add rotation input
    moveToRotations();
  }
  else if (cmd[0] == "interp")
  {
    float x1 = cmd[1].toInt();
    float y1 = cmd[2].toInt();
    float z1 = cmd[3].toInt();
    float x2 = cmd[4].toInt();
    float y2 = cmd[5].toInt();
    float z2 = cmd[6].toInt();
    int subPoints = cmd[7].toInt();
    flTraj.interpolate(x1, x2, y1, y2, z1, z2, subPoints);
  }
}

/*
   Actions to be called by parseCmd
*/
void updateGoalPWMs()
{
  int leg = cmd[1].toInt(); // 1, 2, 3, or 4 (fl, fr, bl, br)

  // TODO: invert directions where necessary

  int s1PWM = cmd[2].toInt();
  int s2PWM = cmd[3].toInt();
  int s3PWM = cmd[4].toInt();

//  Serial.print("PWMS: "); Serial.print(s1PWM); Serial.print(" "); Serial.print(s2PWM); Serial.print(" "); Serial.println(s3PWM);
  if (leg == 1)
  {
    servos[0].write(s1PWM);
    servos[1].write(s2PWM);
    servos[2].write(s3PWM);
  }
  else if (leg == 2)
  {
    servos[3].write(s1PWM);
    servos[4].write(s2PWM);
    servos[5].write(s3PWM);
  }
  else if (leg == 3)
  {
    servos[6].write(s1PWM);
    servos[7].write(s2PWM);
    servos[8].write(s3PWM);
  }
  else if (leg == 4)
  {
    servos[9].write(s1PWM);
    servos[10].write(s2PWM);
    servos[11].write(s3PWM);
  }

  if (debugging)
  {
    Serial.println("PWMs updated");
  }
}

/*
   readInput and parseIntoCmd are for making cmd string
*/
void readInput()
{
  // Fill message array with incoming chars
  while (Serial.available() > 0)
  {
    message[messageIndex] = Serial.read();
    messageIndex++;
  }
  parseIntoCmd();
  messageIndex = 0;
  //  if (debugging)
  //  {
  //    Serial.println(message);
  //  }
}

void parseIntoCmd()
{
  cmdIndex = 0;
  int lasti = 0; // Saves most recent place after last string split
  String messageStr(message); // Char array into string so can use substring
  for (int i = 0; i < messageIndex; i++)
  {
    if (message[i] == ',') // Delineates command word
    {
      cmd[cmdIndex] = messageStr.substring(lasti, i);
      cmdIndex++;
      lasti = i + 1;
    }
    else if (i == messageIndex - 1) // Last command (no ending comma)
    {
      if (message[messageIndex] != ',') // Ignore any stray ending commas
      {
        cmd[cmdIndex] = messageStr.substring(lasti, i + 1);
      }
    }
  }

  if (debugging)
  {
    Serial.print("Received command: ");
    for (int i = 0; i <= cmdIndex; i++)
    {
      Serial.print(cmd[i]);
      Serial.print(" ");
    }
    Serial.print('\n');
  }
}

int withinBounds(int pwmRaw, int pwmMin, int pwmMax, int motorIndex)
{
  int pwmContained = pwmRaw;
  if (pwmRaw < pwmMin)
  {
    pwmContained = pwmMin;
    pwmRaw_exceeded_bounds[motorIndex] = true;
  }
  else if (pwmRaw > pwmMax)
  {
    pwmContained = pwmMax;
    pwmRaw_exceeded_bounds[motorIndex] = true;
  }

  return pwmContained;
}

// Clear char messagea and string cmd arrays
void resetMessage() {
  memset(message, 0, sizeof(message));
}

void resetCmd() {
  for (int i = 0; i < cmdIndex; i++)
  {
    cmd[i] = "";
  }
}

void resetFlags() {
  for (int i = 0; i < NUM_SERVOS; i++)
  {
    pwmRaw_exceeded_bounds[i] = false;
  }
}

void displayFlags()
{
  Serial.print("\nFLAGS\n");
  Serial.println("-------------------------------");

  // Raw PWM exceeded bounds
  Serial.println("Exceeded bounds");
  for (int i = 0; i < NUM_SERVOS; i++)
  {
    Serial.print(i); Serial.print(" : "); Serial.print(pwmRaw_exceeded_bounds[i]); Serial.print("\t");
  }
  Serial.print("\n\n\n");
}

// Turns degree angles into appropriate pwm values for each motor
void moveToRotations()
{
  // Invert pwms where necessary
  for (int i = 0; i < NUM_SERVOS; i++)
  {
    if (invertServo[i] == true)
    {
      rawPWMs[i] = 180 - motorRotations[i];
    }
    else
    {
      rawPWMs[i] = motorRotations[i];
    }
  }

  // Constrain pwms to given bounds
  for (int i = 0; i < NUM_SERVOS; i++)
  {
    servoPWMs[i] = withinBounds(rawPWMs[i], minPWMs[i], maxPWMs[i], i);
  }

  if (debuggingRotations)
  {
    // print pwm outputs
    Serial.print("\nServo PWMS\n");
    for (int i = 0; i < NUM_SERVOS; i++)
    {
      Serial.print(i); Serial.print(" : "); Serial.print(servoPWMs[i]); Serial.print('\t');
    }
    Serial.print("\n");
  }

  // Apply pwms to each servo
  for (int i = 0; i < NUM_SERVOS; i++)
  {
    servos[i].write(servoPWMs[i]);
  }
}

void receiverWDCheck()
{
  if (millis() - receiverWatchDogTimer > receiverWatchDog) // watchdog hasn't been reset in 200 ms
  {
    receiverWDTimerTripped = true;
    
//    if (debugging)
//    {
//      Serial.println("Receiver watchdog hasn't been fed :(");
//    }
  }
  else
  {
    receiverWDTimerTripped = false;
  }
}
