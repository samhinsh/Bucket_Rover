/* ME 210 Final Project, Winter 2016
 * Team: Andrew Anderson, Taylor Clayton, Mario Chapa, Sam Hinshelwood
 * Test Suite
 *
 * Goal:
 * The goal of this project is to design, build, and code an autonomous bot
 * that will pick up tokens from a designated reload station (identifiable by
 * a 5kHz infrared beacon) and drop them off in at least 3 of 5 drop-off buckets 
 * located across the stadium (and identifiable each via 1kHz IR beacons). 
 *
 * Description: 
 * This file describes funcitonality tests for various aspects of the Bucket_Rover bot.
 *
 */
 
/*---------------Includes-----------------------------------*/
#include <Timers.h>
#include <Servo.h>
#include "Pulse.h"

/*---------------Module Defines-----------------------------*/
#define LIGHT_THRESHOLD    350 // smaller at night
#define FENCE_THRESHOLD    700
#define CENTERFIND_TIME    550 // also consider adjusting reverse+stop time
#define ONE_SEC            1000
#define TWO_SEC            2000
#define THREE_SEC          3000
#define TEN_SEC            10000
#define TWO_MIN            120000
#define MTR_SPEED_REGULAR  70
#define MTR_SPEED_FAST     100
#define MTR_SPEED          MTR_SPEED_REGULAR
#define COMPLETE_STOP      200  // was using 200
#define CENTERONLINE_TIME  1000
#define MTR_STOP_DELAY     800
#define NUDGE_REDUCE_CONST_MINOR 50
#define NUDGE_REDUCE_CONST_MAJOR 70
 
/*---------------Function Prototypes-------------------------*/

// Loop Fn's
void CollectEnvInfo();

// CollectEnvInfo Helpers
void ReadTapeSensors();
void Check1kHzBeaconDetector();
void Check5kHzBeaconDetector();

// General Helpers
void FollowLine();
void MoveForward();
void MoveReverse();
void StopMoving();
void SetLeftRightMotorSpeed(int leftMtrSpeed, int rightMtrSpeed);

void StartTimer(int timer, unsigned long time);
unsigned char IsTimerExpired(int timer);

//=======================================================================
// State Enumerations - for bot activity at any given time

// Prepended "s" indicates State type variable
enum State {
  
  // TravelToCenterLine sub-routine states
  sMovingForward = 0, sTurning = 1, 
  sFindingReloadBeacon = 2, sRotatingTowardCenterLine = 3, sGoingToCenterLine = 4, sStopped = 5,
  sCenteringOnLine = 6,
  
  sGoingToBucket = 7, sDroppingOffTokens = 8, sBackingUpToReload = 9
}; 
//=======================================================================

//=======================================================================
// Tape Activity Enums - for quick summary of tape row readings

// Prepended "t" indicates TapeActivity type variable
// Indicates on-status/activity of  tape sensors in a single row
enum TapeActivity {
  
  tLeftAndCenter = 0,  // left and center sensors detecting line, and so on..
  tCenterAndRight = 1,
  tLeftAndRight = 2,
  tCenter = 3,         // only center tape detecting line
  tLeft = 4,
  tRight = 5,
  tAll = 6,            // left, center, and right detecting line
  tNone = 7,           // no sensors in set detecting line
  tUndefined = 8
};
//=======================================================================

//=======================================================================
// Beacon Detector Enums - for human readable result of beacon detection

// Prepended "b" indicates BeaconStat type variable
// Indicates detection of beacon or not
enum BeaconStat {
  
  bUndetected = 0, // signal undetected
  bDetected = 1   // signal detected
};
//=======================================================================

//=======================================================================
// RotateSpeed Enumerations - for bot speed when correcting orientation during 
//                      line following

// Prepended "r" indicates RotateSpeed tupe
enum RotateSpeed {
  
  // TravelToCenterLine sub-routine states
  rRegular, rFast
}; 
//=======================================================================

//=======================================================================
// Timer Enums - for utilizing timing various actions

// Indicates timer type
enum Timer {
  
  MotorSpeed_Timer, // timer between changing the speed of the motor
  CenterLine_Timer,
  Rotate_Timer,
  DropOff_Timer
};
//=======================================================================

//=======================================================================
// State and Environment variables

State state = sFindingReloadBeacon;
TapeActivity centerTapeSet = tUndefined; // center 3 tape-set
TapeActivity outsideTapeSet = tUndefined; // outside two tape-set
BeaconStat beacon = bUndetected; // 5kHz beacon detection status
Servo servo1; // left arm mechanism 
Servo servo2; // right arm mechanism
Servo servo3; // middle arm mechanism
//=======================================================================

//=======================================================================
// Pins - Physical pinout of circuitry

// Unassigned
const int middleCenterTape = A3;     // middle row tape
const int middleLeftTape = A5;
const int middleRightTape = A4;
const int middleFarLeftTape = 0;
const int middleFarRightTape = 0;
const int beaconDetector = A0;       // IR detection circuit
const int leftMtrDirectionPin = 7;  // motor H-bridge direction pin
const int rightMtrDirectionPin = 4;
const int leftMtrEnablePin = 6;       // motor H-bridge power pin (pwm)
const int rightMtrEnablePin = 5;
const int LEDpin = 0;               // state indicator LED
const int servo1Pin = 9; // left arm servo
const int servo2Pin = 11; // right arm servo
const int servo3Pin = 10; // middle arm servo
//=======================================================================

//=======================================================================
// Globals

// function array, for later use
void (* MovementFns[])() = {FollowLine, MoveForward, MoveReverse, StopMoving};


// Initialization (one time, setup) stuff
void setup() {
  
  // attempt to stop motors from running during Arduino hard-reset
  analogWrite(leftMtrEnablePin, LOW);
  analogWrite(rightMtrEnablePin, LOW);
  
  Serial.begin(9600);
  
  // Init pins
  
  // inputs
  pinMode(middleCenterTape, INPUT);
  pinMode(middleLeftTape, INPUT);
  pinMode(middleRightTape, INPUT);
  pinMode(middleFarLeftTape, INPUT);
  pinMode(middleFarRightTape, INPUT);
  pinMode(beaconDetector, INPUT);
  
  digitalWrite(leftMtrDirectionPin,LOW); // init motors to same direction
  digitalWrite(rightMtrDirectionPin,LOW);
  
  // outputs
  pinMode(leftMtrDirectionPin, OUTPUT);
  pinMode(rightMtrDirectionPin, OUTPUT);
  pinMode(leftMtrEnablePin, OUTPUT);
  pinMode(rightMtrEnablePin, OUTPUT);
  pinMode(LEDpin, OUTPUT);;
    
  delay(MTR_STOP_DELAY);
  
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo3.attach(servo3Pin);
  
  servo1.write(0);   // initial (compact) position
  servo2.write(180); // initial (compact) position
  servo3.write(0);   // initial (compact) position
  
  delay(1000);
  
  servo1.detach();
  servo2.detach();
  servo3.detach();
  
}

void loop() {
  
  // TwoSensorLineFollow();
  CollectEnvInfo();
  Serial.print("Tape: "); 
  Serial.println(centerTapeSet);
  Serial.print("State: ");
  Serial.println(state);
  Serial.print("Beacon state: ");
  Serial.println(beacon);
  
  Serial.print("Beacon pin: ");
  Serial.println(digitalRead(beaconDetector));
  
  // if S1
  if(state == sFindingReloadBeacon || state == sRotatingTowardCenterLine || 
     state == sGoingToCenterLine || state == sCenteringOnLine){ 
    TravelToCenterLine();
  } 
  
  // if S2
  else if(state == sGoingToBucket || state == sDroppingOffTokens ||
          state == sBackingUpToReload){
    
    // in, or leaving sGoingToBucket
    if(state == sGoingToBucket){
      if(centerTapeSet == tLeftAndRight){ // middleRowTape sitting on bucket crossroad
        MoveReverse();
        delay(COMPLETE_STOP);
        StopMoving();
        delay(500);
        Serial.println("Dropping off tokens!");
        DropOffTokens(); // start dropping off tokens
        state = sBackingUpToReload;
      } else { // not in front of buckets, keep going to bucket
        GoToBucket(); // move forward to bucket (line follow)
      }
    }
    
    // in, or leaving sDroppingOffTokens
    else if(state == sDroppingOffTokens){
      if(IsTimerExpired(DropOff_Timer)){ // done dropping off tokens (timer expired)
        // move on to sBackingUpToReload
        state = sStopped; // BackupToReloadStation();
      }  
      // otherwise, continue dropping off tokens (implicitly)
      else DropOffTokens();
      MoveReverse(); // exp
      delay(1000);
    }
    
    // in, or leaving sBackingUpToReloadStation (Todo)
    else if(state == sBackingUpToReload){
      if(centerTapeSet == tLeftAndRight){ // hit the reload crossroad from behind
        MoveForward();
        delay(COMPLETE_STOP);
        StopMoving();
        delay(500); // Reload();
        state = sStopped;
      }
      // otherwise, continue backing up (implictly)
      else BackupToReloadStation();
    }
    
    // in, or leaving sReloading (Todo)
  }
}

//=======================================================================
// Functions Needing Testing

// Back the bot up to the reload station
void BackupToReloadStation(){
  state = sBackingUpToReload;
  // change LED indicator
  
  // reverse motors for backing up (no line following)
  MoveReverse();
}

void GoToBucket(){
  state = sGoingToBucket;
  // change LED indicator
  
  // Todo:
  // start motors moving forward (likely, if not handled by FollowLine)
  MoveForward();
  
  // change direction slightly (with motors) if off line (off center tape sensor)
  TwoSensorLineFollow();
}

void DropOffTokens(){
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo3.attach(servo3Pin);
  
  servo1.write(120); // full position
  servo2.write(60);  // full position
  servo3.write(120); // full position
  
  delay(1000);
  
  servo1.write(0);   // initial (compact) position
  servo2.write(180); // initial (compact) position
  servo3.write(0);   // initial (compact) position
  
  delay(1000);
  
  servo1.detach();
  servo2.detach();
  servo3.detach();
}

void TravelToCenterLine(){ 

  // in, or leaving sFindingReloadBeacon
  if(state == sFindingReloadBeacon){
    if(beacon == bUndetected){ // beacon undetected
        FindReloadBeacon();
    } else {
      RotateInPlace('R'); // experiment
      delay(COMPLETE_STOP); // experiment
      StopMoving(); // experiment
      delay(500); // experiment
      RotateTowardCenterLine(); // otherwise, move toward center, change state
    }
  }
  
  // in, or leaving sRotatingTowardCenterLine
  else if(state == sRotatingTowardCenterLine){
    if(!IsTimerExpired(CenterLine_Timer)){ // continue rotating toward line if timer still going
      RotateTowardCenterLine();
    } else {
      RotateInPlace('L'); // experiment
      delay(COMPLETE_STOP); // experiment
      StopMoving(); // experiment
      delay(1000); // experiment
      GoToCenterLine();
    } // otherwise, head to center line
  }
  
  // in, or leaving sGoingToCenterLine
  else if(state == sGoingToCenterLine){
    if(centerTapeSet == tLeft || 
      centerTapeSet == tRight || 
      centerTapeSet == tLeftAndRight){ // line found
      MoveReverse();
      delay(COMPLETE_STOP);
      StopMoving();
      delay(1000);
      CenterOnLine_TwoSensors(); // center on line, namely the center line
    } else GoToCenterLine(); // continue going to center liene
  } 
  
  // in, or leaving sCenterOnLine 
  else if(state == sCenteringOnLine){
    CenterOnLine_TwoSensors(); // hard-coded version on the one in Bucket_Rover
  }
}

// Center bot on line
void CenterOnLine_TwoSensors(){
  state = sCenteringOnLine;
  
  if(centerTapeSet == tLeft){
    
    // move up, then stop
    MoveForward();
    delay(100);
    MoveReverse();
    delay(COMPLETE_STOP);
    StopMoving();
    delay(500);
    
    // rotate right, then stop
    RotateInPlace('R');
    delay(CENTERONLINE_TIME);
    RotateInPlace('L'); // experiment
    delay(COMPLETE_STOP); // experiment
    StopMoving(); // experiment
    delay(500); // experiment
    // state = sStopped; // exp2
    state = sGoingToBucket;
    
  } else /* if(centerTapeSet == tLeftAndRight || centerTapeSet == tRight) */{
    MoveForward(); // exp2
  }
  
  // Todo: Adjust cases as necessary
}

void MoveForward_Fast(){
  SetLeftRightMotorSpeed(MTR_SPEED_FAST, MTR_SPEED_FAST);
}

// verify that the bot can correctly identify the 5kHz reload beacon
// Results: pass
// Review: bot can pretty accurately detect the reload station on slow and high speeds
void FindReloadBeacon(){
  state = sFindingReloadBeacon;
      
  RotateInPlace('L');
  /*
  // set timer if not started
  if(IsTimerExpired(Rotate_Timer)){
    StartTimer(Rotate_Timer, 800);
    RotateInPlace('R'); // experiment
    delay(50); // experiment
    StopMoving();
    delay(500);
  }*/
}

// Rotate predetermined amount (using CenterLine_Timer) toward center line
// Results: fail
// Review: motors stall when StopMoving is called after rotating. On rotating toward
//         line, circumference of rotation is highly inconsistent. Considering removing
//         StopMoving afterward, and/or breaking up rotation into start-stop chunks
void RotateTowardCenterLine(){
  state = sRotatingTowardCenterLine;
  
  // rotate rightward toward center line
  RotateInPlace('R');
  
  // set timer if not started
  if(IsTimerExpired(CenterLine_Timer)){
    StartTimer(CenterLine_Timer, CENTERFIND_TIME);
    // Todo: Fix timing^ based on wheel speed at beacon-detection range
  }
  
  /*
  if(IsTimerExpired(Rotate_Timer)){
    StartTimer(Rotate_Timer, 800);
    RotateInPlace('L'); // experiment
    delay(50); // experiment
    StopMoving(); // experiment
    delay(500); // experiment
  } */
}

// Move forward until hitting center line
// Results: undetermined
// Review: NA
void GoToCenterLine(){
  state = sGoingToCenterLine;
  MoveForward_Fast();
}

// Set left and right motors to specified speeds
// Acceptable -100 <= MtrSpeed <= 100
// Flips the motor's direction if  MtrSpeed < 0
// Results: Pass
// Review: setting MtrDirectionPin's to HIGH is Forward, LOW is reverse
//         analogWrite is sufficient for running motors, Pulse library not needed
void SetLeftRightMotorSpeed(int leftMtrSpeed, int rightMtrSpeed){
  // Todo:
  // Orient/fix code for operating motor (below)
  
  // set direction of motor based on sign of MtrSpeed var(s)
  digitalWrite(leftMtrDirectionPin, (leftMtrSpeed) >= 0? HIGH : LOW );
  digitalWrite(rightMtrDirectionPin, (rightMtrSpeed) >= 0? HIGH : LOW);
  
  analogWrite(leftMtrEnablePin, map(abs(leftMtrSpeed), 0, 100, 0, 200));
  analogWrite(rightMtrEnablePin, map(abs(rightMtrSpeed) + 12, 0, 100, 0, 200));
}

//=======================================================================
// Tests

// verify bot can find the reload beacon and stop while facing it
void FindReloadBeacon_Test(){
  ReportBeacon();
  
  if(beacon == bUndetected){ // beacon undetected
    if(IsTimerExpired(MotorSpeed_Timer)){ // wait for timer, to rotate again
      
      RotateInPlace('L');
      // set timer if not started
      if(IsTimerExpired(MotorSpeed_Timer)){
        StartTimer(MotorSpeed_Timer, 50);
      }
    }
  } else {
    StopMoving();
  }
}

// Line follow using two sensors. Initial state expected to be on line
void TwoSensorLineFollow(){
  static int lineFollowState = 0; // 0 - movingForward, 1 - sTurning
  
  ReadTapeSensors();
  Serial.println(centerTapeSet);
  if (lineFollowState == 0){ // in, or leaving MovingForward
    if(centerTapeSet == tLeft || centerTapeSet == tRight){ // line found
      lineFollowState = 1; // leave this state, and start turning
    } else { // otherwise, no line found
      MoveForward(); // keep moving forward
    }
    
    // in or leaving, sTurning
  } else if (IsTimerExpired(MotorSpeed_Timer)){ // timer expired, or not init'd
    if(centerTapeSet == tLeft){
      lineFollowState = 1;
      
      NudgePath('R', rRegular); // move slightly right
      
      if(IsTimerExpired(MotorSpeed_Timer)){ // start timer
        StartTimer(MotorSpeed_Timer, 50);
      }
    } else if(centerTapeSet == tRight){
      lineFollowState = 1;
      
      NudgePath('L', rRegular); // move slightly right
      
      if(IsTimerExpired(MotorSpeed_Timer)){ // start timer
        StartTimer(MotorSpeed_Timer, 50);
      }
    } else {
      lineFollowState = 0;
    }
  }
}

void TwoSensorLineFollow_Alternate(){
  ReadTapeSensors();
  Serial.println(centerTapeSet);
  if (state == sMovingForward){ // in, or leaving MovingForward
    if(centerTapeSet == tLeft || centerTapeSet == tRight){ // line found
      state = sTurning; // leave this state, and start turning
    } else { // otherwise, no line found
      MoveForward(); // keep moving forward
    }
    
    // in or leaving, sTurning
  } else if (IsTimerExpired(MotorSpeed_Timer)){ // timer expired, or not init'd
    if(centerTapeSet == tLeft){
      state = sTurning;
      
      NudgePath('L', rFast); // move slightly right
      
      if(IsTimerExpired(MotorSpeed_Timer)){ // start timer
        StartTimer(MotorSpeed_Timer, 50);
      }
    } else if(centerTapeSet == tRight){
      state = sTurning;
      
      NudgePath('R', rFast); // move slightly right
      
      if(IsTimerExpired(MotorSpeed_Timer)){ // start timer
        StartTimer(MotorSpeed_Timer, 50);
      }
    } else {
      state = sMovingForward;
    }
  }
}

void ReportBeacon(){
  Serial.println(beacon);
  Check5kHzBeaconDetector();
}

// verify that bot can go from moving forward, to turning slightly
// Results: pass
void NudgePath_Test(){
  MoveForward();
  Serial.println("Moving Forward");
  delay(3000);
  NudgePath('L', rRegular);
  delay(3000);
  NudgePath('R', rRegular);
  delay(3000);
}

// verify that bot performs basic movements: forward, reverse, turn left, 
// turn right, stray left, and stray right
// Results: pass
void Movement_Test(){
  MoveForward();
  delay(5000);
  StopMoving();
  delay(MTR_STOP_DELAY);
  MoveReverse();
  delay(5000);
  StopMoving();
  delay(MTR_STOP_DELAY);
  RotateInPlace('L');
  delay(5000);
  StopMoving();
  delay(MTR_STOP_DELAY);
  RotateInPlace('R');
  delay(5000);
  StopMoving();
  delay(MTR_STOP_DELAY);
  NudgePath_Test();
}

// check that tape sensor correctly detecting tape and changing 
// tapeActivity characterization correct 
void ReportTapeSensors(){
  Serial.println(centerTapeSet);
  ReadTapeSensors();
  
}

// check that the motor can successfully change speeds through a range using pwm
// Also tests timers' functionality
// Results: pass
// Review: Lower range from 0-250 pwm not sufficient to operate motor
//         Detect min on-val in the future
void T_MotorTest_Forward(){
  static int leftMtrSpeed = 0; // gets updated from last value each time (static keyword)
  static int rightMtrSpeed = 0; // gets updated from last value each time
  static int hitTopSpeed = LOW; // gets updated from last value each time
  
  // if timer expired/not started, set timer. Change motor speed
  if(IsTimerExpired(MotorSpeed_Timer)){
    StartTimer(MotorSpeed_Timer, TWO_SEC);
    
    // cycle thru motor speeds
    // takes 10 seconds to reach top speed, same to reach bottom speed
    if(hitTopSpeed == LOW){ // increase speed
      leftMtrSpeed += 20;
      rightMtrSpeed += 20;
    } else { // decrease speed
      leftMtrSpeed -= 20;
      rightMtrSpeed -= 20;
    }
    
    // reset ascent/descent
    if (leftMtrSpeed == MTR_SPEED_FAST || rightMtrSpeed == MTR_SPEED_FAST) hitTopSpeed = HIGH;
    if(leftMtrSpeed == 0 || rightMtrSpeed == 0) hitTopSpeed = LOW;
  }
  
  // Send motor speed to motor
  SetLeftRightMotorSpeed(leftMtrSpeed, rightMtrSpeed);
}
  
// check that the motor can move from forward to reverse
// Results: pass
// Review: make sure to stop motor before reversing direction (don't go
//         directly from forward to reverse
void T_MotorTest_ForwardToReverse(){
  static int mtrSpeed = MTR_SPEED;
  SetLeftRightMotorSpeed(mtrSpeed, mtrSpeed);
  if(IsTimerExpired(MotorSpeed_Timer)){
    StartTimer(MotorSpeed_Timer, THREE_SEC);
    SetLeftRightMotorSpeed(0, 0);
    mtrSpeed = -mtrSpeed; 
  }
}

//=======================================================================

//=======================================================================
// Functions Successfully Tested

// Start specified timer
// Results: pass
void StartTimer(int timer, unsigned long time){
  TMRArd_InitTimer(timer, time);
}

// Return whether specified timer has expired or not
// Results: pass
// Review: incorporate IsTimerExpired 
unsigned char IsTimerExpired(int timer){
  return (unsigned char)(TMRArd_IsTimerExpired(timer));
}

// Set motors to constant forward speed
// Result: pass
void MoveForward(){
  SetLeftRightMotorSpeed(MTR_SPEED, MTR_SPEED);
}

// Set motors to constant reverse speed
// Result: pass
void MoveReverse(){
  SetLeftRightMotorSpeed(-MTR_SPEED, -MTR_SPEED);
}

// Stop motors
// Result: pass
void StopMoving(){
  SetLeftRightMotorSpeed(0, 0);
}

// Get tape readings for frontRowTape and backRowTape sensor rows
// Results: pass
// Review: Works preliminarily on single sensor, test again on multiple sensors
void ReadTapeSensors(){
  
  // collect individual tape readings
  char middleCenter = !digitalRead(middleCenterTape);
  char middleLeft = !digitalRead(middleLeftTape);
  char middleRight = !digitalRead(middleRightTape);
  char middleFarLeft = !digitalRead(middleFarLeftTape);
  char middleFarRight = !digitalRead(middleFarRightTape);
  
  // summarize outside tape set
  if(middleFarLeft > 0 && middleFarRight > 0) outsideTapeSet = tLeftAndRight; // left and right 
                                                                              // on outside tape detect line
  else if(middleFarLeft > 0) outsideTapeSet = tLeft;
  else if(middleFarRight > 0) outsideTapeSet = tRight;
  else outsideTapeSet = tNone;
  
  // summarize center tape set
  if(middleCenter > 0 && middleLeft > 0 && middleRight > 0) centerTapeSet = tAll;
  else if(middleLeft > 0 && middleCenter > 0) centerTapeSet = tLeftAndCenter;
  else if(middleCenter > 0 && middleRight > 0) centerTapeSet = tCenterAndRight;
  else if(middleLeft > 0 && middleRight > 0) centerTapeSet = tLeftAndRight;
  else if(middleLeft > 0) centerTapeSet = tLeft;
  else if(middleRight > 0) centerTapeSet = tRight;
  else if(middleCenter > 0) centerTapeSet = tCenter;
  else centerTapeSet = tNone;
}

// Rotates bot to the left or right slightly, with 2 intensities 
// Inputs: direction - 'L' or 'R'
//         speed     -  rRegular (1) or rFast (2)
void NudgePath(char direction, int speed){
  // set motor speeds, one slower, one faster
  // slower motor has two magnitudes of slowness
  int fasterMtrSpeed = MTR_SPEED;
  int slowerMtrSpeed = MTR_SPEED - ((speed == rRegular)? 
                  NUDGE_REDUCE_CONST_MINOR : NUDGE_REDUCE_CONST_MAJOR);
   
   // if direction is Left, set right mtr to faster speed
   if(direction == 'L'){
     SetLeftRightMotorSpeed(slowerMtrSpeed, fasterMtrSpeed);
   } else if (direction == 'R'){ // otherwise set left motor to faster speed
     SetLeftRightMotorSpeed(fasterMtrSpeed, slowerMtrSpeed);
   }
}

// Verify bot can rotate in place (without translating)
// Results: pass
// Review: the bot translates very slightly when rotating, but distance is pretty negligable 
void RotateInPlace(char direction){
  if(direction == 'L'){
    SetLeftRightMotorSpeed(MTR_SPEED, -MTR_SPEED);
  } else if (direction == 'R'){
    SetLeftRightMotorSpeed(-MTR_SPEED, MTR_SPEED);
  } 
}

// Get reading from 5kHz beacon-detector circuit
// Results: pass
// Review: Better results on botton level of bot
void Check5kHzBeaconDetector(){
  digitalRead(beaconDetector) > 0? beacon = bUndetected : beacon = bDetected;
}

// verify device can collect tape readings and characterize them, then read the
// beacon detector
// Results: pass
// Review: Double-check tape characterizations in the future for holes
void CollectEnvInfo(){
  ReadTapeSensors();
  Check5kHzBeaconDetector();
}


