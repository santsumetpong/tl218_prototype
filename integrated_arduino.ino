/*
  Integrated Fault Detection + Servo Control
  -------------------------------------------
  Monitors a digital fault input (Zone 1, pin 12).
  On fault: servo trips to FAULT_ANGLE and locks out sweep/manual control.
  On clear: servo returns to home and resumes normal operation.
  Manual serial commands are accepted only when no fault is active.

  Wiring:
    Fault Input (Zone 1)         → Pin 12  (use INPUT_PULLUP; short to GND to trigger)
    Servo Signal (orange/yellow) → Pin 9
    Servo Power  (red)           → 5V
    Servo Ground (brown/black)   → GND

  Serial Commands (fault-free only):
    0–90  : Move to angle
    s     : Toggle auto-sweep on/off
    c     : Move to center (45 degrees)
    r     : Reset/return to home (0 degrees)

  Compatible: Arduino Uno, Nano, Mega
*/

#include <Servo.h>

// ── Pin assignments ────────────────────────────────────────────────
const int ZONE1_PIN  = 12;   // Fault input (INPUT_PULLUP; LOW = fault)
const int SERVO_PIN  = 9;

// ── Servo parameters ───────────────────────────────────────────────
const int SWEEP_DELAY  = 15;  // ms per degree during auto-sweep
const int MIN_ANGLE    = 0;
const int MAX_ANGLE    = 90;
const int FAULT_ANGLE  = 90;  // Position servo moves to on trip
const int HOME_ANGLE   = 0;   // Position on power-up and fault clear

// ── State variables ────────────────────────────────────────────────
Servo    myServo;
int      currentAngle = HOME_ANGLE;
bool     sweepMode    = true;   // true = auto-sweep
bool     faultActive  = false;  // tracks latched fault state

// ══════════════════════════════════════════════════════════════════
void setup() {
  pinMode(ZONE1_PIN, INPUT_PULLUP);
  Serial.begin(9600);

  myServo.attach(SERVO_PIN);
  moveToAngle(HOME_ANGLE);

  Serial.println("=== Fault Detection + Servo Control ===");
  Serial.println("Zone 1 monitoring active (Pin 12).");
  Serial.println("Commands (inactive during fault):");
  Serial.println("  0-90 : Move to angle");
  Serial.println("  s    : Toggle auto-sweep");
  Serial.println("  c    : Move to center (45 deg)");
  Serial.println("  r    : Return to home (0 deg)");
  Serial.println("=======================================");
}

// ══════════════════════════════════════════════════════════════════
void loop() {
  checkFault();

  // Block all manual/sweep control while fault is active
  if (faultActive) {
    delay(50);
    return;
  }

  // ── Serial input (fault-free only) ──────────────────────────────
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input == "s") {
      sweepMode = !sweepMode;
      Serial.print("Auto-sweep: ");
      Serial.println(sweepMode ? "ON" : "OFF");

    } else if (input == "c") {
      sweepMode = false;
      moveToAngle(45);

    } else if (input == "r") {
      sweepMode = false;
      moveToAngle(HOME_ANGLE);

    } else {
      int angle = input.toInt();
      if (angle >= MIN_ANGLE && angle <= MAX_ANGLE) {
        sweepMode = false;
        moveToAngle(angle);
      } else {
        Serial.println("Invalid input. Enter 0-90, 's', 'c', or 'r'.");
      }
    }
  }

  // ── Auto-sweep ──────────────────────────────────────────────────
  if (sweepMode) {
    for (int angle = MIN_ANGLE; angle <= MAX_ANGLE; angle++) {
      if (Serial.available() || faultDetected()) break;
      myServo.write(angle);
      currentAngle = angle;
      printAngle(angle);
      delay(SWEEP_DELAY);
    }
    for (int angle = MAX_ANGLE; angle >= MIN_ANGLE; angle--) {
      if (Serial.available() || faultDetected()) break;
      myServo.write(angle);
      currentAngle = angle;
      printAngle(angle);
      delay(SWEEP_DELAY);
    }
  }
}

// ══════════════════════════════════════════════════════════════════
// Fault detection — reads pin, manages state transitions, trips servo
// ══════════════════════════════════════════════════════════════════
void checkFault() {
  bool pinFaulted = (digitalRead(ZONE1_PIN) == LOW);

  if (pinFaulted && !faultActive) {
    // Rising edge: new fault
    faultActive = true;
    Serial.println("1");   // preserve original protocol output
    Serial.println("*** FAULT DETECTED — Zone 1 tripped! Servo moving to FAULT position. ***");
    moveToAngle(FAULT_ANGLE);
    sweepMode = false;

  } else if (!pinFaulted && faultActive) {
    // Falling edge: fault cleared
    faultActive = false;
    Serial.println("0");   // preserve original protocol output
    Serial.println("*** FAULT CLEARED — Zone 1 normal. Returning to home. ***");
    moveToAngle(HOME_ANGLE);
    sweepMode = true;      // resume sweep after clear; change to false if preferred
  }

  delay(50);  // debounce (matches original sketch timing)
}

// Returns true if fault pin is currently asserted (used inside sweep loops)
bool faultDetected() {
  return (digitalRead(ZONE1_PIN) == LOW);
}

// ══════════════════════════════════════════════════════════════════
void moveToAngle(int angle) {
  currentAngle = angle;
  myServo.write(angle);
  Serial.print("Moved to: ");
  Serial.print(angle);
  Serial.println(" deg");
}

void printAngle(int angle) {
  if (angle % 10 == 0) {
    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.print("  [");
    int bars = angle / 10;
    for (int i = 0; i < 9; i++) {
      Serial.print(i < bars ? "=" : " ");
    }
    Serial.println("]");
  }
}
