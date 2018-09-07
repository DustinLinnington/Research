

 /*************************************************************************
* Author             : Dustin Linnington
* Date               : 31/07/2017
* Description        : Sketch used in conjunction with Belief Revision 
*                    : in cooperation with Aaron Hunter
* Notes              : The robot should be situated on the outer perimiter 
*                    : of a maze abd should be facing clockwise with
*                    : the far left line sensor on the outside of the maze and 
*                    : the far right sensor on the inside. The inner left sensor
*                    : should be placed on the maze line, with the inner right
*                    : sensor on the inside of the maze. The maze should have a 
*                    : white background with black lines that are thick enough for
*                    : both the inner left and inner right sensors to be over it
*                    : at the same time. Pressing the onboard button will cause
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

Navigation navigation;

const float INTERSECTION_TIMER_MAX = 0.1;
const float OFF_GRID_TIMER_MAX = 1;

// Navigation
float intersectionTimer = 0;
float offGridTimer = 0;
float previousTime = 0;
bool atPossibleIntersection = false;
bool intersectionChecked = false;
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
enum BehaviourState {exploring = 0, mapping = 1, guarding = 2, patrolling = 3}currentState = mapping; 
enum Direction {backward = 0, forward = 1, right = 2, left = 3};
Direction currentDrivingState = forward;
int drivingSpeed = 100;
int targetLocation[] = {0};

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

    Serial.println("Hi");
}

void loop()
{
    // This flips the bool robotIsActive if the on-board button is pressed.
    checkIfButtonPressed();
    if (!robotIsActive)
        return;

    updateTimer();
    readLineSensorData();
    parseSensorData();
    
    drive(currentDrivingState, drivingSpeed);
}

void deactivateRobot()
{
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
    if (combinedSensorData == previousCombinedSensorData)
    {
        return;
    }
    if (combinedSensorData != 0)
    {
        isOffGrid = false;
        offGridTimer = 0;
        checkingIfOutOfGrid = false;
    }
    if (combinedSensorData != (primeNumbers[farRightSensor] + primeNumbers[midRightSensor] + primeNumbers[midLeftSensor] + primeNumbers[farLeftSensor])
          && combinedSensorData != (primeNumbers[farRightSensor] + primeNumbers[midRightSensor] + primeNumbers[midLeftSensor]))
          {
              atPossibleIntersection = false;
              intersectionTimer = 0;
              intersectionChecked = false;
          }

    if (isOffGrid)
    {
        returnToGrid();
        return;
    }
    
    switch(combinedSensorData)
    {
        case primeNumbers[midRightSensor] + primeNumbers[midLeftSensor]:
        case primeNumbers[midLeftSensor]:
        case primeNumbers[midRightSensor]:
            inIntersection = false;
            currentDrivingState = getDirections();
            break;
        case primeNumbers[farLeftSensor] + primeNumbers[midLeftSensor]:
        case primeNumbers[farLeftSensor]:
            currentDrivingState = left;
            break;
        case primeNumbers[midRightSensor] + primeNumbers[farRightSensor]:
        case primeNumbers[farRightSensor]:
            currentDrivingState = right;
            break;
        case primeNumbers[farRightSensor] + primeNumbers[midRightSensor] + primeNumbers[midLeftSensor] + primeNumbers[farLeftSensor]:
        case primeNumbers[farRightSensor] + primeNumbers[midRightSensor] + primeNumbers[midLeftSensor]:
            if (didEnterIntersection())
            {
                  Serial.println("Entering Intersection");
                currentDrivingState = navigation.intersectionReached(currentState, targetLocation);
                    Serial.println(currentDrivingState);
            }
            break;
        case 0:
            inIntersection = false;
            currentDrivingState = left;
            returnToGrid();
            break;
    }

    previousCombinedSensorData = combinedSensorData;
}

bool didEnterIntersection()
{
    if ((currentDrivingState == forward || currentDrivingState == backward) && inIntersection == false)
    {
        inIntersection = true;
        return true;
    }
    else
    {
        return false; 
    }
}

void returnToGrid()
{
    if (!checkIfOffGrid())
    {
        return;
    }
  
    isOffGrid = true;

    if (currentState == mapping)
    {
        if (navigation.checkIfMapBuilt())
        {
            Serial.println("Map Built");
            currentState = exploring;
        }
    }

    switch(combinedSensorData)
    {
        case primeNumbers[midLeftSensor]:
            isOffGrid = false;
            currentDrivingState = getDirections();
            break;
        case primeNumbers[midRightSensor] + primeNumbers[midLeftSensor]:
        case primeNumbers[midRightSensor]:
        case primeNumbers[farRightSensor]:
        case 0:
            currentDrivingState = right;
            break;
        case primeNumbers[farRightSensor] + primeNumbers[midRightSensor] + primeNumbers[midLeftSensor] + primeNumbers[farLeftSensor]:
        case primeNumbers[farRightSensor] + primeNumbers[midRightSensor] + primeNumbers[midLeftSensor]:
        case primeNumbers[farLeftSensor] + primeNumbers[midRightSensor] + primeNumbers[midLeftSensor]:
            currentDrivingState = forward;
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
            
        // Turn Left
        case right:
            leftSpeed = -speed;
            rightSpeed = -speed;
            break;
            
        // Turn Right
        case left:
            leftSpeed = speed;
            rightSpeed = speed;
            break;
    }

    motor1.run(rightSpeed);
    motor2.run(leftSpeed);
}

// TODO: Put this in a library!
Direction getDirections()
{
    return forward;
}

void updateTimer()
{
    float deltaTime = ((float)millis() / 1000) - previousTime;
  
    if (atPossibleIntersection)
    {
        intersectionTimer += deltaTime;
    }
    if (checkingIfOutOfGrid)
    {
        offGridTimer += deltaTime;
    }

    previousTime = (float)millis() / 1000;
}

bool checkIfIntersection()
{
    atPossibleIntersection = true;
    if (intersectionTimer >= INTERSECTION_TIMER_MAX && intersectionChecked == false)
    {
        intersectionChecked = true;
        intersectionTimer = 0;
        
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
