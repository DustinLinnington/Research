#include <stdlib.h>
#include "Arduino.h"
#include "navigation.h"

Navigation::Navigation()
{

};

const int MAX_GRID_ROWS = 100;
const int MAX_GRID_COLUMNS = 100;

int gridMap[MAX_GRID_ROWS][MAX_GRID_COLUMNS] = { 0 };
int currentPosition[2] = { 0 };
enum Direction
{
	backward = 0,
	forward = 1,
	right = 2,
	left = 3
};
enum BehaviourState
{
	exploring = 0,
	navigating = 1,
	guarding = 2,
	patrolling = 3
};
int currentDirection = forward;

int numColumns = 1;
int numRows = 1;
bool mapIsBuilt = false;

bool travellingAlongRows = true;
// 1 for going left/up, -1 for going right/down
int travellingDirection = 1;

int Navigation::intersectionReached(int currentBehaviour, int targetLocation[])
{
	int dir = 1;
	switch (currentBehaviour)
	{
		case exploring:
			dir = getRandomDirection();
			updatePosition(dir);
			return dir;
			break;
		case mapping:
			updateMap();
			dir = forward;
			updatePosition(dir);
			return dir;
			break;
		case guarding:
			dir = findPathTo(targetLocation);
			updatePosition(dir);
			return dir;
			break;
		case patrolling:
			break;
		default:
			break;
	}
	return forward;
}

int Navigation::getRandomDirection()
{
	return rand() % 4 + 1;
}

void Navigation::updatePosition(int dir)
{
	switch (dir)
	{
		case forward:
			if (travellingAlongRows)
			{
				currentPosition[0] += travellingDirection;
			}
			else
			{
				currentPosition[1] += travellingDirection;
			}
			break;
		case backward:
			if (travellingAlongRows)
			{
				currentPosition[0] -= travellingDirection;
			}
			else
			{
				currentPosition[1] -= travellingDirection;
			}
			break;
		case left:
			travellingAlongRows = !travellingAlongRows;

			// If you've turned from a column onto a row then you will maintain your positive/negative direction

			// Else if you were going 'right' along rows you will now be going 'up' along columns
			if (!travellingAlongRows && travellingDirection == -1)
			{
				travellingDirection = 1;
			}
			// Else if you were going 'left' along rows previously, you will now be going 'down' along columns
			else if (!travellingAlongRows && travellingDirection == 1)
			{
				travellingDirection = -1;
			}
			currentPosition[1] += travellingDirection;
			break;
		case right:
			travellingAlongRows = !travellingAlongRows;

			// If you've turned from a row onto a column then you will maintain your positive/negative direction

			// Else if you were going 'up' along columns you will now be going 'right' along rows
			if (travellingAlongRows && travellingDirection == 1)
			{
				travellingDirection = -1;
			}
			// Else if you were going 'down' along columns previously, you will now be going 'left' along rows
			else if (travellingAlongRows && travellingDirection == -1)
			{
				travellingDirection = 1;
			}
			currentPosition[0] += travellingDirection;
			break;
		default:
			break;
	}
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

bool Navigation::checkIfMapBuilt()
{
	if (!mapIsBuilt)
	{
		if (travellingAlongRows)
		{
			travellingAlongRows = false;
		}
		else
		{
			buildMap();
			return true;
		}
	}

	return false;
}

void Navigation::buildMap()
{
	gridMap[0][0] = 1;

	for (int i = 0; i < numRows; ++i)
	{
		for (int j = 0; j < numColumns; ++j)
		{
			gridMap[i][j] = 1;
		}
	}

	mapIsBuilt = true;
}

int Navigation::findPathTo(int targetLocation[])
{
	int rowDifference = targetLocation[0] - currentPosition[0];
	int columnDifference = targetLocation[1] - currentPosition[1];

	if (travellingAlongRows && travellingDirection == 1)
	{
		// The target is 'below' us
		if (rowDifference < 0)
		{
			return right;
		}
		// The target is 'above' us
		else if (rowDifference > 0)
		{
			return left;
		}
		if (columnDifference < 0)
		{
			// See which direction we can turn to without falling off the grid
			if (gridMap[currentPosition[0] + 1][currentPosition[1]] == 1)
			{
				return right;
			}
			else
			{
				return left;
			}
		}
	}
	else if (travellingAlongRows && travellingDirection == -1)
	{
		// The target is 'below' us
		if (rowDifference < 0)
		{
			return left;
		}
		// The target is 'above' us
		else if (rowDifference > 0)
		{
			return right;
		}
		// We have to turn around
		if (columnDifference > 0)
		{
			// See which direction we can turn to without falling off the grid
			if (gridMap[currentPosition[0] + 1][currentPosition[1]] == 1)
			{
				return left;
			}
			else
			{
				return right;
			}
		}
	}
	else if (!travellingAlongRows && travellingDirection == 1)
	{
		// The target is 'right' of us
		if (columnDifference < 0)
		{
			return right;
		}
		// The target is 'left' of us
		else if (columnDifference > 0)
		{
			return left;
		}
		if (rowDifference < 0)
		{
			// See which direction we can turn to without falling off the grid
			if (gridMap[currentPosition[0]][currentPosition[1] + 1] == 1)
			{
				return left;
			}
			else
			{
				return right;
			}
		}
	}
	else if (!travellingAlongRows && travellingDirection == -1)
	{
		// The target is 'right' of us
		if (columnDifference < 0)
		{
			return left;
		}
		// The target is 'left' of us
		else if (columnDifference > 0)
		{
			return right;
		}
		if (rowDifference > 0)
		{
			// See which direction we can turn to without falling off the grid
			if (gridMap[currentPosition[0]][currentPosition[1] + 1] == 1)
			{
				return right;
			}
			else
			{
				return left;
			}
		}
	}
	return forward;
}