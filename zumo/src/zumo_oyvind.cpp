
#include <Arduino.h>
#include <Wire.h>
#include <Zumo32U4.h>

enum DriveState {
  followLine,         // 0
  intersection,       // 1
  leftIntersection,   // 2
  rightIntersection,  // 3
  turnLeft,           // 4
  turnRight,          // 5
  deadEnd,            // 6
  reverse,            // 7
  missingLine         // 8
};

Zumo32U4OLED display;
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ButtonC buttonC;

uint16_t lineSensorValues[5];
uint16_t position = 2000;
uint16_t maxSpeed = 100;  
uint16_t deadEndCheck;
uint16_t leftTurnCheck;
uint16_t rightTurnCheck;

int16_t positionError = 0;
int16_t lastError = 0;
int16_t speedDifference = 0;
int16_t leftSpeed;
int16_t rightSpeed;
int16_t lineSensorError;


DriveState driveState = followLine;

void calibrateLineSensors();
void loadCustomCharacters();
void printBar(uint8_t height);
void printReadingsToDisplay();
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

  display.clear();
  display.print(F("Press C"));
  display.gotoXY(0, 1);
  display.print(F("to calib"));
  buttonC.waitForButton();

  calibrateLineSensors();
  loadCustomCharacters();
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
  leftSpeed = (int16_t)maxSpeed + speedDifference - lineSensorError;
  leftSpeed = constrain(leftSpeed, 0, (int16_t)maxSpeed);
  rightSpeed = (int16_t)maxSpeed - speedDifference + lineSensorError;
  rightSpeed = constrain(rightSpeed, 0, (int16_t)maxSpeed);

  if ((lineSensorValues[0] > 500 || lineSensorValues[4] > 500) 
  && (driveState == followLine || driveState == intersection || driveState == reverse))
  {
    driveState = intersection;

    if (lineSensorValues[0] > 500 && lineSensorValues[4] > 500)
    {
      deadEndCheck += 1;
      leftTurnCheck = 0;
      rightTurnCheck = 0;
    }
    else if (lineSensorValues[0] > 500 && lineSensorValues[4] < 500)
    {
      deadEndCheck = 0;
      leftTurnCheck += 1;
      rightTurnCheck = 0;
    }
    else if (lineSensorValues[0] < 500 && lineSensorValues[4] > 500)
    {
      deadEndCheck = 0;
      leftTurnCheck = 0;
      rightTurnCheck += 1;
    }
  }

  switch (driveState) 
  {
    case followLine:
      /*
      if (deadEndCheck > 2)
      {
        driveState = deadEnd;
        motors.setSpeeds(0, 0);
        sampleTime = millis();
      }
      else if (leftTurnCheck > 2)
      {
        driveState = leftIntersection;
      }
      else if (rightTurnCheck > 2)
      {
        driveState = rightIntersection;
      }
      else if ((lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]) < 500)
      {
        driveState = missingLine;
      }
      else if (lineSensorValues[0] + lineSensorValues[4] < 500)
      {
        motors.setSpeeds(leftSpeed, rightSpeed);
      }
      */      

      motors.setSpeeds(leftSpeed, rightSpeed);

      break;
    case intersection:
      motors.setSpeeds(maxSpeed * 0.8, maxSpeed * 0.8);

      if (lineSensorValues[0] + lineSensorValues[4] < 100)
      {
        if ((lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]) < 300)
        {
          if (deadEndCheck > leftTurnCheck)
          {
            if (deadEndCheck > rightTurnCheck)
            {
              driveState = deadEnd;
              motors.setSpeeds(0, 0);
              sampleTime = millis();
            }
            else
            {
              driveState = turnRight;
              sampleTime = millis();
            }
          }
          else if (leftTurnCheck > rightTurnCheck)
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
          deadEndCheck = 0;
          leftTurnCheck = 0;
          rightTurnCheck = 0;
        }
      }

      break;
    case leftIntersection:
      motors.setSpeeds(maxSpeed * 0.8, maxSpeed * 0.8);

      if (lineSensorValues[0] < 100 && lineSensorValues[4] < 100)
      {
        if ((lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]) < 1000)
        {
          driveState = turnLeft;
          sampleTime = millis();
        }
        else
        {
          driveState = followLine;
          deadEndCheck = 0;
          leftTurnCheck = 0;
          rightTurnCheck = 0;
        }
      }

      break;
    case rightIntersection:
      motors.setSpeeds(maxSpeed * 0.8, maxSpeed * 0.8);

      if (lineSensorValues[0] < 100 && lineSensorValues[4] < 100)
      {
        if ((lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]) < 1000)
        {
          driveState = turnRight;
          sampleTime = millis();
        }
        else
        {
          driveState = followLine;
          deadEndCheck = 0;
          leftTurnCheck = 0;
          rightTurnCheck = 0;
        }
      }

      break;
    case turnLeft:
      motors.setSpeeds(-(maxSpeed * 0.9), maxSpeed);

      if (millis() - sampleTime > (850 / maxSpeed * 100))
      {
        motors.setSpeeds(0, 0);

        if (millis() - sampleTime > (850 / maxSpeed * 100 + 50))
        {
          driveState = followLine;
        }
      }

      break;
    case turnRight:
      motors.setSpeeds(maxSpeed, -(maxSpeed * 0.9));

      if (millis() - sampleTime > (850 / maxSpeed * 100))
      {
        motors.setSpeeds(0, 0);
        
        if (millis() - sampleTime > (850 / maxSpeed * 100 + 50))
        {
          driveState = followLine;
        }
      }

      break;
    case deadEnd:
      if (millis() - sampleTime >= 100)
      {
        motors.setSpeeds(-(maxSpeed / 2), -(maxSpeed / 2));
        if (lineSensorValues[0] < 100 && lineSensorValues[4] < 100)
        {
          //leftTurnCheck = 0;
          //rightTurnCheck = 0;
          driveState = reverse;
        }
      }

      break;
    case reverse:
      if (leftTurnCheck > 1)
      {
        driveState = turnLeft;
        sampleTime = millis();
      }
      if (rightTurnCheck > 1)
      {
        driveState = turnRight;
        sampleTime = millis();
      }

      break;
    case missingLine:
      motors.setSpeeds(maxSpeed * 0.8, maxSpeed * 0.8);

      if ((lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]) > 500)
      {
        driveState = followLine;
      }

      break;
  }

  //readSensorValues();

  /*
  */
  Serial.print("ST: ");
  Serial.print(driveState);
  Serial.print(" | PO: ");
  Serial.print(position);
  Serial.print(" | ER: ");
  Serial.print(positionError);
  Serial.print(" | DE: ");
  Serial.print(deadEndCheck);
  Serial.print(" | LE: ");
  Serial.print(leftTurnCheck);
  Serial.print(" | RI: ");
  Serial.println(rightTurnCheck);
  
}


// ---------------------
// ----- FUNCTIONS -----
// ---------------------


void calibrateLineSensors()
  {
  delay(1000);

  display.clear();
  display.gotoXY(0, 0);
  display.print(F("Line cal"));

  for (uint16_t i = 0; i < 162; i++)
  {
    display.gotoXY(0, 1);
    display.print(i);
    lineSensors.calibrate();

    if (i > 40 && i < 115)
    {
      motors.setSpeeds(-100, 100);
    }
    else if (i == 40 || i == 115)
    {
      motors.setSpeeds(0, 0);
      delay(100);
    }
    else
    {
      motors.setSpeeds(100, -100);
    }
  }

  motors.setSpeeds(0, 0);

  display.clear();
}


void loadCustomCharacters()
  {
  static const char levels[] PROGMEM = {
    0, 0, 0, 0, 0, 0, 0, 63, 63, 63, 63, 63, 63, 63
  };
  display.loadCustomCharacter(levels + 0, 0);  // 1 bar
  display.loadCustomCharacter(levels + 1, 1);  // 2 bars
  display.loadCustomCharacter(levels + 2, 2);  // 3 bars
  display.loadCustomCharacter(levels + 3, 3);  // 4 bars
  display.loadCustomCharacter(levels + 4, 4);  // 5 bars
  display.loadCustomCharacter(levels + 5, 5);  // 6 bars
  display.loadCustomCharacter(levels + 6, 6);  // 7 bars
}


void printBar(uint8_t height)
{
  if (height > 8) 
  { 
    height = 8; 
  }
  const char barChars[] = {' ', 0, 1, 2, 3, 4, 5, 6, (char)255};
  display.print(barChars[height]);
}


void printReadingsToDisplay()
{
  display.gotoXY(0, 0);
  for (uint8_t i = 0; i < 5; i++)
  {
    uint8_t barHeight = map(lineSensorValues[i], 0, 1000, 0, 8);
    printBar(barHeight);
  }
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

  printReadingsToDisplay();
  printReadingsToSerial();
}
