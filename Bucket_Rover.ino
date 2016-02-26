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
 * Thu Feb 25 - Added function prototypes, fleshed out DropOffTokensThenReload()
 */
 
/*---------------Includes-----------------------------------*/
#include <Timers.h>

/*---------------Module Defines-----------------------------*/
#define LIGHT_THRESHOLD    350 // smaller at night
#define FENCE_THRESHOLD    700
#define ONE_SEC            1000
#define THREE_SEC          3000
#define TEN_SEC            10000
#define MTR_SPEED          100
 
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

// DropOffTokensThenReload Helpers
void GoToBucket();
void DropOffTokens();
void BackupToReloadStation();
void Reload();

// General Helpers
void FollowLine();
void StartTimer(int timer, unsigned long time);
unsigned char IsTimerExpired(int timer);


//=======================================================================
// State Enumerations - for bot activity at any given time

// Prepended "s" indicates State type variable
enum State {
  
  // TravelToCenterLine sub-routine states
  sFindingReloadBeacon, sGoingToReloadLine, sRightOnLine1, sFollowingLine1, sLeftOnLine2, 
  sFollowingLine2, sRightOnLine2,
   
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
  tAll,            // left, center, and right detecting line
  tNone,           // no sensors detecting line
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
// Timer Enums - for utilizing timing various actions

// Indicates timer type
enum Timer {
  
  Rotate_Timer,  // timer for making quick, small angles of rotation
  DropOff_Timer, // token drop off timer
  Reload_Timer,  // token reload timer
  Competition_Timer
};
//=======================================================================

//=======================================================================
// Pins - Physical pinout of circuitry

// Unassigned
const int frontCenterTape = 0;  // front row tape
const int middleCenterTape = 0; // middle row tape
const int middleLeftTape = 0;
const int middleRightTape = 0;
const int beaconDetector = 0;   // IR detection circuit
//=======================================================================

//=======================================================================
// State and Environment variables

State state = sFindingReloadBeacon;  // bot state
TapeActivity frontRowTape = tUndefined;  // front row tape sensors
TapeActivity middleRowTape = tUndefined; // middle row tapes
BeaconStat beacon_1kHz = bUndetected; // 1kHz beacon detection status
BeaconStat beacon_5kHz = bUndetected; // 5kHz beacon detection status
//=======================================================================

// Initialization (one time, setup) stuff
void setup() {
  // Init pins
  // Init competition timer
}

// Stuff done many times 
void loop() {
  // if competition timer expired
    // do nothing
  
  // Collect real-time information about the environment
  CollectEnvInfo();
  
  // One-time-accomplishable sub-routine for finding the 5kHz reload box 
  // from the right side and traveling to the center line of the stadium.
  // Can be entered into at various states
  if(true){
    TravelToCenterLine();
  } 
  
  // Sub-routine for traveling to the buckets, dropping off tokens
  // and returning to the reload station. To be accomplished many times. 
  // Can be entered into at various states
  else {
    DropOffTokensThenReload();
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

/*=======================================================================
 * Sub-routine for finding the 5kHz reload box from the right side and 
 * traveling to the center line of the stadium.
 * Applicable enter states: 
     sFindingReloadBeacon - rotate (then set small I_timer) until 5kHz signal
                            found via IR detection
     sGoingToReloadLine   - move forward until line detected in front
     sRightOnLine1        - rotate to the right for small duration (R_timer)
     sFollowingLine1      - follow line, TBD
     sLeftOnLine2         - TBD, need condition for turning left (or new states
                            for simply detecting front && right tape sensors at
                            "crossroad" of center line, then turning right)
     sFollowingLine2      - TBD
     sRightOnLine2        - TBD
========================================================================*/
void TravelToCenterLine(){
  // Todo:
  // Abstract out necessary states
}

/*----------TravelToCenterLine Helpers----------*/

// Todo:
// Abstract out helpers

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
    if(middleRowTape != tAll){ // middle not sitting on crossroad in front of buckets
      GoToBucket(); // move forward to bucket (line follow)
    } else { // middleRowTape sitting on crossroad
      DropOffTokens(); // start dropping off tokens
    }
  }
  
  // in, or leaving sDroppingOffTokens
  else if(state == sDroppingOffTokens){
    if(IsTimerExpired(DropOff_Timer)){ // done dropping off tokens (timer expired)
      // move on to sBackingUpToReload
      BackupToReloadStation();
    }  
    // otherwise, continue dropping off tokens (implicitly)
  }
  
  // in, or leaving sBackingUpToReload
  else if(state == sBackingUpToReload){
    if(middleRowTape == tAll){ // hit the reload crossroad from behind
      Reload();
    }
    // otherwise, continue backing up (implictly)
  }
  
  // in, or leaving sReloading
  else if (state == sReloading){
     if(IsTimerExpired(Reload_Timer)){ // reloading-timer finished
       GoToBucket(); // repeat
     }
     // otherwise, continue reloading (idly)
  }
}

/*----------DropOffTokensThenReload Helpers----------*/

// Go to the bucket
void GoToBucket(){
  state == sGoingToBucket;
  
  // Todo:
  // start motors moving forward (likely, if not handled by FollowLine)
  
  // change direction slightly (with motors) if off line (off center tape sensor)
  FollowLine();
}

// Drop off tokens by turning on servos & starting

void DropOffTokens(){
  state == sDroppingOffTokens;
  
  // Todo:
  // stop motors
  
  // start drop-off mechanism timer
  if(IsTimerExpired(DropOff_Timer)){
    StartTimer(DropOff_Timer, THREE_SEC); // 3-second timer
  }
  
  // Todo:
  // turn drop-off servo(s) on
}

// Back the bot up to the reload station
void BackupToReloadStation(){
  state == sBackingUpToReload;
  
  // Todo:
  // reverse motors for backing up (no line following)
}

// Remain idle at reload station while user loads tokens
void Reload(){
  state == sReloading;
  
  // Todo:
  // stop motors, remain idle
  
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

// Start specified timer
void StartTimer(int timer, unsigned long time){
  TMRArd_InitTimer(timer, time);
}

// Return whether timer has expired or not
unsigned char IsTimerExpired(int timer){
  return (unsigned char)(TMRArd_IsTimerExpired(timer));
}

