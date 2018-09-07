 /*************************************************************************
* Author             : Dustin Linnington
* Date               : 17/09/2017
* Description        : Sketch used in conjunction with Belief Revision 
*                    : in cooperation with Aaron Hunter
* Notes              : The robot should be situated on the outer perimiter 
*                    : of a maze abd should be facing clockwise with
*                    : the far left line sensor on the outside line of the maze. 
*                    : Pressing the onboard button will cause
*                    : the robot to begin creating a map for the maze.
*                    : It will stop when the button is pressed again or it
*                    : finishes mapping the maze.
* Right Sensor Port1 : MePort(1).dRead1()  
* Left Sensor Port1  : MePort(3).dRead1()         
* Right Sensor Port2 : MePort(1).dRead2()  
* Left Sensor Port2  : MePort(2).dRead2()
**************************************************************************/ 

#include <navigation.h>
#include "MeOrion.h"
#include <Wire.h>
#include <SoftwareSerial.h>
#include <Time.h>

const float OFF_GRID_TIMER_MAX = 0.3;
const int ROBOT_NUMBER = 0;
const int GUARD_LOCATION_X = 4;
const int GUARD_LOCATION_Y = 3;
const int GUARD_LOCATION_X2 = 0;
const int GUARD_LOCATION_Y2 = 1;

// Navigation
Navigation navigation;
float offGridTimer = 0;
float previousTime = 0;
bool checkingIfOutOfGrid = false;
bool isOffGrid = false;
bool returningToGrid = false;
uint8_t lastSensorState = 0;

// Motor Info
MeDCMotor motor1(M1);
MeDCMotor motor2(M2);

// Map navigation
bool atDestination = true;
bool inIntersection = false;
enum BehaviourState {exploring = 0, mapping = 1, guarding = 2, patrolling = 3}currentBehaviour = mapping; 
enum Direction {backward = 0, forward = 1, right = 2, left = 3, fastRight = 4, fastLeft = 5, stopped = 6};
Direction currentDrivingState = forward;
int drivingSpeed = 200;
int targetLocation[] = {GUARD_LOCATION_X, GUARD_LOCATION_Y};
int returnTo[2] = {0};

// Robot Sensors
enum Sensors {farRightSensor = 0, midRightSensor = 1, midLeftSensor = 2, farLeftSensor = 3};
constexpr int primeNumbers[] = {3, 5, 7, 11};
int sensorValues[4];
int combinedSensorData = 0;
int previousCombinedSensorData = 0;
Me4Button buttonSensor;
uint8_t keyPressed = 0;
bool onBoardButtonReleased = true;
bool robotIsActive = false;
MeBuzzer buzzer;


void setup()
{
    Serial.begin(9600);
    //Serial.println(navigation.checkIfMapBuilt());
}

void loop()
{
    // This flips the bool robotIsActive if the on-board button is pressed.
    checkIfButtonPressed();
    if (!robotIsActive)
    {
        return;
    }

    updateTimer();
    readLineSensorData();
    parseSensorData();
    
    //drive(currentDrivingState, drivingSpeed);
}

void deactivateRobot()
{
    robotIsActive = false;
    drive(0, 0);
}

void checkIfButtonPressed()
{
    if (!(analogRead(7) > 100) && onBoardButtonReleased)
    {
        buttonPressed();
        onBoardButtonReleased = false;
    }
    else if (analogRead(7) > 100)
    {
        onBoardButtonReleased = true;
    }
}

void buttonPressed()
{
    robotIsActive = !robotIsActive;
    if (!robotIsActive)
    {
        buzzer.tone(350, 300);
        sei();
        buzzer.tone(200, 300);
        sei();
        deactivateRobot();
    }
    else
    {        
        buzzer.tone(400, 300);
        sei();
        buzzer.tone(550, 300);
        sei();
    }
}

void readLineSensorData()
{
    combinedSensorData = 0;
  
    sensorValues[farRightSensor] = MePort(3).dRead1();
    sensorValues[midRightSensor] = MePort(1).dRead1();
    sensorValues[midLeftSensor] = MePort(1).dRead2();
    sensorValues[farLeftSensor] = MePort(2).dRead2();

    if (sensorValues[farRightSensor] == 0)
    {
        combinedSensorData += primeNumbers[farRightSensor];
    }
    if (sensorValues[midRightSensor] == 0)
    {
        combinedSensorData += primeNumbers[midRightSensor];
    }
    if (sensorValues[midLeftSensor] == 0)
    {
        combinedSensorData += primeNumbers[midLeftSensor];
    }
    if (sensorValues[farLeftSensor] == 0)
    {
        combinedSensorData += primeNumbers[farLeftSensor];
    }
}

void parseSensorData()
{
    if (combinedSensorData != 0)
    {
        returnedToGrid();
    }
  
     switch(combinedSensorData)
     {      
        case primeNumbers[farLeftSensor] + primeNumbers[midLeftSensor] + primeNumbers[midRightSensor] + primeNumbers[farRightSensor]:
        {
            if (didEnterIntersection())
            {
                currentDrivingState = navigation.intersectionReached(currentBehaviour, targetLocation);
                Serial.print("Current Driving State: ");
                Serial.println(currentDrivingState);
                Serial.println(navigation.getCurrentPositionX());
                Serial.println(navigation.getCurrentPositionY());
                if (currentDrivingState == stopped)
                {
                    Serial.println("Found it!");
                    buzzer.tone(200, 50);
                    sei();
                    buzzer.tone(200, 50);
                    sei();
//                    if (targetLocation[0] != GUARD_LOCATION_X2 && targetLocation[1] != GUARD_LOCATION_Y2)
//                    {
//                        targetLocation[0] = GUARD_LOCATION_X2;
//                        targetLocation[1] = GUARD_LOCATION_Y2;       
//                        currentDrivingState = navigation.intersectionReached(currentBehaviour, targetLocation);     
//                    }
                }
            }
            break;
        }
        case 0:
        {
            if (currentBehaviour != mapping && currentDrivingState != forward)
            {
                break;
            }
            if (checkIfOffGrid())
            {
                if (isOffGrid == false)
                {
                    isOffGrid = true;
                    if (navigation.checkIfMapBuilt() && currentBehaviour == mapping)
                    {
                        currentBehaviour = guarding;
                        buzzer.tone(350, 50);
                        sei();
                        buzzer.tone(350, 50);
                        sei();                            
                    }
                    else
                    {
                        currentDrivingState = fastRight;
                    }
                }
                else
                {
                    currentDrivingState = fastRight;
                }
            }
            else
            {
                drive(left, drivingSpeed);
                return;
            }
        }
        default:
        {
            inIntersection = false;
            break;
        }
     }

     moveInDirection();
}

void moveInDirection()
{
    switch(currentDrivingState)
    {
        case forward:
        {
            switch(combinedSensorData)
            {
                case primeNumbers[farLeftSensor] + primeNumbers[midLeftSensor] + primeNumbers[midRightSensor] + primeNumbers[farRightSensor]:
                case primeNumbers[farLeftSensor]:
                {
                    drive(forward, drivingSpeed);
                    break;
                }
                case primeNumbers[midLeftSensor]:
                case primeNumbers[midLeftSensor] + primeNumbers[farLeftSensor]:
                case primeNumbers[midRightSensor]:
                case primeNumbers[midRightSensor] + primeNumbers[midLeftSensor]:
                {
                    drive(right, drivingSpeed * 0.75);
                    break;
                }
                case primeNumbers[farRightSensor]:
                case primeNumbers[farRightSensor] + primeNumbers[midRightSensor]:
                {
                    drive(right, drivingSpeed);
                    break;
                }
                case primeNumbers[midRightSensor] + primeNumbers[midLeftSensor] + primeNumbers[farRightSensor]:
                {
                    drive(left, drivingSpeed * 0.75);
                    break;
                }
                default:
                {
                    drive(forward, drivingSpeed * 0.75);
                    break;
                }
            }
            break;
        }
        case right:
        {
            switch(combinedSensorData)
            {
                case 0:
                {
                    currentDrivingState = left;
                    break;
                }
                default:
                {
                    drive(fastRight, drivingSpeed);
                    break;
                }
            }
            break;
        }
        // Returning to grid
        case fastRight:
        {
            switch(combinedSensorData)
            {
                case primeNumbers[farLeftSensor]:
                {
                    currentDrivingState = forward;
                    break;
                }
                default:
                {
                    drive(fastRight, drivingSpeed);
                    break;
                }
            }
            break;
        }
        case left:
        {
            switch(combinedSensorData)
            {
                case primeNumbers[farLeftSensor]:
                {
                    currentDrivingState = forward;
                    break;
                }
                default:
                {
                    drive(fastLeft, drivingSpeed);
                    break;
                }
            }
            break;
        }
        case stopped:
            drive(0, 0);
            break;
    }
}

void drive(int directionToGo, uint8_t speed)
{ 
    int leftSpeed = 0;
    int rightSpeed = 0;
    switch(directionToGo)
    {
        // Backwards
        case backward:
            leftSpeed = speed;
            rightSpeed = -speed;
            break;
            
        // Forwards
        case forward:
            leftSpeed = -speed;
            rightSpeed = speed;
            break;
            
        // Turn Right
        case right:
            leftSpeed = -speed;
            rightSpeed = 0;
            break;
            
        // Turn Left
        case left:
            leftSpeed = 0;
            rightSpeed = speed;
            break;

        // Fast Right
        case fastRight:
            leftSpeed = -speed;
            rightSpeed = -speed;
            break;

        // Fast Left
        case fastLeft:
            leftSpeed = speed;
            rightSpeed = speed;
            break;
    }

    motor1.run(rightSpeed);
    motor2.run(leftSpeed);
}

void updateTimer()
{
    float deltaTime = ((float)millis() / 1000) - previousTime;
  
    if (checkingIfOutOfGrid)
    {
        offGridTimer += deltaTime;
    }

    previousTime = (float)millis() / 1000;
}

bool didEnterIntersection()
{
    // If we haven't already checked if we're in an intersection, then that means we just entered it.
    if (inIntersection == false)
    {
        inIntersection = true;
        return true;
    }
    else
    {
        return false; 
    }
}

// Check if the robot has gone off the grid or if it's just wandered into white space within the grid.
bool checkIfOffGrid()
{
    checkingIfOutOfGrid = true;
    if (offGridTimer >= OFF_GRID_TIMER_MAX)
    {
        return true;
    }
    
    return false;
}

void returnedToGrid()
{
    checkingIfOutOfGrid = false;
    offGridTimer = 0;
    isOffGrid = false;
}

