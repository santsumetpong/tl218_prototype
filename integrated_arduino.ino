/*
  Integrated Fault Detection + Servo Control
  -------------------------------------------
  Monitors a digital fault input (Zone 1, pin 12).
  On fault: servo slowly moves to FAULT_ANGLE.
  On clear: servo slowly returns to HOME_ANGLE.
  Serial commands move the servo incrementally to a target angle.

  Wiring:
    Fault Input (Zone 1)         → Pin 12  (INPUT_PULLUP; short to GND to trigger)
    Servo Signal (orange/yellow) → Pin 9
    Servo Power  (red)           → 5V
    Servo Ground (brown/black)   → GND

  Serial Commands:
    0–90 : Move to angle
    r    : Return to home (0 deg)
    c    : Move to center (45 deg)
*/

#include <Servo.h>

// ── Pin assignments ────────────────────────────────────────────────
const int ZONE1_PIN  = 12;
const int SERVO_PIN  = 9;

// ── Servo parameters ───────────────────────────────────────────────
const int MIN_ANGLE   = 0;
const int MAX_ANGLE   = 90;
const int FAULT_ANGLE = 90;
const int HOME_ANGLE  = 0;
const int STEP_DELAY  = 30;   // ms between each 1° step — increase to slow down further

// ── State variables ────────────────────────────────────────────────
Servo myServo;
int   currentAngle = HOME_ANGLE;
int   targetAngle  = HOME_ANGLE;
bool  faultActive  = false;

// ══════════════════════════════════════════════════════════════════
void setup() {
  pinMode(ZONE1_PIN, INPUT_PULLUP);
  Serial.begin(9600);

  myServo.attach(SERVO_PIN);
  myServo.write(HOME_ANGLE);

  Serial.println("=== Fault Detection + Servo Control ===");
  Serial.println("Zone 1 monitoring active (Pin 12).");
  Serial.println("Commands:");
  Serial.println("  0-90 : Move to angle");
  Serial.println("  r    : Return to home (0 deg)");
  Serial.println("  c    : Move to center (45 deg)");
  Serial.println("=======================================");
}

// ══════════════════════════════════════════════════════════════════
void loop() {
  checkFault();
  if (!faultActive) handleSerial();
  stepTowardsTarget();
  delay(50);
}

// ══════════════════════════════════════════════════════════════════
// Moves one degree toward targetAngle each call
// ══════════════════════════════════════════════════════════════════
void stepTowardsTarget() {
  if (currentAngle == targetAngle) return;

  if (currentAngle < targetAngle) {
    currentAngle++;
  } else {
    currentAngle--;
  }

  myServo.write(currentAngle);
  Serial.print("Angle: ");
  Serial.println(currentAngle);
  delay(STEP_DELAY);
}

// ══════════════════════════════════════════════════════════════════
// Fault detection — trips or clears servo target
// ══════════════════════════════════════════════════════════════════
void checkFault() {
  bool pinFaulted = (digitalRead(ZONE1_PIN) == LOW);

  if (pinFaulted && !faultActive) {
    faultActive = true;
    targetAngle = FAULT_ANGLE;
    Serial.println("1");
    Serial.println("*** FAULT — Zone 1 tripped. Moving to fault position. ***");

  } else if (!pinFaulted && faultActive) {
    faultActive = false;
    targetAngle = HOME_ANGLE;
    Serial.println("0");
    Serial.println("*** CLEARED — Zone 1 normal. Returning to home. ***");
  }
}

// ══════════════════════════════════════════════════════════════════
// Serial command handler
// ══════════════════════════════════════════════════════════════════
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
