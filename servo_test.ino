/*
  Servo Motor Test
  ----------------
  Sweeps a servo from 0° to 180° and back continuously.
  Also listens for Serial input to manually set the angle.

  Wiring:
    Servo Signal (orange/yellow) → Pin 9
    Servo Power  (red)           → 5V
    Servo Ground (brown/black)   → GND

  Compatible: Arduino Uno, Nano, Mega
*/

#include <Servo.h>

Servo myServo;

const int SERVO_PIN   = 9;
const int SWEEP_DELAY = 15;   // ms between each degree step
const int MIN_ANGLE   = 0;
const int MAX_ANGLE   = 90;

int currentAngle = 0;
bool sweepMode   = true;      // true = auto-sweep, false = manual via Serial

void setup() {
  Serial.begin(9600);
  myServo.attach(SERVO_PIN);
  myServo.write(currentAngle);

  Serial.println("=== Servo Motor Test ===");
  Serial.println("Commands:");
  Serial.println("  0-180  : Move to angle (e.g. type '90' and Enter)");
  Serial.println("  s      : Toggle auto-sweep on/off");
  Serial.println("  c      : Move to center (90 degrees)");
  Serial.println("========================");
}

void loop() {
  // Handle Serial input
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input == "s") {
      sweepMode = !sweepMode;
      Serial.print("Auto-sweep: ");
      Serial.println(sweepMode ? "ON" : "OFF");
    } else if (input == "c") {
      sweepMode = false;
      moveToAngle(90);
    } else {
      int angle = input.toInt();
      if (angle >= MIN_ANGLE && angle <= MAX_ANGLE) {
        sweepMode = false;
        moveToAngle(angle);
      } else {
        Serial.println("Invalid input. Enter 0-180, 's', or 'c'.");
      }
    }
  }

  // Auto-sweep mode
  if (sweepMode) {
    // Sweep 0 -> 180
    for (int angle = MIN_ANGLE; angle <= MAX_ANGLE; angle++) {
      if (Serial.available()) break;   // bail out if user sends a command
      myServo.write(angle);
      printAngle(angle);
      delay(SWEEP_DELAY);
    }
    // Sweep 180 -> 0
    for (int angle = MAX_ANGLE; angle >= MIN_ANGLE; angle--) {
      if (Serial.available()) break;
      myServo.write(angle);
      printAngle(angle);
      delay(SWEEP_DELAY);
    }
  }
}

void moveToAngle(int angle) {
  currentAngle = angle;
  myServo.write(angle);
  Serial.print("Moved to: ");
  Serial.print(angle);
  Serial.println(" degrees");
}

void printAngle(int angle) {
  // Print progress bar every 10 degrees
  if (angle % 10 == 0) {
    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.print("°  [");
    int bars = angle / 10;
    for (int i = 0; i < 18; i++) {
      Serial.print(i < bars ? "=" : " ");
    }
    Serial.println("]");
  }
}
