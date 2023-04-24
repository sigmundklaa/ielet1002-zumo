
#include <Arduino.h>
#include <Wire.h>
#include <Zumo32U4.h>

enum DriveState 
{
  followLine,         // 0
  intersection,       // 1
  turnLeft,           // 2
  turnRight,          // 3
  deadEnd,            // 4
  chargeBattery,      // 5
  emptyTrash,         // 6
  reverse,            // 7
  missingLine,        // 8
  address,            // 9
  stop                // 10
};

struct TurnCheck
{
  uint16_t left;
  uint16_t deadEnd;
  uint16_t right;

  void update(uint16_t leftSensor, uint16_t rightSensor)
  {
    if (leftSensor > 200 && rightSensor > 200)
    {
      deadEnd += 1;
    }
    else if (leftSensor > 200 && rightSensor < 200)
    {
      left += 1;
    }
    else if (leftSensor < 200 && rightSensor > 200)
    {
      right += 1;
    }
  }

  void reset()
  {
    left = 0;
    deadEnd = 0;
    right = 0;
  }
};

TurnCheck turnCheck;
DriveState driveState = followLine;

Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ButtonC buttonC;

uint16_t lineSensorValues[5];
uint16_t position = 2000;
uint16_t maxSpeed = 100;  

int16_t positionError = 0;
int16_t lastError = 0;
int16_t speedDifference = 0;
int16_t leftSpeed;
int16_t rightSpeed;
int16_t lineSensorError;
int16_t bankBalance = 100;

uint8_t currentAddress = 0;
uint8_t stopAddress = 2;
uint8_t batteryCharge = 100;

bool emptyTrashAddress[3] = {1, 0, 1};
bool holdingTrash = false;

void calibrateLineSensors();
void printReadingsToSerial();
void readSensorValues();


// -----------------
// ----- SETUP -----
// -----------------


void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);

  lineSensors.initFiveSensors();

  Serial.println("Press C on Zumo to calibrate line sensors.");
  buttonC.waitForButton();

  Serial.println("Calibrating line sensors.");
  calibrateLineSensors();
  
  Serial.println("Line sensors calibrated!");
}


// ----------------
// ----- LOOP -----
// ----------------


void loop()
{
  static uint32_t sampleTime = 0;

  position = lineSensors.readLine(lineSensorValues);
  positionError = position - 2000;
  speedDifference = positionError / 4 + 3 * (positionError - lastError);
  lineSensorError = lineSensorValues[1] - lineSensorValues[3];
  lastError = positionError;
  leftSpeed = (int16_t)maxSpeed + speedDifference - lineSensorError/7;
  leftSpeed = constrain(leftSpeed, 0, (int16_t)maxSpeed);
  rightSpeed = (int16_t)maxSpeed - speedDifference + lineSensorError/7;
  rightSpeed = constrain(rightSpeed, 0, (int16_t)maxSpeed);

  if ((lineSensorValues[0] > 200 || lineSensorValues[4] > 200))
  {
    turnCheck.update(lineSensorValues[0], lineSensorValues[4]);

    if (driveState == followLine)
    {
      driveState = intersection;
    }
  }

  switch (driveState) 
  {
    case followLine:
      if ((lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]) < 400)
      {
        driveState = missingLine;
      }
      else
      {
        motors.setSpeeds(leftSpeed, rightSpeed);
      }

      break;
    case intersection:
      motors.setSpeeds(maxSpeed * 0.8, maxSpeed * 0.8);

      if (lineSensorValues[0] + lineSensorValues[4] < 100)
      {
        if ((lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]) < 1000)
        {
          if (turnCheck.deadEnd > turnCheck.left)
          {
            if (turnCheck.deadEnd > turnCheck.right)
            {
              driveState = deadEnd;
              sampleTime = millis();
            }
            else
            {
              driveState = turnRight;
              sampleTime = millis();
            }
          }
          else if (turnCheck.left > turnCheck.right)
          {
            driveState = turnLeft;
            sampleTime = millis();
          }
          else
          {
            driveState = turnRight;
            sampleTime = millis();
          }
        }
        else
        {
          driveState = followLine;
          turnCheck.reset();
        }
      }

      break;
    case turnLeft:
      motors.setSpeeds(-(maxSpeed*0.7), maxSpeed);

      if (millis() - sampleTime > 1300)
      {
        motors.setSpeeds(0, 0);

        if (millis() - sampleTime > 1350)
        {
          driveState = followLine;
          turnCheck.reset();
        }
      }

      break;
    case turnRight:
      motors.setSpeeds(maxSpeed, -(maxSpeed*0.7));

      if (millis() - sampleTime > 1300)
      {
        motors.setSpeeds(0, 0);
        
        if (millis() - sampleTime > 1350)
        {
          driveState = followLine;
          turnCheck.reset();
        }
      }

      break;
    case deadEnd:
      if (millis() - sampleTime < 100)
      {
        motors.setSpeeds(0, 0);

        if (batteryCharge < 20)
        {
          driveState = chargeBattery;
          sampleTime = millis();
        }
        else if (holdingTrash)
        {
          driveState = emptyTrash;
          sampleTime = millis();
        }
      }
      if (millis() - sampleTime >= 100)
      {
        motors.setSpeeds(-(maxSpeed / 2), -(maxSpeed / 2));

        if (millis() - sampleTime >= 1000)
        {
          if (lineSensorValues[0] < 100 && lineSensorValues[4] < 100)
          {
            turnCheck.reset();
            driveState = reverse;
          }
        }
      }

      break;
    case chargeBattery:
      motors.setSpeeds(0, 0);
      
      if (batteryCharge >= 100)
      {
        driveState = deadEnd;
        sampleTime = millis();
      }
      else if (millis() - sampleTime >= 50)
      {
        batteryCharge += 1;
        bankBalance -= 1;
        sampleTime = millis();
      }

      break;
    case emptyTrash:
      if (millis() -  sampleTime > 2000)
      {
        holdingTrash = false;
        driveState = deadEnd;
        sampleTime = millis();
      }
      break;
    case reverse:
      if (turnCheck.left)
      {
        driveState = turnLeft;
        turnCheck.reset();
        sampleTime = millis();
      }
      if (turnCheck.right)
      {
        driveState = turnRight;
        turnCheck.reset();
        sampleTime = millis();
      }

      break;
    case missingLine:
      if (lineSensorValues[0] + lineSensorValues[4] > 500)
      {
        driveState = address;
      }

      motors.setSpeeds(maxSpeed * 0.8, maxSpeed * 0.8);

      if ((lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]) > 500)
      {
        driveState = followLine;
      }

      break;
    case address:
      if (currentAddress > 2)
      {
        currentAddress = 0;
      }

      if (emptyTrashAddress[currentAddress])
      {
        emptyTrashAddress[currentAddress] = false;
        holdingTrash = true;
        driveState = stop;
        currentAddress += 1;
        sampleTime = millis();
      }
      else if ((lineSensorValues[0] + lineSensorValues[4]) < lineSensorValues[2])
      {
        currentAddress += 1;
        turnCheck.reset();
        driveState = followLine;
      }

      break;
    case stop:
      motors.setSpeeds(0, 0);

      if (millis() - sampleTime > 2000)
      {
        motors.setSpeeds(maxSpeed*0.5, maxSpeed*0.5);
      }

      if (millis() - sampleTime > 2500)
      {
        driveState = followLine;
        turnCheck.reset();
      }

      break;
  }
}


// ---------------------
// ----- FUNCTIONS -----
// ---------------------


void calibrateLineSensors()
  {
  delay(1000);

  for (uint16_t i = 0; i < 115; i++)
  {
    lineSensors.calibrate();

    if (i > 30 && i < 85)
    {
      motors.setSpeeds(-100, 100);
    }
    else if (i == 30 || i == 85)
    {
      motors.setSpeeds(0, 0);
      delay(50);
    }
    else
    {
      motors.setSpeeds(100, -100);
    }
  }

  motors.setSpeeds(0, 0);
}


void printReadingsToSerial()
{
  static char buffer[80];
  Serial.print("Position: ");
  Serial.print(lineSensors.readLine(lineSensorValues));
  Serial.print(" | Line values: ");
  sprintf(buffer, "%4d %4d %4d %4d %4d\n",
    lineSensorValues[0],
    lineSensorValues[1],
    lineSensorValues[2],
    lineSensorValues[3],
    lineSensorValues[4]
  );
  Serial.print(buffer);
}


void readSensorValues()
{
  lineSensors.readCalibrated(lineSensorValues);

  printReadingsToSerial();
}
