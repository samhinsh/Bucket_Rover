/* ME 210 Final Project, Winter 2016
 * Team: Andrew Anderson, Taylor Clayton, Mario Chapa, Sam Hinshelwood
 *
 * Goal:
 * The goal of this project is to design, build, and code an autonomous bot
 * that will pick up tokens from a designated reload station (identifiable by
 * a 5kHz infrared beacon) and drop them off in at least 3 of 5 drop-off buckets 
 * located across the stadium (and identifiable each via 1kHz IR beacons). 
 *
 * Description: 
 * This file describes a bot equipped with a motor drive board, two DC motors,
 * ground-facing tape sensors, an IR-signal detection circuit (for recognizing 
 * 1kHz and 5kHz signals), a forward-facing phototransistor, an Arduino Uno,
 * servomotor(s) (for operating the coin-drop-off mechanism), and LEDs for debugging.
 *
 * Changelog:
 * Tue Feb 23 - Initial code and comments
 * Wed Feb 24 - Added enums, defines, and CollectEnvInfo helpers
 * Thu Feb 25 - Added function prototypes, fleshed out DropOffTokensThenReload(),
                abstracted movement fn's, added beacon detection logic,
                added tape reading logic
 * Mon Feb 29 - Abstracted TravelToCenterLine() helpers, added Rotate()
 * 
 * Todos: 
 * -Formulate beacon-sensing code
 * -Formulate line-following code
 * -Formulate Rotate()
 * -Test all subsystems 
 * -Place temporary stops in before reversing motor direction
 */
 
/*---------------Includes-----------------------------------*/
#include <Timers.h>
#include <Servo.h>
#include "Pulse.h"

/*---------------Module Defines-----------------------------*/
#define LIGHT_THRESHOLD    350 // smaller at night
#define FENCE_THRESHOLD    700
#define HALF_SEC           500
#define ONE_SEC            1000
#define TWO_SEC            2000
#define THREE_SEC          3000
#define TEN_SEC            10000
#define TWO_MIN            120000
#define MTR_SPEED_REGULAR  85
#define MTR_SPEED_FAST     100

// Todo: Decide if multiple speed defines needed
 
/*---------------Function Prototypes-------------------------*/

// Loop Fn's
void CollectEnvInfo();
void TravelToCenterLine();
void DropOffTokensThenReload();

// CollectEnvInfo Helpers
void ReadTapeSensors();
void Check1kHzBeaconDetector();
void Check5kHzBeaconDetector();

// TravelToCenterLine Helpers
void FindReloadBeacon();
void RotateTowardCenterLine();
void GoToCenterLine();
void CenterOnLine();

// DropOffTokensThenReload Helpers
void GoToBucket();
void DropOffTokens();
void BackupToReloadStation();
void Reload();

// General Helpers
void FollowLine();
void Rotate(char direction, int speed);
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
  sFindingReloadBeacon, sRotatingTowardCenterLine, sGoingToCenterLine, sCenteringOnLine,
   
  // DropOffTokensThenReload
  sGoingToBucket, sDroppingOffTokens, sBackingUpToReload, sReloading 
}; 
//=======================================================================

//=======================================================================
// Tape Activity Enums - for quick summary of tape row readings

// Prepended "t" indicates TapeActivity type variable
// Indicates on-status/activity of  tape sensors in a single row
enum TapeActivity {
  
  tLeftAndCenter,  // left and center sensors detecting line, and so on..
  tCenterAndRight,
  tLeftAndRight,
  tCenter,         // only center tape detecting line
  tLeft,
  tRight,
  tAll,            // left, center, and right detecting line
  tNone,           // no sensors in set detecting line
  tUndefined
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
  
  Rotate_Timer,     // timer for making quick, small angles of rotation
  DropOff_Timer,    // token drop off timer
  Reload_Timer,     // token reload timer
  CenterLine_Timer, // timer for rotating toward the center stadium line
  Competition_Timer // 2-minute all around timer
};
//=======================================================================

//=======================================================================
// Pins - Physical pinout of circuitry

// Unassigned
const int middleCenterTape = 0;     // middle row tape
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
// State and Environment variables

State state = sFindingReloadBeacon;  // bot state
TapeActivity centerTapeSet = tUndefined; // center 3 tape-set
TapeActivity outsideTapeSet = tUndefined; // outside two tape-set
BeaconStat beacon_1kHz = bUndetected; // 1kHz beacon detection status
BeaconStat beacon_5kHz = bUndetected; // 5kHz beacon detection status
Servo servo1; // arm mechanism 
Servo servo2; // arm mechanism
Servo servo3; // arm mechanism
//=======================================================================

// Initialization (one time, setup) stuff
void setup() {
  Serial.begin(9600);
  
  // Init pins
  
  // inputs
  pinMode(middleCenterTape, INPUT);
  pinMode(middleLeftTape, INPUT);
  pinMode(middleRightTape, INPUT);
  pinMode(middleFarLeftTape, INPUT);
  pinMode(middleFarRightTape, INPUT);
  pinMode(beaconDetector, INPUT);
  
  // outputs
  pinMode(leftMtrDirectionPin, OUTPUT);
  pinMode(rightMtrDirectionPin, OUTPUT);
  pinMode(leftMtrEnablePin, OUTPUT);
  pinMode(rightMtrEnablePin, OUTPUT);
  pinMode(LEDpin, OUTPUT);
  
  digitalWrite(leftMtrDirectionPin,LOW); // init motors to same direction
  analogWrite(leftMtrEnablePin,0);
  
  digitalWrite(rightMtrDirectionPin,LOW);
  analogWrite(rightMtrEnablePin,0);
  
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo3.attach(servo3Pin);
  
  // Init competition timer
  StartTimer(Competition_Timer, TWO_MIN);
}

// Stuff done many times 
void loop() {
  
  // if competition timer expired
  if(IsTimerExpired(Competition_Timer)){
    // stop moving, do nothing
  } else { // run the bot
  
    // Collect real-time information about the environment
    CollectEnvInfo();
    
    // Sub-routine for traveling to the buckets, dropping off tokens
    // and returning to the reload station. To be accomplished many times. 
    // Can be entered into at various states
    if(state == sGoingToBucket || state == sDroppingOffTokens || 
       state == sBackingUpToReload || state == sReloading) {
         
         DropOffTokensThenReload();
    }
    
    // One-time-accomplishable sub-routine for finding the 5kHz reload box 
    // in the beginning from the right side and traveling to the center 
    // line of the stadium. Can be entered into at various states
    else {
      
      TravelToCenterLine();
    } 
  }

}

/*=======================================================================
 * Collect real-time information about the environment.
 * This info will be stored in global variables toward the beginning of 
 * this file, and used in later sub-routines. 
 * Information collected:
     Front Row Tape Sensor(s) - placement TBD
     Center Tape Sensor(s)    - placement TBD
     Back Row Tape Sensor(s)  - placement TBD
     1 kHz IR Detector        - Bucket Beacon signal
     5 kHz IR Detector        - Reload Beacon signal
========================================================================*/
void CollectEnvInfo(){
  ReadTapeSensors();
  Check1kHzBeaconDetector();
  Check5kHzBeaconDetector();
}

/*----------CollectEnvInfo Helpers----------*/

// Get tape readings for center and outside tape rows
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

/*=======================================================================
 * Sub-routine for finding the 5kHz reload box from the right side and 
 * traveling to the center line of the stadium.
 * Applicable enter states: 
     sFindingReloadBeacon      - rotate small amount (Rotate_Timer) until 5kHz signal
                                 found via IR detection
     sRotatingTowardCenterLine - rotate (CenterLine_Timer) to face center line
     sGoingToCenterLine        - move forward until line detected (central line)
     sCenteringOnLine          - rotate until center tape only detected on line
========================================================================*/
void TravelToCenterLine(){
  // in, or leaving sFindingReloadBeacon
  if(state == sFindingReloadBeacon){
    if(beacon_5kHz == bUndetected){ // beacon undetected
      if(IsTimerExpired(Rotate_Timer)){ // wait for timer, to rotate again
        FindReloadBeacon();
      }
    } else RotateTowardCenterLine(); // otherwise, move toward center
  }
  
  // in, or leaving sRotatingTowardCenterLine
  else if(state == sRotatingTowardCenterLine){
    if(!IsTimerExpired(CenterLine_Timer)){ // continue rotating toward line if timer still going
      RotateTowardCenterLine();
    } else GoToCenterLine(); // otherwise, head to center line
  }
  
  // in, or leaving sGoingToCenterLine
  else if(state == sGoingToCenterLine){
    if(centerTapeSet == tUndefined || centerTapeSet == tNone){ // no line found
      GoToCenterLine();
    } else CenterOnLine(); // center on line that was found (namely, the center line)
  } 
  
  // in, or leaving sCenteringOnLine
  else if(state == sCenteringOnLine){
    if(centerTapeSet != tCenter){ // center tape sensor not the only one sensing the line
      CenterOnLine(); // continue centering on line
    } else GoToBucket(); // start following the line to the bucket(s)
  } 
  
}

/*----------TravelToCenterLine Helpers----------*/

// Rotate until 5kHz signal detected from reload beacon
void FindReloadBeacon(){
  state = sFindingReloadBeacon;
  
  // rotate left at regular speed
  Rotate('L', rRegular);
  
  // set timer if not started
  if(IsTimerExpired(Rotate_Timer)){
    StartTimer(Rotate_Timer, HALF_SEC);
  }
}

// Rotate predetermined amount (using CenterLine_Timer) toward center line
void RotateTowardCenterLine(){
  state = sRotatingTowardCenterLine;
  
  // rotate rightward toward center line
  Rotate('R', rRegular);
  
  // set timer if not started
  if(IsTimerExpired(CenterLine_Timer)){
    StartTimer(CenterLine_Timer, TWO_SEC); // 2-second timer
    
    // Todo: Fix timing^ based on wheel speed at beacon-detection range
  }
}

// Move forward until hitting center line
void GoToCenterLine(){
  state = sGoingToCenterLine;
  MoveForward();
}

// Center bot on line
void CenterOnLine(){
  state = sCenteringOnLine;
  
  if(centerTapeSet == tLeft){
    MoveForward();
  } else if(centerTapeSet == tLeftAndCenter){
    Rotate('R', rRegular);
  }
  
  // Todo: Test if third case (for right tape) is necessary
}


/*=======================================================================
 * Sub-routine for traveling to the buckets, dropping off tokens and
 * returning to the reload station.
 * Applicable enter states: 
     sGoingToBucket       - follow line until front row detects "crossroad"
     sDroppingOffTokens   - activate servo drop off mechanism (D_timer)
     sBackingUpToReload   - back up until back row detects "crossroad"
     sReloading           - remain idle for reloading (RD_timer)
========================================================================*/
void DropOffTokensThenReload(){
  
  // in, or leaving sGoingToBucket
  if(state == sGoingToBucket){ 
    if(centerTapeSet == tAll){ // middleRowTape sitting on crossroad
      DropOffTokens(); // start dropping off tokens
    } else { // not in front of buckets, keep going to bucket
      GoToBucket(); // move forward to bucket (line follow)
    }
  }
  
  // in, or leaving sDroppingOffTokens
  else if(state == sDroppingOffTokens){
    if(IsTimerExpired(DropOff_Timer)){ // done dropping off tokens (timer expired)
      // move on to sBackingUpToReload
      BackupToReloadStation();
    }  
    // otherwise, continue dropping off tokens (implicitly)
    else DropOffTokens();
  }
  
  // in, or leaving sBackingUpToReload
  else if(state == sBackingUpToReload){
    if(centerTapeSet == tAll){ // hit the reload crossroad from behind
      Reload();
    }
    // otherwise, continue backing up (implictly)
    else BackupToReloadStation();
  }
  
  // in, or leaving sReloading
  else if (state == sReloading){
     if(IsTimerExpired(Reload_Timer)){ // reloading-timer finished
       GoToBucket(); // repeat
     }
     // otherwise, continue reloading (idly)
     else Reload();
  }
}

/*----------DropOffTokensThenReload Helpers----------*/

// Go to the bucket
void GoToBucket(){
  state = sGoingToBucket;
  // change LED indicator
  
  // Todo:
  // start motors moving forward (likely, if not handled by FollowLine)
  MoveForward();
  
  // change direction slightly (with motors) if off line (off center tape sensor)
  FollowLine();
}

// Drop off tokens by turning on servos & starting
// Called continually (Todo)
void DropOffTokens(){
  state = sDroppingOffTokens;
  // change LED indicator
  
  // stop motors
  StopMoving();
  
  // start drop-off mechanism timer
  if(IsTimerExpired(DropOff_Timer)){
    StartTimer(DropOff_Timer, THREE_SEC); // 3-second timer
  }
  
  // Todo:
  // turn drop-off servo(s) on
}

// Back the bot up to the reload station
void BackupToReloadStation(){
  state = sBackingUpToReload;
  // change LED indicator
  
  // reverse motors for backing up (no line following)
  MoveReverse();
}

// Remain idle at reload station while user loads tokens
void Reload(){
  state = sReloading;
  // change LED indicator
  
  // remain idle
  StopMoving();
  
  // start reload timer (change if reload-release switch is desired)
  if(IsTimerExpired(Reload_Timer)){
    StartTimer(Reload_Timer, THREE_SEC); // 5-second timer
  }
}


/*----------General Helpers----------*/

// Line-following algorithm
// Change bot direction slightly by varying left-right motor speed
// to remain centered on middleRowTape's center tape sensor)
void FollowLine(){
  
  // Todo:
  // Implement tape-reading cases and motor turning + timer (?-unsure)
}

// Rotates bot to the left or right (direction), with one of two speeds 
// Inputs: direction - 'L' or 'R'
//         speed     -  rRegular (1) or rFast (2)
void Rotate(char direction, int speed){
  // Todo: 
  // Formulate code for wheel speeds
}

// Set motors to constant forward speed
void MoveForward(){
  SetLeftRightMotorSpeed(MTR_SPEED_REGULAR, MTR_SPEED_REGULAR);
}

// Set motors to constant reverse speed
void MoveReverse(){
  SetLeftRightMotorSpeed(-MTR_SPEED_REGULAR, -MTR_SPEED_REGULAR);
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
  
  analogWrite(leftMtrEnablePin, map(abs(leftMtrSpeed), 0, 100, 0, 150));
  analogWrite(rightMtrEnablePin, map(abs(leftMtrSpeed), 0, 100, 0, 150));
}

// Start specified timer
void StartTimer(int timer, unsigned long time){
  TMRArd_InitTimer(timer, time);
}

// Return whether specified timer has expired or not
unsigned char IsTimerExpired(int timer){
  return (unsigned char)(TMRArd_IsTimerExpired(timer));
}

