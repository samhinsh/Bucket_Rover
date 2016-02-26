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
#include "Pulse.h"

/*---------------Module Defines-----------------------------*/
#define LIGHT_THRESHOLD    350 // smaller at night
#define FENCE_THRESHOLD    700
#define ONE_SEC            1000
#define THREE_SEC          3000
#define TEN_SEC            10000
#define TWO_MIN            120000
#define MTR_SPEED          100
 
/*---------------Function Prototypes-------------------------*/
// General Helpers
void FollowLine();
void MoveForward();
void MoveReverse();
void StopMoving();
void SetLeftRightMotorSpeed(int leftMtrSpeed, int rightMtrSpeed);

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


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}

//=======================================================================
// Functions Needing Testing

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
//=======================================================================
