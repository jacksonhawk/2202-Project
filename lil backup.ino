#include <Servo.h>
#include <EEPROM.h>
#include <uSTimer2.h>
#include <CharliePlexM.h>
#include <Wire.h>
#include <I2CEncoder.h>
#include <MPU6050_tockn.h>
#include <SoftwareSerial.h>

//servo declaration
Servo servo_RightMotor;
Servo servo_LeftMotor;
Servo basket;
Servo arm;

//IRSHIT
SoftwareSerial front_ir(A3, 13);
SoftwareSerial back_ir(A1, 13);// RX, TX


I2CEncoder encoder_RightMotor;
I2CEncoder encoder_LeftMotor;

long Prev_Left_Motor_Position;
long Prev_Right_Motor_Position;
long Curr_Left_Motor_Position;
long Curr_Right_Motor_Position;

MPU6050 mpu6050(Wire);

int US_check = 0;

//pins (changeable)
const int ultrasonic_Front_IN = 6;
const int ultrasonic_Front_OUT = 5;
const int ultrasonic_Left_IN = 8;
const int ultrasonic_Left_OUT = 7;
const int ultrasonic_Right_IN = 4;
const int ultrasonic_Right_OUT = 0;
const int motor_Right = 11;
const int motor_Left = 10;
const int infrared_Back = A1;
const int infrared_Front = A3;
const int button_Back = A0;


//const int bumper = 2;

//calibration values
unsigned int motor_Speed = 1900;        //run speed
unsigned int motor_Speed_Offset = 200;
unsigned int motor_Left_Speed;
unsigned int motor_Right_Speed;
unsigned int distance_Front = 20;  //FM US distance
unsigned int distance_Left = 20;  //FM US distance
unsigned int distance_Right = 20;  //FM US distance
unsigned int distance_US = 20;


//helper variables
unsigned long ul_Echo_Time;
bool Left_turn;
bool Right_turn;
bool hit = false;
bool isReturning = false;
bool turnt = false;
unsigned int timeCharged = 0;
int direction_East = 0;
int degree_Tolerance = 5;
bool isReturned = false;
bool infraredSeen_Back = false;
bool infraredSeen_Front = false;
int mode = 2;
bool firstRun = true;
int lastmode = 0;
long last_time;
int IR_state;

//dummy function for gyroscope
double getDegrees() {
  double currAngle = mpu6050.getAngleZ();
  if (currAngle < 0)
  {
    currAngle = currAngle + (((int(currAngle)) / 360) * -360) + 360;
  }
  if (currAngle > 360)
  {
    currAngle = currAngle - (((int(currAngle)) / 360) * 360);
  }
  return currAngle;
}

void check_US() // Function to check ultrasonics
{
  int middle = Ping(ultrasonic_Front_IN, ultrasonic_Front_OUT); // Change to actual variable
  //Serial.println(middle);
  if (middle <= 5) {
    int left = Ping(ultrasonic_Left_IN, ultrasonic_Left_OUT); // change to actual variables for pins of US
    int right = Ping(ultrasonic_Right_IN, ultrasonic_Right_OUT);
    if (right < left) {
      servo_LeftMotor.writeMicroseconds(1300);
      servo_RightMotor.writeMicroseconds(1500);
      Left_turn = true;
      Right_turn = false;
    }
    else if (left <= right) {
      servo_LeftMotor.writeMicroseconds(1500);
      servo_RightMotor.writeMicroseconds(1300);
      Right_turn = true;
      Left_turn = false;
    }
    Prev_Left_Motor_Position = encoder_LeftMotor.getRawPosition();
    Prev_Right_Motor_Position = encoder_RightMotor.getRawPosition();
  }
  else {
    // keep current speeds
      servo_LeftMotor.writeMicroseconds(1700);
      servo_RightMotor.writeMicroseconds(1700);
  }
}


// measure distance to target using ultrasonic sensor
int Ping(int ci_Ultrasonic_Data, int ci_Ultrasonic_Ping)
{
  //Ping Ultrasonic
  //Send the Ultrasonic Range Finder a 10 microsecond pulse per tech spec
  digitalWrite(ci_Ultrasonic_Ping, HIGH);
  delayMicroseconds(10);  //The 10 microsecond pause where the pulse in "high"
  digitalWrite(ci_Ultrasonic_Ping, LOW);
  //use command pulseIn to listen to Ultrasonic_Data pin to record the
  //time that it takes from when the Pin goes HIGH until it goes LOW
  ul_Echo_Time = pulseIn(ci_Ultrasonic_Data, HIGH, 10000);

  return (ul_Echo_Time / 58);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // setup compass
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  pinMode(ultrasonic_Front_IN, INPUT); // Set up Ultra Sonics
  pinMode(ultrasonic_Front_OUT, OUTPUT);
  pinMode(ultrasonic_Left_IN, INPUT);
  pinMode(ultrasonic_Left_OUT, OUTPUT);
  pinMode(ultrasonic_Right_IN, INPUT);
  pinMode(ultrasonic_Right_OUT, OUTPUT);

  // set up drive motors
  pinMode(motor_Right, OUTPUT);
  servo_RightMotor.attach(motor_Right);
  pinMode(motor_Left, OUTPUT);
  servo_LeftMotor.attach(motor_Left);

  // set up encoders. Must be initialized in order that they are chained together,
  // starting with the encoder directly connected to the Arduino. See I2CEncoder docs
  // for more information
  encoder_LeftMotor.init(1.0 / 3.0 * MOTOR_393_SPEED_ROTATIONS, MOTOR_393_TIME_DELTA);
  encoder_LeftMotor.setReversed(false);  // adjust for positive count when moving forward
  encoder_RightMotor.init(1.0 / 3.0 * MOTOR_393_SPEED_ROTATIONS, MOTOR_393_TIME_DELTA);
  encoder_RightMotor.setReversed(true);  // adjust for positive count when moving forward

  pinMode(A2, INPUT_PULLUP);
  //attachInterrupt(0, bumpers, RISING);

  delay(3000);
  servo_LeftMotor.writeMicroseconds(1750);
  mpu6050.update();
  arm.attach(9);
  basket.attach(3);  
}


//insert dump code here (it is okay to have stopping code)
void dump() {
   arm.write(10);
  basket.write(180);
  delay(1000);
  arm.write(40);
  delay(1000);
  arm.write(80);
  delay(100);
  basket.write(140);
  delay(1000);
  arm.write(120);
  delay(1000);
  basket.write(120);
  delay(1000);
  servo_RightMotor.writeMicroseconds(1700);

  arm.write(150);
  delay(100);
  basket.write(95);
  delay(2000);
  arm.write(170);
  delay(100);
  basket.write(115);
  delay(1000);
}

void loop() {
  arm.write(10);
  basket.write(180);
    mpu6050.update();
//  Serial.println("D,B,Mode");
 // Serial.println(mpu6050.getAngleZ());
  //Serial.print(getDegrees());
  if (mode!=lastmode){
      Serial.print(mode);
      Serial.print(" ");
      Serial.print(getDegrees());
      Serial.println(" ");
  }
  lastmode = mode;
  
  if ((millis()-last_time)>=20){
    last_time = millis();
    IR_state++;
    if (IR_state == 1){
      front_ir.begin(2400);
    }

    else if (IR_state == 2){
      back_ir.begin(2400);
      IR_state = 0;
    }
  }
  
  if (front_ir.available()) {
    Serial.print("Front ");
    int x = front_ir.read();
    if ((x >= 49 && x <= 53) || (x >= 64 && x <= 68)) {
      Serial.println("FOUND");
      infraredSeen_Front = true;
    }
    else {
      Serial.println("NOT FOUND");
      infraredSeen_Front = false;
    }
  }
  if (back_ir.available()) {
    Serial.print("Back");
    int x = back_ir.read();
    if ((x >= 49 && x <= 53) || (x >= 64 && x <= 68)) {
      Serial.println("FOUND");
      infraredSeen_Back = true;
    }
    else {
      Serial.println("NOT FOUND");
      infraredSeen_Back = false;
    }
  }


  //always check IRs first and front distance
  distance_Front = Ping(ultrasonic_Front_IN, ultrasonic_Front_OUT);

  switch (mode) {
    //run forward until u hit something
    case 1:
    if (Ping(ultrasonic_Front_IN, ultrasonic_Front_OUT)>=10){
      servo_LeftMotor.writeMicroseconds(1700);
      servo_RightMotor.writeMicroseconds(1700);
    }
    else{
      mode++;
    }
      break;
    case 2:
      if(digitalRead(button_Back)==HIGH){
        timeCharged = millis(); //reset charge time
        dump();   //dump
        mode++;   //move on
      }
      else{
        servo_LeftMotor.writeMicroseconds(1300);  //go backwards
        servo_RightMotor.writeMicroseconds(1300);
      }
      break;

    //normal Run  
    case 3:
      if (millis() - timeCharged >=20000){
        mode++;
      }
      else{
        check_US();
      }
    break;
    //back in code (go back set distance + constant)
    case 4:
     break;
   

    //dumping code, then reset
    case 5:
   break;
  }

}