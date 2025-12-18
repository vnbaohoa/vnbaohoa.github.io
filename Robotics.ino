#include <Servo.h> // Include servo library

//  SERVO SETUP
const int leftServoPin = 13;
const int rightServoPin = 12;
const int leftArm = 11;
const int rightArm = 10;
struct QTIdata {
  int INside;
  int OUTside;
};

Servo servoLeft; // Declare left and right servos
Servo servoRight;
Servo armLeft;
Servo armRight;

// TCS3200 COLOR SENSOR
#define s0 2       //Module pins wiring
#define s1 3
//#define s2 4
#define s3 5
#define out 4

int MyColor; // 1: Red, 2: Blue
int YouColor;
int CurrentColor;

//QTI SETUP
#define leftQTI_IN 8
#define leftQTI_OUT 9
#define rightQTI_IN 7
#define rightQTI_OUT 6

const unsigned int QTITimeout   = 3000;  // us
const unsigned int QTIhold = 200;  // adjust after testing
QTIdata QTI;

// Data for Wheel control
const int Stop_Pulse = 1500;
const int Max_Pulse = 1650;
const int Min_Pulse = 1350;
// Tunning data
long leftPulse = 0;
long rightPulse = 0;
unsigned long numberTune = 0;
int leftTunedPulse = Min_Pulse;
int rightTunedPulse = Max_Pulse;

int  Time90 = 660; // microseconds, time for spin Left 90
float speed = 5*3863/24;
const unsigned long BackupTime = 800; // microseconds time for backup 5 inches

bool LeftEdge = false;
int botProcess = '2';

void setup() {
  Serial.begin(9600);
  delay(2000);

  // Servo motor
  servoLeft.attach(leftServoPin); // Attach left signal to pin 13
  servoRight.attach(rightServoPin); // Attach right signal to pin 12
  armLeft.attach(leftArm); // Attach left signal arm to pin 11
  armRight.attach(rightArm); // Attach right signal arm to pin 10
  closeArm();
  // Color sensor
  pinMode(s0, OUTPUT);   //pin modes
  pinMode(s1, OUTPUT);
  //    pinMode(s2,OUTPUT);  // s2 is always LOW
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);

  // Output of color sensor
  digitalWrite(s0, HIGH); //Putting S0/S1 on HIGH/HIGH levels  means the output frequency scalling is at 100% (recommended)
  digitalWrite(s1, HIGH); //LOW/LOW is off HIGH/LOW is 20% and LOW/HIGH is  2%


  //  MyColor = GetColors();
  if (MyColor == 1) {
    armLeft.write(90); delay(500);
    armLeft.write(180);
    YouColor = 2;
  };
  if (MyColor == 2) {
    armRight.write(110); delay(500);
    armRight.write(0);
    YouColor = 1;
  };
  botProcess = '2';
  openArm();
  
};

void loop() {
  delay(2000);
  switch (botProcess) {
    case '1':
      closeArm();
      runTuning();
      break;
    case '2':
      process2();
      break;
    case '3':
      process3();
      break;
    case '4':
      process4();
      break;
    case '5':
      process5();
      break;
    case '6':
      process6();
      break;
    case '7':
      process7();
  };
};

void runTuning() {
  int baseforwardL = 1400;
  int baseforwardR = 1600;
  int basestep = 10;
  int right_Pulse = baseforwardR;
  int left_Pulse = baseforwardL;
  while (GetColors() != 3) {
    QTI = readQTI(rightQTI_IN, rightQTI_OUT);
    if ((QTI.INside <= QTIhold) && (QTI.OUTside > QTIhold)) {
    }
    else if (QTI.INside > QTIhold && QTI.OUTside > QTIhold) {
      leftPulse = baseforwardL + basestep; // 1400 + 10 =1410 slower
      rightPulse = baseforwardR + basestep; // 1600 + 10 = 1610 faster
    }
    else if (QTI.INside < QTIhold && QTI.OUTside < QTIhold) {
      leftPulse = baseforwardL - basestep; // 1400 -10 = 1390 faster
      rightPulse = baseforwardR - basestep; // 1600 -10 = 1590 slower
    }
    driveForward(leftPulse, rightPulse);
    leftPulse += leftPulse;
    rightPulse += rightPulse;
    numberTune++;
  };
  if (numberTune > 0) {
    leftTunedPulse = (int)leftPulse / numberTune;
    rightTunedPulse = (int)rightPulse / numberTune;
  }
  // go back 5 inches
  driveForward(Stop_Pulse + (Stop_Pulse - leftTunedPulse), Stop_Pulse - (rightTunedPulse - Stop_Pulse));
  static unsigned long StartTime = 0;
  if (StartTime == 0) {
    StartTime = millis();
  }
  if (millis() - StartTime >= BackupTime) {
    stopBot();
    StartTime = 0;
    delay(200);
  }
}

void process2() {
  static char p2state = 'm'; // move forward
  CurrentColor = GetColors();
  static unsigned long StartTime = 0;
  openArm();
  switch (p2state) {
    case 's':
      spinLeft();
      if (StartTime == 0) {
        StartTime = millis();
      }
      if (millis() - StartTime >= Time90) {
        stopBot();
        p2state = 'm';
        openArm();
        StartTime = 0;
      }
      break;
    case 'm':
      driveForward(leftTunedPulse, rightTunedPulse);
      if (CurrentColor != MyColor) {
        stopBot();
        spinLeft(); // spin 180
        if (StartTime == 0) {
          StartTime = millis();
        }
        if (millis() - StartTime >= 2 * Time90) {
          stopBot();
          StartTime = 0;
          botProcess = '3';
          p2state = 's';
        }
      }
      break;
  }
}

void process3() {
  static char p3state = 'm';
  CurrentColor = GetColors();
  static unsigned long StartTime = 0;
  switch (p3state) {
    case 'm':
      driveForward(leftTunedPulse, rightTunedPulse);
      if (CurrentColor != MyColor) {
        stopBot();
        p3state = 'r'; // spin right
      }
      break;
    case 'r':
      spinRight(); // spin 90
      if (StartTime == 0) {
        StartTime = millis();
      }
      if (millis() - StartTime >= Time90) {
        stopBot();
        StartTime = 0;
        botProcess = '4';
        p3state = 'm';
      }
      break;
  }
}

void process4() {
  static char p4state = 'm';
  CurrentColor = GetColors();
  static unsigned long StartTime = 0;
  unsigned long countTime;
  switch (p4state) {
    case 'm':
      driveForward(leftTunedPulse, rightTunedPulse);
      if (StartTime == 0) {
        StartTime = millis();
      }
      countTime = millis() - StartTime;
      if ((CurrentColor != MyColor) || countTime >= BackupTime) {
        stopBot();
        StartTime = 0;
        if (countTime >= BackupTime) {
          p4state = 's'; // spin left
        } else {
          p4state = 's';
          LeftEdge = true;
        }
      }
      break;
    case 's': // spin 180
      spinLeft(); // spin 180
      if (StartTime == 0) {
        StartTime = millis();
      }
      if (millis() - StartTime >= 2 * Time90) {
        stopBot();
        StartTime = 0;
        if (LeftEdge) {
          botProcess = '5';
        } else {
          botProcess = '3';
        }
        p4state = 'm';
      }
      break;
  }
}

void process5() {
  static char p5state = 's'; // spin left 90
  CurrentColor = GetColors();
  static unsigned long StartTime = 0;
  switch (p5state) {
    case 's':
      spinLeft();
      if (StartTime == 0) {
        StartTime = millis();
      }
      if (millis() - StartTime >= Time90) {
        stopBot();
        p5state = 'm';
        openArm();
        StartTime = 0;
      }
      break;
    case 'm':
      if (CurrentColor == MyColor) {
        driveForward(leftTunedPulse, rightTunedPulse);
      } else {
        stopBot();
        spinRight(); // spin 90
        if (StartTime == 0) {
          StartTime = millis();
        }
        if (millis() - StartTime >= Time90) {
          stopBot();
          StartTime = 0;
          botProcess = '6';
          p5state = 's';
        }
      }
      break;
  }
}

void process6() {
  static char p6state = 'm';
  CurrentColor = GetColors();
  static unsigned long StartTime = 0;
  switch (p6state) {
    case 'm':
       driveForward(leftTunedPulse, rightTunedPulse);
      if (CurrentColor != MyColor) {
        stopBot();
        p6state = 's'; // spin left
      }
      break;
    case 's': // spin 180
      spinLeft(); // spin 180
      if (StartTime == 0) {
        StartTime = millis();
      }
      if (millis() - StartTime >= 2 * Time90) {
        stopBot();
        StartTime = 0;
        p6state = 'm';
        botProcess = '7';
      }
      break;
  }
}

void process7() {
  static char p7state = 'm';
  CurrentColor = GetColors();
  static unsigned long StartTime = 0;
  switch (p7state) {
    case 'm':
       driveForward(leftTunedPulse, rightTunedPulse);
      if (CurrentColor != MyColor) {
        stopBot();
        p7state = 's'; // spin left
      }
      break;
    case 's': // spin 180
      spinLeft(); // spin 180
      if (StartTime == 0) {
        StartTime = millis();
      }
      if (millis() - StartTime >= Time90) {
        stopBot();
        StartTime = 0;
        p7state = 'm';
        botProcess = '6';
      }
      break;
  }
}

QTIdata readQTI(int pinINside, int pinOUTside) {
  long durationIN = 0;
  long durationOUT = 0;
  QTIdata data;
  // Charge
  pinMode(pinINside, OUTPUT);
  pinMode(pinOUTside, OUTPUT);

  digitalWrite(pinINside, HIGH);
  digitalWrite(pinOUTside, HIGH);
  delayMicroseconds(200);

  // Discharge and measure
  pinMode(pinINside, INPUT);
  pinMode(pinOUTside, INPUT);
  digitalWrite(pinINside, LOW);
  digitalWrite(pinOUTside, LOW);

  while (digitalRead(pinINside) || digitalRead(pinOUTside)) {
    if (digitalRead(pinINside)) {
      durationIN++;
    }
    if (digitalRead(pinOUTside)) {
      durationOUT++;
    }
  }
  data.INside = durationIN;
  data.OUTside = durationOUT;
  return data;
};

void driveForward() {
  servoLeft.writeMicroseconds(Min_Pulse);
  servoRight.writeMicroseconds(Max_Pulse);
};

void driveForward(int left, int right) {
  servoLeft.writeMicroseconds(left);
  servoRight.writeMicroseconds(right);
}

void goForth(int inches) {
  int time = (int)inches / speed * 1000;
  driveForward();
  delay(time);
};

void driveBackward() {
  servoLeft.writeMicroseconds(Max_Pulse);
  servoRight.writeMicroseconds(Min_Pulse);
};

void goBack(int inches) {
  int time = (int)inches / speed * 1000;
  driveBackward();
  delay(time);
};
void stopBot() {
  servoLeft.writeMicroseconds(Stop_Pulse);
  servoRight.writeMicroseconds(Stop_Pulse);
};

void offBot() {
  servoLeft.writeMicroseconds(0);
  servoRight.writeMicroseconds(0);
}

void spinLeft() {
  servoLeft.writeMicroseconds(Min_Pulse);
  servoRight.writeMicroseconds(Min_Pulse);
};

void spinLeft(int angle) {
  int turnTime = (int)Time90 * angle / 90 + 0.5;
  servoLeft.writeMicroseconds(Min_Pulse);
  servoRight.writeMicroseconds(Min_Pulse);
  delay(turnTime);
  stopBot();
};

void spinRight() {
  servoLeft.writeMicroseconds(Max_Pulse);
  servoRight.writeMicroseconds(Max_Pulse);
};

void spinRight(int angle) {
  int turnTime = (int)Time90 * angle / 90 + 0.5;
  servoLeft.writeMicroseconds(Max_Pulse);
  servoRight.writeMicroseconds(Max_Pulse);
  delay(turnTime);
  stopBot();
};

void turnRight() {
  servoLeft.writeMicroseconds(Max_Pulse);
  servoRight.writeMicroseconds(Stop_Pulse);
};

void turnLeft() {
  servoLeft.writeMicroseconds(Stop_Pulse);
  servoRight.writeMicroseconds(Min_Pulse);
};

int GetColors() { // 0: unknown, 1: red, 2: blue, 3: black, 4: white
  int Red = 0;
  int Blue = 0;
  int Green = 0;
  // digitalWrite(s2,  LOW);                                           //s2/s3 =  LOW/LOW is for RED
  digitalWrite(s3, LOW);
  Red = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);       //until "out" go LOW, we start measuring the duration and stops when "out" is  HIGH again, if you have trouble with this expression check the bottom of the code
  delay(10);
  digitalWrite(s3, HIGH);                                         // s2/s3 = LOW/HIGH is for Blue
  Blue = pulseIn(out, digitalRead(out) == HIGH ? LOW  : HIGH);
  delay(10);
  /*digitalWrite(s2, HIGH);                                         // s2/s3 =  HIGH/HIGH  is for green
    Green = pulseIn(out,  digitalRead(out) == HIGH ? LOW : HIGH);
    delay(10);*/
  Serial.print("Red  value= ");
  Serial.print(Red);
  Serial.print("\t");
  Serial.print("Blue  value= ");
  Serial.print(Blue);
  Serial.println("\t");
  //Serial.print("Green  value= ");
  //Serial.print(Green);
  //Serial.print("\t");
  if (Red >= 20 && Blue > 18) return 3;
  if (Red <Blue ) return 1;
  if ( Red > Blue ) return 2;
  return 0; // unknown
};

void closeArm() {
  armLeft.write(170);
  armRight.write(0);
  delay(500);
};
void openArm() {
  armLeft.write(90);
  armRight.write(110);
  delay(500);
};
