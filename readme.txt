Author: Dustin Linnington
Last Updated: 2017/09/28

This project is intended to showcase the use of belief revision in Makeblock robots using a grid for the robots to drive on and sensors equipped on the robots (IR, Ultrasonic and Sound). 

The current state of the project is a RobotController written in Arduino that reads inputs from the robot's sensors and then activates the motors in the wheels, a Navigation Library that can map out the size of the grid the robot is on and give it directions to reach a certain target in the grid, and separate Belief Revision/Satisfiability checking programs intended to be rewritten in C/C++ for use on each robot.

The known issues for the project are:
- Navigation creates the correct map size, but building a path between points leads to errors
- Turning the robot left/right was extremely difficult to do properly as the detection with only 2 IR sensors on each robot wasn't enough to line the robot up on the grid lines.
- Another IR sensor has been added to 2 robots, but the code to use the information from them is not written.
- Belief revision needs to be written as a C library for the robots to use.
- Robots need to implement detection so as not to bump in to eachother on the grid.