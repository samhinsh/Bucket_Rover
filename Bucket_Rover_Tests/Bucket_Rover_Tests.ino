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
#define ONE_SEC            1000
#define TWO_SEC            2000
#define THREE_SEC          3000
#define TEN_SEC            10000
#define TWO_MIN            120000
#define MTR_SPEED_REGULAR  85
#define MTR_SPEED_FAST     100
#define MTR_STOP_DELAY     1500
 
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
  
  bDetected,   // signal detected
  bUndetected  // signal undetected
};
//=======================================================================

//=======================================================================
// Timer Enums - for utilizing timing various actions

// Indicates timer type
enum Timer {
  
  MotorSpeed_Timer, // timer between changing the speed of the motor
};
//=======================================================================

//=======================================================================
// State and Environment variables

TapeActivity centerTapeSet = tUndefined; // center 3 tape-set
TapeActivity outsideTapeSet = tUndefined; // outside two tape-set
BeaconStat beacon_1kHz = bUndetected; // 1kHz beacon detection status
BeaconStat beacon_5kHz = bUndetected; // 5kHz beacon detection status
Servo servo1; // arm mechanism 
Servo servo2; // arm mechanism
Servo servo3; // arm mechanism
//=======================================================================

//=======================================================================
// Pins - Physical pinout of circuitry

// Unassigned
const int middleCenterTape = A4;     // middle row tape
const int middleLeftTape = 0;
const int middleRightTape = 0;
const int middleFarLeftTape = 0;
const int middleFarRightTape = 0;
const int beaconDetector = 0;       // IR detection circuit
const int leftMtrDirectionPin = 7;  // motor H-bridge direction pin
const int rightMtrDirectionPin = 4;
const int leftMtrEnablePin = 6;       // motor H-bridge power pin (pwm)
const int rightMtrEnablePin = 5;
const int LEDpin = 0;               // state indicator LED
const int servo1Pin = 9;
const int servo2Pin = 10;
const int servo3Pin = 11;
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
  pinMode(LEDpin, OUTPUT);
    
  delay(MTR_STOP_DELAY);
  
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo3.attach(servo3Pin);
  
}

void loop() {
  ReportTapeSensors();
}


//=======================================================================
// Tests

void ReportTapeSensors(){
  Serial.println(centerTapeSet);
  ReadTapeSensors();
  
}

//=======================================================================
// Functions Needing Testing

// Get reading from 1kHz beacon-detector circuit
void Check1kHzBeaconDetector(){
  // Todo: set beacon pin to 1kHz
  
  // on-value inverted (detected shows LOW on circuit), so flip the result
  !digitalRead(beaconDetector) > 0? beacon_1kHz = bDetected : beacon_1kHz = bUndetected;
}

// Get reading from 5kHz beacon-detector circuit
void Check5kHzBeaconDetector(){
  // Todo: set beacon pin to 5kHz
  
  !digitalRead(beaconDetector) > 0? beacon_5kHz = bDetected : beacon_5kHz = bUndetected;
}

void FollowLine(){
  
  // Todo:
  // Implement tape-reading cases and motor turning + timer (?-unsure)
}
//=======================================================================

//=======================================================================
// Functions Successfully Tested

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
  analogWrite(rightMtrEnablePin, map(abs(leftMtrSpeed), 0, 100, 0, 200));
}

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

// check that the motor can move from forward to reverse
// Results: pass
// Review: make sure to stop motor before reversing direction (don't go
//         directly from forward to reverse
void T_MotorTest_ForwardToReverse(){
  static int mtrSpeed = MTR_SPEED_REGULAR;
  SetLeftRightMotorSpeed(mtrSpeed, mtrSpeed);
  if(IsTimerExpired(MotorSpeed_Timer)){
    StartTimer(MotorSpeed_Timer, THREE_SEC);
    SetLeftRightMotorSpeed(0, 0);
    mtrSpeed = -mtrSpeed; 
  }
}

// Set motors to constant forward speed
// Result: pass
void MoveForward(){
  SetLeftRightMotorSpeed(MTR_SPEED_REGULAR, MTR_SPEED_REGULAR);
}

// Set motors to constant reverse speed
// Result: pass
void MoveReverse(){
  SetLeftRightMotorSpeed(-MTR_SPEED_REGULAR, -MTR_SPEED_REGULAR);
}

// Stop motors
// Result: pass
void StopMoving(){
  SetLeftRightMotorSpeed(0, 0);
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


