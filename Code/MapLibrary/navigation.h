#ifndef navigation_h
#define navigation_h
#include "Arduino.h"

 /*************************************************************************
* Author             : Dustin Linnington
* Date               : 2017/09/25
* Description        : Sketch used in conjunction with Belief Revision 
*                    : in cooperation with Aaron Hunter
* Notes              : The Navigation script is intended to keep track of
					 : the current position of the robot and to create paths
					 : to a destination on the grid. The navigation script
					 : will then return a direction to the robot which
					 : will be performed via the RobotController script.
* Issues			 : There are small bugs with the script that causes it
					 : to give inaccurate directions.
**************************************************************************/ 

class Navigation
{
public:
	Navigation();
	enum Direction
	{
		backward = 0,
		forward = 1,
		right = 2,
		left = 3,
		fastRight = 4,
		fastLeft = 5,
		stopped = 6
	};
	enum BehaviourState
	{
		exploring = 0,
		mapping = 1,
		guarding = 2,
		patrolling = 3
	};
	int currentDirection;
	int currentPosition[];

	int intersectionReached(int currentBehaviour, int targetLocation[]);
	bool checkIfMapBuilt();
	int getCurrentPositionX();
	int getCurrentPositionY();

private:
	int getRandomDirection();
	void updatePosition();
	void updateOrientation(int dir);
	int findPathTo(int targetLocation[]);
	void updateMap();
	void buildMap();

	const int MAX_GRID_COLUMNS;
	const int MAX_GRID_ROWS;
	int gridMap[][100];


	int numColumns;
	int numRows;
	bool mapIsBuilt;

	bool travellingAlongRows;
	int travellingDirection;
};
#endif // !NAVIGATION_H