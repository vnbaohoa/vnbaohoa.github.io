#include <Servo.h>

// ---------------- SERVO PINS ----------------
const int LEFT_SERVO_PIN  = 13;
const int RIGHT_SERVO_PIN = 12;

// ---------------- SERVO MODEL ----------------
// Left forward: 1500 -> 1300 (smaller = faster)
// Right forward: 1500 -> 1700 (larger = faster)
const int SERVO_MIN  = 1300;
const int SERVO_STOP = 1500;
const int SERVO_MAX  = 1700;

// Spin speed
const int SPIN_SPEED_OFFSET = 80;   // TUNE speed

Servo leftServo;
Servo rightServo;

// ------------- BASIC DRIVE HELPERS -------------

void driveRaw(int leftPulse, int rightPulse) {
  leftPulse  = constrain(leftPulse,  SERVO_MIN, SERVO_MAX);
  rightPulse = constrain(rightPulse, SERVO_MIN, SERVO_MAX);
  leftServo.writeMicroseconds(leftPulse);
  rightServo.writeMicroseconds(rightPulse);
}

void stopMotors() {
  leftServo.writeMicroseconds(SERVO_STOP);
  rightServo.writeMicroseconds(SERVO_STOP);
}

// ------------- SERIAL LINE READER -------------
// Read one full line (until ENTER). Works with "Newline" or "Both NL & CR".

String readSerialLine() {
  String s = "";
  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        // If we have any chars collected, return the line
        if (s.length() > 0) {
          return s;
        }
        // If s is empty, it was just an "empty ENTER" -> return empty string
        return "";
      } else {
        s += c;
      }
    }
  }
}

// ------------- TUNING HELPERS -------------

// Wait for user to press ENTER (empty line)
void waitForSecondEnter() {
  while (true) {
    if (Serial.available()) {
      String line = readSerialLine();
      // empty line -> just ENTER
      if (line.length() == 0) {
        return;
      }
      // if user typed something else, ignore and keep waiting
    }
  }
}

// These four functions implement:
// 1) Start moving
// 2) Keep moving until you press ENTER
// 3) Stop and print elapsed time

void tuneSpinLeft90() {
  Serial.println("\nTUNE: SPIN LEFT 90°");
  Serial.println("Now spinning. When robot reaches 90°, press ENTER.");
  delay(1000);

  unsigned long start = millis();

  while (true) {
    // Spin left: left backward (>1500), right forward (>1500 with your wiring)
    int leftPulse  = SERVO_STOP + SPIN_SPEED_OFFSET;
    int rightPulse = SERVO_STOP + SPIN_SPEED_OFFSET;
    driveRaw(leftPulse, rightPulse);

    if (Serial.available()) {
      String line = readSerialLine();
      if (line.length() == 0) {
        // empty ENTER -> stop
        break;
      }
      // else ignore
    }
  }

  stopMotors();
  unsigned long dt = millis() - start;
  Serial.print("RESULT: SPIN LEFT 90° time = ");
  Serial.print(dt);
  Serial.println(" ms");
}

void tuneSpinLeft180() {
  Serial.println("\nTUNE: SPIN LEFT 180°");
  Serial.println("Now spinning. When robot completes 180°, press ENTER.");
  delay(1000);

  unsigned long start = millis();

  while (true) {
    int leftPulse  = SERVO_STOP + SPIN_SPEED_OFFSET;
    int rightPulse = SERVO_STOP + SPIN_SPEED_OFFSET;
    driveRaw(leftPulse, rightPulse);

    if (Serial.available()) {
      String line = readSerialLine();
      if (line.length() == 0) {
        break;
      }
    }
  }

  stopMotors();
  unsigned long dt = millis() - start;
  Serial.print("RESULT: SPIN LEFT 180° time = ");
  Serial.print(dt);
  Serial.println(" ms");
}

void tuneForward5in() {
  Serial.println("\nTUNE: FORWARD 5 inches");
  Serial.println("Now moving forward. When robot has moved 5 inches, press ENTER.");
  delay(1000);

  unsigned long start = millis();

  while (true) {
    // Forward: left <1500, right >1500
    int leftPulse  = 1400; // pick a forward speed
    int rightPulse = 1600;
    driveRaw(leftPulse, rightPulse);

    if (Serial.available()) {
      String line = readSerialLine();
      if (line.length() == 0) {
        break;
      }
    }
  }

  stopMotors();
  unsigned long dt = millis() - start;
  Serial.print("RESULT: FORWARD 5 inches time = ");
  Serial.print(dt);
  Serial.println(" ms");
}

void tuneBackward5in() {
  Serial.println("\nTUNE: BACKWARD 5 inches");
  Serial.println("Now moving backward. When robot has moved 5 inches back, press ENTER.");
  delay(1000);

  unsigned long start = millis();

  while (true) {
    // Backward mirror: left >1500, right <1500
    int leftPulse  = 1600;
    int rightPulse = 1400;
    driveRaw(leftPulse, rightPulse);

    if (Serial.available()) {
      String line = readSerialLine();
      if (line.length() == 0) {
        break;
      }
    }
  }

  stopMotors();
  unsigned long dt = millis() - start;
  Serial.print("RESULT: BACKWARD 5 inches time = ");
  Serial.print(dt);
  Serial.println(" ms");
}

// ------------- SETUP & LOOP -------------

void setup() {
  Serial.begin(9600);
  leftServo.attach(LEFT_SERVO_PIN);
  rightServo.attach(RIGHT_SERVO_PIN);
  stopMotors();
  delay(1000);

  Serial.println("=== DRIVE TUNING PROGRAM ===");
  Serial.println("Type command, press ENTER, then when motion is correct, press ENTER again:");
  Serial.println("  1 : Tune SPIN LEFT 90°");
  Serial.println("  2 : Tune SPIN LEFT 180°");
  Serial.println("  3 : Tune FORWARD 5 inches");
  Serial.println("  4 : Tune BACKWARD 5 inches");
  Serial.println();
  Serial.println("Example: type 1, press ENTER -> robot spins; when at 90°, press ENTER again.");
}

void loop() {
  if (Serial.available()) {
    String cmd = readSerialLine();  // waits until you press ENTER

    if (cmd == "1") {
      tuneSpinLeft90();
    }
    else if (cmd == "2") {
      tuneSpinLeft180();
    }
    else if (cmd == "3") {
      tuneForward5in();
    }
    else if (cmd == "4") {
      tuneBackward5in();
    }
    else if (cmd.length() == 0) {
      // Just ENTER while idle -> do nothing
    }
    else {
      Serial.print("Unknown command: ");
      Serial.println(cmd);
    }

    Serial.println("\nReady for next command (1–4).");
  }
}