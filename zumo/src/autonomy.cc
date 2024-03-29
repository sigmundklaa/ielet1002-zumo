
#include "autonomy.hh"
#include "battery.hh"
#include "common.hh"
#include "comms.hh"
#include "controller.hh"
#include "housekeep.hh"
#include <Arduino.h>
#include <Wire.h>
#include <Zumo32U4.h>
#include <logging/log.hh>

#define LOG_MODULE autonomy
LOG_REGISTER(common::log_gateway);

// Enum for driveState-switchen for bedre oversikt
enum DriveState {
    followLine,
    intersection,
    turnLeft,
    turnRight,
    deadEnd,
    chargeBattery,
    emptyTrash,
    reverse,
    missingLine,
    address,
    stop,
    waitInit,
    fullStop,
};

// Struct for ryddigere måte for å sjekke svinger i banen
struct TurnCheck {
    uint16_t left;
    uint16_t deadEnd;
    uint16_t right;

    void
    update(uint16_t leftSensor, uint16_t rightSensor)
    {
        if (leftSensor > 200 && rightSensor > 200) {
            deadEnd += 1;
        } else if (leftSensor > 200 && rightSensor < 200) {
            left += 1;
        } else if (leftSensor < 200 && rightSensor > 200) {
            right += 1;
        }
    }

    void
    reset()
    {
        left = 0;
        deadEnd = 0;
        right = 0;
    }
};

static uint8_t enabled = 1;

static TurnCheck turnCheck;
static DriveState driveState = waitInit;

static uint16_t* lineSensorValues;
static uint16_t position = 2000;
static uint16_t maxSpeed = 100;

static int16_t positionError = 0;
static int16_t lastError = 0;
static int16_t speedDifference = 0;
static int16_t leftSpeed;
static int16_t rightSpeed;
static int16_t lineSensorError;

static uint8_t currentAddress = 0;

static bool emptyTrashAddress[3] = {0, 0, 0};
static bool holdingTrash = false;

static void calibrateLineSensors();
static void printReadingsToSerial();

/**
 * @brief Handles button press for a specific button, and acts depending on the
 * current state. If the Zumo is waiting for initialization, it will be
 * calibrated. If it is stopped, it will start and vice versa.
 *
 */
static void
initButtonPress()
{
    if (!enabled) {
        return;
    }

    switch (driveState) {
    case waitInit: {
        LOG_INFO(<< "Calibrating line sensors.");
        calibrateLineSensors();

        LOG_INFO(<< "Line sensors calibrated!");

        driveState = fullStop;
        break;
    };
    case fullStop:
    case stop: {
        driveState = followLine;
        hal::controller.start();
        break;
    };
    default: {
        driveState = fullStop;
        hal::controller.stop();
        break;
    };
    };
}

void
autonomy::on_init()
{
    LOG_INFO(
        << "Line sensors can be calibrated by pressing the C button on the Zumo"
    );
    hal::controller.button_c.set_0s_callback(initButtonPress);

    lineSensorValues =
        reinterpret_cast<uint16_t*>(hal::controller.lines_data());
}

void
autonomy::toggle(uint8_t mode)
{
    if (mode && !enabled) {
        driveState = waitInit;
    }

    enabled = mode;
}

// ----------------
// ----- LOOP -----
// ----------------

void
autonomy::on_tick()
{
    if (driveState == waitInit || !enabled) {
        return;
    }

    static uint32_t sampleTime = 0;
    const uint8_t base = 150;

    // Sjekker posisjon ift. linjen og beregner motorhastighet for å rette opp
    position = hal::controller.position();
    positionError = position - 2000;
    speedDifference = positionError / 4 + 3 * (positionError - lastError);
    lineSensorError = lineSensorValues[1] - lineSensorValues[3];
    lastError = positionError;
    leftSpeed = (int16_t)base + speedDifference - lineSensorError / 7;
    leftSpeed = map(constrain(leftSpeed, 0, (int16_t)base), 0, 400, 0, 255);
    rightSpeed = (int16_t)base - speedDifference + lineSensorError / 7;
    rightSpeed = map(constrain(rightSpeed, 0, (int16_t)base), 0, 400, 0, 255);

    // Oppdaterer søppelstatus for adressene fra serveren
    uint8_t trashRemote;
    if (comms::trash_gw.read(&trashRemote, sizeof(trashRemote)) ==
        trashRemote) {
        if (trashRemote > 0 && trashRemote <= 2) {
            emptyTrashAddress[trashRemote] = 1;
        } else {
            LOG_ERR(<< "invalid trash index");
        }
    }

    if ((lineSensorValues[0] > 200 || lineSensorValues[4] > 200)) {
        turnCheck.update(lineSensorValues[0], lineSensorValues[4]);

        if (driveState == followLine) {
            driveState = intersection;
        }
    }

    // Switchen for linjefølgingen. Inneholder mange sensorsjekk for å være
    // sikker på at den er pålitelig. millis()-delayene brukes stort sett for å
    // sørge for at motorene ikke endrer retning uten stopp i mellom for å
    // spare tannhjulene
    switch (driveState) {
    case followLine:
        if (millis() - sampleTime > 100) {
            if ((lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]
                ) < 400) {
                driveState = missingLine;
            } else {
                hal::controller.set_speeds(leftSpeed, rightSpeed);
            }
        }

        break;
    case intersection:
        hal::controller.set_speeds(maxSpeed * 0.8, maxSpeed * 0.8);

        if (lineSensorValues[0] + lineSensorValues[4] < 100) {
            if ((lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]
                ) < 1000) {
                if (turnCheck.deadEnd > turnCheck.left &&
                    turnCheck.deadEnd > turnCheck.right) {
                    driveState = deadEnd;
                    sampleTime = millis();
                } else if (turnCheck.left > turnCheck.right) {
                    driveState = turnLeft;
                    sampleTime = millis();
                } else {
                    driveState = turnRight;
                    sampleTime = millis();
                }
            } else {
                driveState = followLine;
                turnCheck.reset();
            }
        }

        break;
    case turnLeft:
        if (millis() - sampleTime <= 100) {
            hal::controller.set_speeds(0, 0);
        }
        if (millis() - sampleTime > 100) {
            hal::controller.set_speeds(-(maxSpeed * 0.6), maxSpeed);

            if (position > 1900 && position < 2100) {
                if (lineSensorValues[2] > 900) {
                    driveState = followLine;
                    hal::controller.set_speeds(0, 0);
                    sampleTime = millis();
                    turnCheck.reset();
                }
            }
        }

        break;
    case turnRight:
        if (millis() - sampleTime <= 100) {
            hal::controller.set_speeds(0, 0);
        }
        if (millis() - sampleTime > 100) {
            hal::controller.set_speeds(maxSpeed, -(maxSpeed * 0.6));

            if (position > 1900 && position < 2100) {
                if (lineSensorValues[2] > 900) {
                    driveState = followLine;
                    hal::controller.set_speeds(0, 0);
                    sampleTime = millis();
                    turnCheck.reset();
                }
            }
        }

        break;
    case deadEnd:
        if (millis() - sampleTime < 100) {
            hal::controller.set_speeds(0, 0);

            if (swbat::battery.need_charge() ||
                swbat::battery.need_replacement() ||
                swbat::battery.need_service()) {

                swbat::battery.charge();
                driveState = chargeBattery;
                sampleTime = millis();
            } else if (holdingTrash) {
                driveState = emptyTrash;
                sampleTime = millis();
            }
        }
        if (millis() - sampleTime >= 100) {
            hal::controller.set_speeds(-(maxSpeed / 2), -(maxSpeed / 2));

            if (millis() - sampleTime >= 1000) {
                if (lineSensorValues[0] < 100 && lineSensorValues[4] < 100) {
                    turnCheck.reset();
                    driveState = reverse;
                }
            }
        }

        break;
    case chargeBattery:
        hal::controller.set_speeds(0, 0);

        if (!swbat::battery.charging()) {
            driveState = deadEnd;
            sampleTime = millis();
        } else if (millis() - sampleTime >= 50) {
            sampleTime = millis();
        }

        break;
    case emptyTrash:
        if (millis() - sampleTime > 2000) {
            holdingTrash = false;
            driveState = deadEnd;
            sampleTime = millis();
        }

        break;
    case reverse:
        if (turnCheck.left) {
            driveState = turnLeft;
            turnCheck.reset();
            sampleTime = millis();
        }
        if (turnCheck.right) {
            driveState = turnRight;
            turnCheck.reset();
            sampleTime = millis();
        }

        break;
    case missingLine:
        if (lineSensorValues[0] + lineSensorValues[4] > 500) {
            driveState = address;
        }

        hal::controller.set_speeds(maxSpeed * 0.8, maxSpeed * 0.8);

        if ((lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]) >
            500) {
            driveState = followLine;
        }

        break;
    case address:
        if (currentAddress > 2) {
            currentAddress = 0;
        }

        if (emptyTrashAddress[currentAddress]) {
            comms::trash_gw.write(&currentAddress, sizeof(currentAddress));

            emptyTrashAddress[currentAddress] = false;
            holdingTrash = true;
            driveState = stop;
            currentAddress += 1;
            sampleTime = millis();
        } else if ((lineSensorValues[0] + lineSensorValues[4]) < lineSensorValues[2]) {
            currentAddress += 1;
            turnCheck.reset();
            driveState = followLine;
        }

        break;
    case stop:
        hal::controller.set_speeds(0, 0);

        if (millis() - sampleTime > 2000) {
            hal::controller.set_speeds(maxSpeed * 0.5, maxSpeed * 0.5);
        }

        if (millis() - sampleTime > 2500) {
            driveState = followLine;
            turnCheck.reset();
        }

        break;
    case waitInit:
    case fullStop:
        break;
    }
}

// Funksjonen kjøres før linjefølgingen for å kalibrere linjefølgingssensorene
void
calibrateLineSensors()
{
    delay(1000);
    hal::controller.start();
    uint8_t speed = 75; // 255 / 4;

    for (uint16_t i = 0; i < 112; i++) {
        hal::controller.calibrate();

        if (i > 30 && i < 85) {
            hal::controller.set_speeds(-speed, speed);
        } else {
            hal::controller.set_speeds(speed, -speed);
        }

        hal::controller.run();
    }

    hal::controller.stop();
}

// Funksjon som ble brukt mye under programmering for å printe til serial det
// Zumoens sensorer leste
void
printReadingsToSerial()
{
    static char buffer[80];
    ::sprintf(
        buffer, "%4d %4d %4d %4d %4d", lineSensorValues[0], lineSensorValues[1],
        lineSensorValues[2], lineSensorValues[3], lineSensorValues[4]
    );

    LOG_INFO(
        << "Drive state: " << String(driveState) << " | Position: "
        << String(hal::controller.position()) << " | Line values: " << buffer
    );
}
