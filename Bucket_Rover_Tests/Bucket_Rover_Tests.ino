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
#include "Pulse.h"

/*---------------Module Defines-----------------------------*/
#define LIGHT_THRESHOLD    350 // smaller at night
#define FENCE_THRESHOLD    700
#define ONE_SEC            1000
#define TWO_SEC            2000
#define THREE_SEC          3000
#define TEN_SEC            10000
#define TWO_MIN            120000
#define MTR_SPEED          100
 
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
// Timer Enums - for utilizing timing various actions

// Indicates timer type
enum Timer {
  
  MotorSpeed_Timer, // timer between changing the speed of the motor
};
//=======================================================================

//=======================================================================
// Pins - Physical pinout of circuitry

// Unassigned
const int frontCenterTape = 0;      // front row tape
const int middleCenterTape = 0;     // middle row tape
const int middleLeftTape = 0;
const int middleRightTape = 0;
const int beaconDetector = 0;       // IR detection circuit
const int leftMtrDirectionPin = 0;  // motor H-bridge direction pin
const int rightMtrDirectionPin = 0;
const int leftMtrStepPin = 0;       // motor H-bridge power pin (pwm)
const int rightMtrStepPin = 0;
const int LEDpin = 0;               // state indicator LED
//=======================================================================

//=======================================================================
// Globals

// function array, for later use
void (* MovementFns[])() = {FollowLine, MoveForward, MoveReverse, StopMoving};


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  T_MotorTest_Forward();
}


//=======================================================================
// Tests

// check that the motor can successfully change speeds through a range using pwm
// Also tests timers' functionality
// Results: 
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
    if (leftMtrSpeed == 100 || rightMtrSpeed == 100) hitTopSpeed = HIGH;
    if(leftMtrSpeed == 0 || rightMtrSpeed == 0) hitTopSpeed = LOW;
  }
  
  // Send motor speed to motor
  SetLeftRightMotorSpeed(leftMtrSpeed, rightMtrSpeed);
}

//=======================================================================
// Functions Needing Testing


// Set left and right motors to specified speeds
// Acceptable -100 <= MtrSpeed <= 100
// Flips the motor's direction if  MtrSpeed < 0
void SetLeftRightMotorSpeed(int leftMtrSpeed, int rightMtrSpeed){
  // Todo:
  // Orient/fix code for operating motor (below)
  
  // set direction of motor based on sign of MtrSpeed var(s)
  digitalWrite(leftMtrDirectionPin, (leftMtrSpeed) >= 0? HIGH : LOW );
  digitalWrite(rightMtrDirectionPin, (rightMtrSpeed) >= 0? HIGH : LOW);
  
  // set pwm for each motor, and send pulse to pin(s)
  InitPulse(leftMtrStepPin, map(abs(leftMtrSpeed), 0, 100, 10, 1000));
  InitPulse(rightMtrStepPin, map(abs(rightMtrSpeed), 0, 100, 10, 1000));
  if(IsPulseFinished()){
    Pulse(1); // non-blocking, expect timing to be off (Todo)
  }
}

// Start specified timer
void StartTimer(int timer, unsigned long time){
  TMRArd_InitTimer(timer, time);
}

// Return whether specified timer has expired or not
unsigned char IsTimerExpired(int timer){
  return (unsigned char)(TMRArd_IsTimerExpired(timer));
}

// Set motors to constant forward speed
void MoveForward(){
  SetLeftRightMotorSpeed(MTR_SPEED, MTR_SPEED);
}

// Set motors to constant reverse speed
void MoveReverse(){
  SetLeftRightMotorSpeed(-MTR_SPEED, -MTR_SPEED);
}

// Stop motors
void StopMoving(){
  SetLeftRightMotorSpeed(0, 0);
}

void CollectEnvInfo(){
  ReadTapeSensors();
  Check1kHzBeaconDetector();
  Check5kHzBeaconDetector();
}

// Get tape readings for frontRowTape and backRowTape sensor rows
void ReadTapeSensors(){
  // Todo:
  // Get pinout and reading code
}

// Get reading from 1kHz beacon-detector circuit
void Check1kHzBeaconDetector(){
  // Todo:
  // Get pinout and reading code
}

// Get reading from 5kHz beacon-detector circuit
void Check5kHzBeaconDetector(){
  // Todo:
  // Get pinout and reading code
}

void FollowLine(){
  
  // Todo:
  // Implement tape-reading cases and motor turning + timer (?-unsure)
}
//=======================================================================

