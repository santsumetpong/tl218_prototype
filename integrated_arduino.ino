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

// ── Pin assignments ────────────────────────────────────────────────
const int ZONE1A_PIN = A0;
const int ZONE1B_PIN = A1;
const int ZONE2_PIN  = A5;
const int SERVO_PIN1A = A4;
const int SERVO_PIN1B = A3;
const int SERVO_PIN2  = A2;

// ── Servo parameters ───────────────────────────────────────────────
const int MIN_ANGLE   = 0;
const int MAX_ANGLE   = 90;
const int FAULT_ANGLE = 60;
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
  pinMode(ZONE1A_PIN, INPUT_PULLUP);
  pinMode(ZONE1B_PIN, INPUT_PULLUP);
  pinMode(ZONE2_PIN,  INPUT_PULLUP);
  Serial.begin(9600);

  servoTree1a.attach(SERVO_PIN1A);
  servoTree1a.write(HOME_ANGLE);

  servoTree1b.attach(SERVO_PIN1B);
  servoTree1b.write(HOME_ANGLE);

  servoTree2.attach(SERVO_PIN2);
  servoTree2.write(HOME_ANGLE);

  Serial.println("=== Fault Detection + Servo Control ===");
  Serial.println("Zone 1a monitoring active (Pin 12).");
  Serial.println("Zone 1b monitoring active (Pin 11).");
  Serial.println("Zone 2  monitoring active (Pin 13).");
  Serial.println("Commands:");
  Serial.println("  0-90 : Move to angle");
  Serial.println("  r    : Return to home (0 deg)");
  Serial.println("  c    : Move to center (45 deg)");
  Serial.println("=======================================");
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
  bool pinFaulted1a = (digitalRead(ZONE1A_PIN) == LOW);
  bool pinFaulted1b = (digitalRead(ZONE1B_PIN) == LOW);
  bool pinFaulted2  = (digitalRead(ZONE2_PIN)  == LOW);

  // ── Zone 1a ──
  if (pinFaulted1a && !faultActive1a) {
    faultActive1a = true;
    targetAngle1a = FAULT_ANGLE;
    Serial.println("Zone1aFault");
    Serial.println("*** FAULT — Zone 1a tripped. Moving to fault position. ***");
  } else if (!pinFaulted1a && faultActive1a) {
    faultActive1a = false;
    targetAngle1a = HOME_ANGLE;
    Serial.println("Zone1aNoFault");
    Serial.println("*** CLEARED — Zone 1a normal. Returning to home. ***");
  }

  // ── Zone 1b ──
  if (pinFaulted1b && !faultActive1b) {
    faultActive1b = true;
    targetAngle1b = FAULT_ANGLE;
    Serial.println("Zone1bFault");
    Serial.println("*** FAULT — Zone 1b tripped. Moving to fault position. ***");
  } else if (!pinFaulted1b && faultActive1b) {
    faultActive1b = false;
    targetAngle1b = HOME_ANGLE;
    Serial.println("Zone1bNoFault");
    Serial.println("*** CLEARED — Zone 1b normal. Returning to home. ***");
  }

  // ── Zone 2 ──
  if (pinFaulted2 && !faultActive2) {
    faultActive2 = true;
    targetAngle2 = FAULT_ANGLE;
    Serial.println("Zone2Fault");
    Serial.println("*** FAULT — Zone 2 tripped. Moving to fault position. ***");
  } else if (!pinFaulted2 && faultActive2) {
    faultActive2 = false;
    targetAngle2 = HOME_ANGLE;
    Serial.println("Zone2NoFault");
    Serial.println("*** CLEARED — Zone 2 normal. Returning to home. ***");
  }
}

// ══════════════════════════════════════════════════════════════════
// Serial command handler
// ══════════════════════════════════════════════════════════════════
/*
void handleSerial() {
  if (Serial.available() == 0) return;

  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input == "r") {
    targetAngle = HOME_ANGLE;
    Serial.println("Target: home (0 deg)");
  } else if (input == "c") {
    targetAngle = 45;
    Serial.println("Target: center (45 deg)");
  } else {
    int angle = input.toInt();
    if (angle >= MIN_ANGLE && angle <= MAX_ANGLE) {
      targetAngle = angle;
      Serial.print("Target: ");
      Serial.println(angle);
    } else {
      Serial.println("Invalid. Enter 0-90, 'r', or 'c'.");
    }
  }
}
*/
