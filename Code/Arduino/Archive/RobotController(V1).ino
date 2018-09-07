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

#include "MeOrion.h"
#include <Wire.h>
#include <SoftwareSerial.h>
#include <Time.h>

const float INTERSECTION_TIMER_MAX = 0.1;
const float OFF_GRID_TIMER_MAX = 1;
const int MAX_GRID_ROWS = 6;
const int MAX_GRID_COLUMNS = 6;

// Math Variables
float tau = 6.28318530718;
float pi = 3.14159265358;

// Map Variables
int gridMap[MAX_GRID_ROWS][MAX_GRID_COLUMNS] = {1};
uint8_t columns = 0;
uint8_t rows = 0;
uint8_t mapLatitude = 0;
uint8_t mapLongitude = 0;
bool isTravellingAlongRows = true;
bool mapBuilt = false;

// Navigation

float intersectionTimer = 0;
float offGridTimer = 0;
float previousTime = 0;
bool atPossibleIntersection = false;
bool intersectionChecked = false;
bool checkingIfOutOfGrid = false;
bool isOffGrid = false;
bool returningToGrid = false;
uint8_t currentPostion = 0;
uint8_t lastSensorState = 0;

//////////////////
// Robot Variables
//////////////////
bool robotIsActive = false;
uint8_t roboWidth = 0;
uint8_t roboLength = 0;
MeBuzzer buzzer;

// Motor Info
MeDCMotor motor1(M1);
MeDCMotor motor2(M2);
uint8_t motorSpeed = 200;

// Sensor Info
//MeUltrasonicSensor ultraSensor(PORT_6); 
uint8_t maximumRange = 140;

Me4Button buttonSensor;
uint8_t keyPressed = 0;
bool onBoardButtonReleased = true;

///// REWRITTEN VARIABLES /////

// Map navigation
byte currentPosition[2] = {0};
byte destinationPosition[2] = {0};
bool atDestination = true;
bool inIntersection = false;
enum State {exploring = 0, navigating = 1}currentState; 
enum DrivingState {backward = 0, forward = 1, turnRight = 2, turnLeft = 3}currentDrivingState;
int drivingSpeed = 200;

// Robot Sensors
enum Sensors {farRightSensor = 0, midRightSensor = 1, midLeftSensor = 2, farLeftSensor = 3};
constexpr int primeNumbers[] = {3, 5, 7, 11};
int sensorValues[4] = {0};
int previousSensorValues[4] = {0};

int combinedSensorData = 0;
int previousCombinedSensorData = 0;

void setup()
{
    Serial.begin(9600);

    currentDrivingState = forward;
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

void driveTo(int column, int row)
{
    if (currentPosition[0] == column && currentPosition[1] == row)
    {
        return;
    }
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

    if (isOffGrid)
    {
        returnToGrid();
        return;
    }
    
    switch(combinedSensorData)
    {
        case primeNumbers[midLeftSensor]:
            inIntersection = false;
            currentDrivingState = getDirections();
            break;
        case primeNumbers[farLeftSensor] + primeNumbers[midLeftSensor]:
        case primeNumbers[farLeftSensor]:
            currentDrivingState = turnLeft;
            break;
        case primeNumbers[midRightSensor] + primeNumbers[midLeftSensor]:
        case primeNumbers[midRightSensor] + primeNumbers[farRightSensor]:
        case primeNumbers[midRightSensor]:
        case primeNumbers[farRightSensor]:
            currentDrivingState = turnRight;
            break;
        case (primeNumbers[farRightSensor] + primeNumbers[midRightSensor] + primeNumbers[midLeftSensor] + primeNumbers[farLeftSensor]):
        case (primeNumbers[farRightSensor] + primeNumbers[midRightSensor] + primeNumbers[midLeftSensor]):
            if (didEnterIntersection())
            {
                registerIntersection();
            }
            break;
        case 0:
            inIntersection = false;
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
    isOffGrid = true;

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
            currentDrivingState = turnRight;
            break;
        case primeNumbers[farRightSensor] + primeNumbers[midRightSensor] + primeNumbers[midLeftSensor] + primeNumbers[farLeftSensor]:
        case primeNumbers[farRightSensor] + primeNumbers[midRightSensor] + primeNumbers[midLeftSensor]:
        case primeNumbers[farLeftSensor] + primeNumbers[midRightSensor] + primeNumbers[midLeftSensor]:
            currentDrivingState = forward;
            break;
    }
}

// TODO: Put this in a library!
void registerIntersection()
{
    
}
// TODO: Put this in a library!
DrivingState getDirections()
{
    return forward;
}

void sensorStateChangeCleanup()
{
    switch(lastSensorState)
    {
        case S1_IN_S2_IN: 
            intersectionChecked = false;
            returningToGrid = false;
            break;
        case S1_IN_S2_OUT:
            
            break;
        case S1_OUT_S2_IN: 
            
            break;
        case S1_OUT_S2_OUT: 
            checkingIfOutOfGrid = false;
            isOffGrid = false;
            offGridTimer = 0;
            break;
        default: 
            break;
    }
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

void drive(int direction, uint8_t speed)
{ 
    int leftSpeed = 0;
    int rightSpeed = 0;
    switch(direction)
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
        case turnRight:
            leftSpeed = -speed;
            rightSpeed = -speed;
            break;
            
        // Turn Right
        case turnLeft:
            leftSpeed = speed;
            rightSpeed = speed;
            break;
    }

    motor1.run(rightSpeed);
    motor2.run(leftSpeed);
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

void changeDirection()
{
    // If isTravellingAlongRows is false that means we are now at columns, and if we're turning then we are going back to rows.
    // What that means is that the map has already been fully explored (since it's a symmetrical grid) and we can now build it.
    if (isTravellingAlongRows == false && mapBuilt == false)
    {
        buildMap();
    }
    isTravellingAlongRows = !isTravellingAlongRows;
}

void encounteredIntersection()
{
    if (currentState == exploring)
    {
        if (isTravellingAlongRows)
        {
            addRow();
        }
        else
        {
            addColumn();
        }
    }
    else
    {}
}

void addRow()
{
    rows++;
    Serial.println("Rows: ");
    Serial.println(rows);
}

void addColumn()
{
    columns++;
    Serial.println("Columns: ");
    Serial.println(columns);
}

void buildMap()
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < columns; ++j)
        {
            gridMap[i][j] = 1;
        }
    }

    mapBuilt = true;
    Serial.println(rows * columns);
}
