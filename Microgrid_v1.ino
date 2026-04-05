/*
  Integrated Fault Detection + Servo Control
  -------------------------------------------
  Monitors three digital fault inputs: Zone 1a (pin 12), Zone 1b (pin 11), Zone 2 (pin 13).
  On fault: corresponding servo slowly moves to FAULT_ANGLE.
  On clear: servo slowly returns to HOME_ANGLE.
  Serial commands move the servo incrementally to a target angle.

  Wiring:
    Fault Input Zone 1a          → Pin 12  (INPUT_PULLUP; short to GND to trigger)
    Fault Input Zone 1b          → Pin 11  (INPUT_PULLUP; short to GND to trigger)
    Fault Input Zone 2           → Pin 13  (INPUT_PULLUP; short to GND to trigger)
    Servo 1a Signal              → Pin 9
    Servo 1b Signal              → Pin 10
    Servo 2  Signal              → Pin 6
    Servo Power  (red)           → 5V
    Servo Ground (brown/black)   → GND

  Serial Commands:
    0–90 : Move to angle
    r    : Return to home (0 deg)
    c    : Move to center (45 deg)
*/

#include <Servo.h>

#define Pole_1 13 // Between sub and islanding point
#define Pole_2 3 // Between sub and islanding point
#define Pole_3 4 // Islanding pole
#define Pole_4 5 // BESS pole
#define Pole_5 6 // End of zone 1
#define Pole_6 7 // Zone 2
#define WTP 9 // Water treatment plant
#define House_1 8 // Misc load 1
#define House_2 10 // Misc load 2
#define House_3 11 // Residential area
#define House_4 12 // Residential area
#define House_5 2 // Residential area

// ── Pin assignments ────────────────────────────────────────────────
const int Switch_1 = A0;
const int Switch_2 = A1;
const int Switch_3  = A2;
const int Servo_1 = A4;
const int Servo_2 = A3;
const int Servo_3  = A5;

// ── Servo parameters ───────────────────────────────────────────────
const int MIN_ANGLE   = 0;
const int MAX_ANGLE   = 90;
const int FAULT_ANGLE = 30;
const int HOME_ANGLE  = 0;
const int STEP_DELAY  = 5;   // ms between each 1° step — increase to slow down further

// ── State variables ────────────────────────────────────────────────
Servo servoTree1a;
Servo servoTree1b;
Servo servoTree2;

int  currentAngle1a = HOME_ANGLE;
int  targetAngle1a  = HOME_ANGLE;
int  currentAngle1b = HOME_ANGLE;
int  targetAngle1b  = HOME_ANGLE;
int  currentAngle2  = HOME_ANGLE;
int  targetAngle2   = HOME_ANGLE;

bool faultActive1a = false;
bool faultActive1b = false;
bool faultActive2  = false;

// ══════════════════════════════════════════════════════════════════
void setup() {
  
  pinMode(Pole_1, OUTPUT);
  pinMode(Pole_2, OUTPUT);
  pinMode(Pole_3, OUTPUT);
  pinMode(Pole_4, OUTPUT);
  pinMode(Pole_5, OUTPUT);
  pinMode(Pole_6, OUTPUT);
  pinMode(WTP, OUTPUT);
  pinMode(House_1, OUTPUT);
  pinMode(House_2, OUTPUT);
  pinMode(House_3, OUTPUT);
  pinMode(House_4, OUTPUT);
  pinMode(House_5, OUTPUT);

  // inital state, loads are powered by the substation
  digitalWrite(Pole_1, HIGH);
  digitalWrite(Pole_2, HIGH);
  digitalWrite(Pole_3, HIGH);
  digitalWrite(Pole_4, LOW);
  digitalWrite(Pole_5, HIGH);
  digitalWrite(Pole_6, HIGH);
  digitalWrite(WTP, HIGH);
  digitalWrite(House_1, HIGH);
  digitalWrite(House_2, HIGH);
  digitalWrite(House_3, HIGH);
  digitalWrite(House_4, HIGH);
  digitalWrite(House_5, HIGH);
  
  pinMode(Switch_1, INPUT_PULLUP);
  pinMode(Switch_2, INPUT_PULLUP);
  pinMode(Switch_3,  INPUT_PULLUP);

  servoTree1a.attach(Servo_2);
  servoTree1a.write(HOME_ANGLE);

  servoTree1b.attach(Servo_1);
  servoTree1b.write(HOME_ANGLE);

  servoTree2.attach(Servo_3);
  servoTree2.write(HOME_ANGLE);

}

// ══════════════════════════════════════════════════════════════════
void loop() {
  checkFault();
  stepTowardsTarget();
  delay(10);
}

// ══════════════════════════════════════════════════════════════════
// Moves one degree toward targetAngle each call
// ══════════════════════════════════════════════════════════════════
void stepTowardsTarget() {
  bool moved = false;

  if (currentAngle1a != targetAngle1a) {
    currentAngle1a += (currentAngle1a < targetAngle1a) ? 1 : -1;
    servoTree1a.write(currentAngle1a);
    moved = true;
  }

  if (currentAngle1b != targetAngle1b) {
    currentAngle1b += (currentAngle1b < targetAngle1b) ? 1 : -1;
    servoTree1b.write(currentAngle1b);
    moved = true;
  }

  if (currentAngle2 != targetAngle2) {
    currentAngle2 += (currentAngle2 < targetAngle2) ? 1 : -1;
    servoTree2.write(currentAngle2);
    moved = true;
  }

  if (moved) delay(STEP_DELAY);
}

// ══════════════════════════════════════════════════════════════════
// Fault detection — trips or clears servo targets
// ══════════════════════════════════════════════════════════════════
void checkFault() {
  bool pinFaulted1a = (digitalRead(Switch_1) == LOW);
  bool pinFaulted1b = (digitalRead(Switch_2) == LOW);
  bool pinFaulted2  = (digitalRead(Switch_3)  == LOW);

  // ── Zone 1a ──
  if (pinFaulted1a && !faultActive1a) {  
    faultActive1a = true;
    targetAngle1a = FAULT_ANGLE;
    // Microgrid in islanded operation
    delay(1000);
    digitalWrite(Pole_1, LOW);
    digitalWrite(Pole_2, LOW);
    digitalWrite(Pole_3, HIGH);
    digitalWrite(Pole_4, HIGH);
    digitalWrite(Pole_5, HIGH);
    digitalWrite(Pole_6, HIGH);
    digitalWrite(WTP, HIGH);
    digitalWrite(House_1, HIGH);
    digitalWrite(House_2, HIGH);
    digitalWrite(House_3, HIGH);
    digitalWrite(House_4, HIGH);
    digitalWrite(House_5, HIGH);
  } else if (!pinFaulted1a && faultActive1a) {
    faultActive1a = false;
    targetAngle1a = HOME_ANGLE;
  }

  // ── Zone 1b ──
  if (pinFaulted1b && !faultActive1b) {
    faultActive1b = true;
    targetAngle1b = FAULT_ANGLE;
    delay(1000);
    digitalWrite(Pole_1, LOW);
    digitalWrite(Pole_2, LOW);
    digitalWrite(Pole_3, LOW);
    digitalWrite(Pole_4, LOW);
    digitalWrite(Pole_5, LOW);
    digitalWrite(Pole_6, LOW);
    digitalWrite(WTP, LOW);
    digitalWrite(House_1, LOW);
    digitalWrite(House_2, LOW);
    digitalWrite(House_3, LOW);
    digitalWrite(House_4, LOW);
    digitalWrite(House_5, LOW);
  } else if (!pinFaulted1b && faultActive1b) {
    faultActive1b = false;
    targetAngle1b = HOME_ANGLE;
  }

  // ── Zone 2 ──
  if (pinFaulted2 && !faultActive2) {
    faultActive2 = true;
    targetAngle2 = FAULT_ANGLE;
    delay(1000);
    digitalWrite(Pole_1, LOW);
    digitalWrite(Pole_2, LOW);
    digitalWrite(Pole_3, HIGH);
    digitalWrite(Pole_4, HIGH);
    digitalWrite(Pole_5, HIGH);
    digitalWrite(Pole_6, LOW);
    digitalWrite(WTP, HIGH);
    digitalWrite(House_1, HIGH);
    digitalWrite(House_2, HIGH);
    digitalWrite(House_3, LOW);
    digitalWrite(House_4, LOW);
    digitalWrite(House_5, LOW);
  } else if (!pinFaulted2 && faultActive2) {
    faultActive2 = false;
    targetAngle2 = HOME_ANGLE;
  }
}




