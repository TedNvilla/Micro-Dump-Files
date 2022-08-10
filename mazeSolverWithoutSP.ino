#include <SparkFun_TB6612.h>
#include <QTRSensors.h>

//////////////TUNING////////////////
#define Kp 0.031                  //
#define Kd 0.4                    //
#define MaxSpeed 120              //
#define BaseSpeed 100             //
const int turnSpeed = 150;        //
const int turnDelay = 300;        //
const int turnDelayA = 620;       //
const int offsetDelay = 100;      //
const int offsetDelaySpeed = 80;  //
const int sensorDelay = 40;       //
const int sensorDelaySpeed = 90;  //
////////////////////////////////////

/////MOTOR PROPERTIES/////////
#define AIN1 3              //
#define BIN1 7              //
#define AIN2 4              //
#define BIN2 8              //
#define PWMA 5              //
#define PWMB 6              //
#define STBY 9              //
const int offsetA = 1;      //
const int offsetB = -1;     //
//////////////////////////////

#define NUM_SENSORS  5 
QTRSensors qtr;

Motor motorR = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motorL = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

unsigned int sensorValues[NUM_SENSORS];

void setup()
{
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){14, 15, 16, 17, 18}, NUM_SENSORS);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  int i;
  delay(1000);
  for (uint16_t i = 0; i < 80; i++)
  {
    if (i < 20 || i >= 60){ right(motorL, motorR, 80); }
    else
      left(motorL, motorR, 80);
    qtr.calibrate();
  }
  brake(motorL, motorR);
  digitalWrite(LED_BUILTIN, LOW);
  delay(2000); 
}

void followSegment(){
  int lastError = 0;
  while(1){
    unsigned int sensorValues[5];
    int position = qtr.readLineBlack(sensorValues); 
    int error = position - 2000;
  
    int motorSpeed = Kp * error + Kd * (error - lastError);
    lastError = error;
  
    int rightMotorSpeed = BaseSpeed + motorSpeed;
    int leftMotorSpeed = BaseSpeed - motorSpeed;
  
    if (rightMotorSpeed > MaxSpeed ) rightMotorSpeed = MaxSpeed; 
    if (leftMotorSpeed > MaxSpeed ) leftMotorSpeed = MaxSpeed;
    if (rightMotorSpeed < 0) rightMotorSpeed = 0;
    if (leftMotorSpeed < 0) leftMotorSpeed = 0;
    motorR.drive(rightMotorSpeed);
    motorL.drive(leftMotorSpeed);
    if (sensorValues[1] < 200 && sensorValues[2] < 200 && sensorValues[3] < 200){ return; }
    else if (sensorValues[0] > 200 || sensorValues[4] > 200){ return; }
  }
}

void turn(unsigned char dir) {
  switch(dir) {
  case 'L':
    left(motorL, motorR, turnSpeed);
    delay(turnDelay);
    break;
  case 'R':
    right(motorL, motorR, turnSpeed);
    delay(turnDelay);
    break;
  case 'B':
    left(motorL, motorR, turnSpeed);
    delay(turnDelayA);
    break;
  case 'S':
    break;
  }
}

unsigned char select_turn(unsigned char found_left, unsigned char found_straight, unsigned char found_right)
{
  if (found_left){ return 'L'; }
  else if (found_straight) { return 'S'; }
  else if (found_right) { return 'R'; }
  else { return 'B'; }
}

void loop()
{
  while(1){
    followSegment();

    motorL.drive(sensorDelaySpeed);
    motorR.drive(sensorDelaySpeed);
    delay(sensorDelay);
    unsigned char found_left = 0;
    unsigned char found_straight = 0;
    unsigned char found_right = 0;
    
    unsigned int sensorValues[5];
    int position = qtr.readLineBlack(sensorValues);

    if (sensorValues[0] > 200){found_right = 1;}      
    if (sensorValues[4] > 200){found_left = 1;}
    
    motorR.drive(offsetDelaySpeed);
    motorL.drive(offsetDelaySpeed);
    delay(offsetDelay);
    
    position = qtr.readLineBlack(sensorValues);

    if (sensorValues[1] > 200 || sensorValues[2] > 200 || sensorValues[3] > 200){ found_straight = 1; }      
    if (sensorValues[1] > 600 && sensorValues[2] > 600 && sensorValues[3] > 600){ break; }

    unsigned char dir = select_turn(found_left, found_straight, found_right);
    turn(dir);
  }
  brake(motorL,motorR);
  delay(10000);
}
