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
 */

// Initialization (one time, setup) stuff
void setup() {
  // put your setup code here, to run once:

}

// Stuff done many times 
void loop() {
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

/* Collect real-time information about the environment.
 * This info will be stored in global variables toward the beginning of 
 * this file, and used in later sub-routines. 
 */
void CollectEnvInfo(){}

/* Sub-routine for finding the 5kHz reload box from the right side and 
 * traveling to the center line of the stadium.
 * Applicable enter states: 
 *   FR (find reload beacon)       - 
     GR (go to reload line)        -
     R1 (turn right onto 1st line) -
     FL1 (follow 1st line)         - 
     L (turn left onto 2nd line)   - 
     FL2 (follow 2nd line)         -
     R2 (trn right on center line) - 
 */
void TravelToCenterLine(){}

/* Sub-routine for traveling to the buckets, dropping off tokens and
 * returning to the reload station.
 * Applicable enter states: 
 *   GB (go to bucket)           -
     D (drop off chips)          -
     B (backup to reload statn.) -
     RLD (reload tokens)         -
 */
void DropOffTokensThenReload(){}
