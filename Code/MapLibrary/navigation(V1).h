#ifndef navigation_h
#define navigation_h
#include "Arduino.h"

class Navigation
{
	public:
		Navigation();
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
			mapping = 1,
			guarding = 2,
			patrolling = 3
		};
		int currentDirection;
		int currentPosition[];

		int intersectionReached(int currentBehaviour, int targetLocation[]);
		bool checkIfMapBuilt();

	private:
		int getRandomDirection();
		void updatePosition(int dir);
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