#include <stdlib.h>
#include "Arduino.h"
#include "navigation.h"

const int MAX_GRID_ROWS = 100;
const int MAX_GRID_COLUMNS = 100;

int gridMap[MAX_GRID_COLUMNS][MAX_GRID_ROWS] = { 0 };
int currentPosition[2] = { 0 };
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

int numColumns;
int numRows;
bool mapIsBuilt;

bool travellingAlongRows;
// 1 for going west/north, -1 for going east/south (when viewing the grid with the bottom right as 0, 0)
int travellingDirection;

Navigation::Navigation()
{
	travellingAlongRows = true;
	travellingDirection = 1;
	numColumns = 1;
	numRows = 1;
	mapIsBuilt = false;
	currentDirection = forward;
};

int Navigation::intersectionReached(int currentBehaviour, int targetLocation[])
{
	if (currentBehaviour != stopped)
	{
		updatePosition();
	}

	int dir = 1;
	switch (currentBehaviour)
	{
		case exploring:
			dir = getRandomDirection();
			updateOrientation(dir);
			return dir;
			break;
		case mapping:
			updateMap();
			dir = forward;
			updateOrientation(dir);
			return dir;
			break;
		case guarding:
		case stopped:
			dir = findPathTo(targetLocation);
			updateOrientation(dir);
			return dir;
			break;
		case patrolling:
			break;
		default:
			break;
		}
	return forward;
}

int Navigation::getCurrentPositionX()
{
	return currentPosition[0];
}

int Navigation::getCurrentPositionY()
{
	return currentPosition[1];
}

int Navigation::getRandomDirection()
{
	return rand() % 4 + 1;
}

void Navigation::updateOrientation(int dir)
{

	switch (dir)
	{
		case left:
			// If you've turned from a column onto a row then you will maintain your positive/negative direction
			// Else if you were going 'south' along columns you will now be going 'west' along rows
			if (!travellingAlongRows && travellingDirection == -1)
			{
				travellingDirection = -1;
			}
			// Else if you were going 'north' along columns previously, you will now be going 'east' along rows
			else if (!travellingAlongRows && travellingDirection == 1)
			{
				travellingDirection = 1;
			}
			travellingAlongRows = !travellingAlongRows;
			break;
		case right:
			// If you've turned from a row onto a column then you will maintain your positive/negative direction
			// Else if you were going 'north' along columns you will now be going 'east' along rows
			if (!travellingAlongRows && travellingDirection == 1)
			{
				travellingDirection = -1;
			}
			// Else if you were going 'south' along columns previously, you will now be going 'west' along rows
			else if (!travellingAlongRows && travellingDirection == -1)
			{
				travellingDirection = 1;
			}

			travellingAlongRows = !travellingAlongRows;
			break;
		default:
			break;
	}
}

void Navigation::updatePosition()
{
	if (travellingAlongRows)
	{
		currentPosition[0] += travellingDirection;
	}
	else
	{
		currentPosition[1] += travellingDirection;
	}
}

bool Navigation::checkIfMapBuilt()
{
	if (mapIsBuilt == false)
	{
		if (travellingAlongRows == true)
		{
			travellingAlongRows = false;
			return false;
		}
		else
		{
			travellingAlongRows = true;
			travellingDirection = -1;
			mapIsBuilt = true;
			//buildMap();
			return true;
		}
	}
	return true;
}

void Navigation::updateMap()
{
	if (travellingAlongRows)
	{
		numColumns++;
	}
	else
	{
		numRows++;
	}
}

void Navigation::buildMap()
{
	gridMap[0][0] = 1;

	for (int i = 0; i < numColumns; ++i)
	{
		for (int j = 0; j < numRows; ++j)
		{
			gridMap[i][j] = 1;
		}
	}

	mapIsBuilt = true;
}

int Navigation::findPathTo(int targetLocation[])
{
	int columnDifference = targetLocation[0] - currentPosition[0];
	int rowDifference = targetLocation[1] - currentPosition[1];

	// We've arrived
	if (rowDifference == 0 && columnDifference == 0)
	{
		return stopped;
	}

	if (travellingAlongRows)
	{
		// Going west!
		if (travellingDirection == 1)
		{
			// It's in front of us
			if (columnDifference > 0)
			{
				return forward;
			}
			// It's behind us
			else if (columnDifference < 0)
			{
				return right;
			}
			else
			{
				// If we're travelling east and the row to go to is above us
				// Or if we're travelling west and the row to go to is below us
				if (rowDifference > 0 && travellingDirection == 1
					|| rowDifference < 0 && travellingDirection == -1)
				{
					return right;
				}
				else if (rowDifference > 0 && travellingDirection == -1
					|| rowDifference < 0 && travellingDirection == 1)
				{
					return right;
				}
			}
		}
		// Going east!
		else
		{
			// It's in front of us
			if (columnDifference < 0)
			{
				return forward;
			}
			// It's behind us
			else if (columnDifference > 0)
			{
				return right;
			}
			else
			{
				return right;
			}
		}
	}
	else
	{
		// Going north!
		if (travellingDirection == 1)
		{
			// It's in front of us
			if (rowDifference > 0)
			{
				return forward;
			}
			// It's behind us
			else if (rowDifference < 0)
			{
				return right;
			}
			else
			{
				return right;
			}
		}
		// Going south!
		else
		{
			// It's in front of us
			if (rowDifference < 0)
			{
				return forward;
			}
			// It's behind us
			else if (rowDifference > 0)
			{
				return right;
			}
			else
			{
				return right;
			}
		}

	}

	//if (travellingAlongRows && travellingDirection == 1)
	//{
	//	// The target is 'below' us
	//	if (rowDifference < 0)
	//	{
	//		return right;
	//	}
	//	// The target is 'above' us
	//	else if (rowDifference > 0)
	//	{
	//		// TODO: Implement left turns
	//		return right;
	//	}
	//	if (columnDifference < 0)
	//	{
	//		// See which direction we can turn to without falling off the grid
	//		if (gridMap[currentPosition[0] + 1][currentPosition[1]] == 1)
	//		{
	//			return right;
	//		}
	//		else
	//		{
	//			// TODO: Implement left turns
	//			return right;
	//		}
	//	}
	//}
	//else if (travellingAlongRows && travellingDirection == -1)
	//{
	//	// The target is 'below' us
	//	if (rowDifference < 0)
	//	{
	//		// TODO: Implement left turns
	//		return right;
	//	}
	//	// The target is 'above' us
	//	else if (rowDifference > 0)
	//	{
	//		return right;
	//	}
	//	// We have to turn around
	//	if (columnDifference > 0)
	//	{
	//		// See which direction we can turn to without falling off the grid
	//		if (gridMap[currentPosition[0] + 1][currentPosition[1]] == 1)
	//		{
	//			// TODO: Implement left turns
	//			return right;
	//		}
	//		else
	//		{
	//			return right;
	//		}
	//	}
	//}
	//else if (!travellingAlongRows && travellingDirection == 1)
	//{
	//	// The target is 'right' of us
	//	if (columnDifference < 0)
	//	{
	//		return right;
	//	}
	//	// The target is 'left' of us
	//	else if (columnDifference > 0)
	//	{
	//		// TODO: Implement left turns
	//		return right;
	//	}
	//	if (rowDifference < 0)
	//	{
	//		// See which direction we can turn to without falling off the grid
	//		if (gridMap[currentPosition[0]][currentPosition[1] + 1] == 1)
	//		{
	//			// TODO: Implement left turns
	//			return right;
	//		}
	//		else
	//		{
	//			return right;
	//		}
	//	}
	//}
	//else if (!travellingAlongRows && travellingDirection == -1)
	//{
	//	// The target is 'right' of us
	//	if (columnDifference < 0)
	//	{
	//		// TODO: Implement left turns
	//		return right;
	//	}
	//	// The target is 'left' of us
	//	else if (columnDifference > 0)
	//	{
	//		return right;
	//	}
	//	if (rowDifference > 0)
	//	{
	//		// See which direction we can turn to without falling off the grid
	//		if (gridMap[currentPosition[0]][currentPosition[1] + 1] == 1)
	//		{
	//			return right;
	//		}
	//		else
	//		{
	//			// TODO: Implement left turns
	//			return right;
	//		}
	//	}
	//}
	return forward;
}