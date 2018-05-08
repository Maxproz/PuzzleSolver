========================================================================
    CONSOLE APPLICATION : PuzzleSolver Project Overview
========================================================================


// Puzzle Solver for Nurikabe 
/* https://en.wikipedia.org/wiki/Nurikabe_(puzzle) */

// http://homepages.e3.net.nz/~djm/cppcontainers.html
// Container picker including unordered_ options
// https://i.stack.imgur.com/G70oT.png 

// http://homepages.e3.net.nz/~djm/cppiterators.html
// http://homepages.e3.net.nz/~djm/cppstrings.html


// Step 1: Define the problem (I want a better way to keep track of my friends’ phone numbers.)

// I want to make a solver for nurikabe puzzles.




// Step 2: Collect requirements For example:
// (Phone numbers should be saved, so they can be recalled later)

// Our program should be able to dectect if we gave it an invalid puzzle as input
// Once we have given our unsolved puzzle as input, we shouldn't need any other user input to solve the puzzle.
// Our program needs to have a system where it "tests" the result of marking cells
// - (Marking unknown cells white may force other cells to be black and vice versa(unknown to black test also), 
// - (because a section of black would be isolated when they need to connect))
// No blind guessing should be required 
// Our program needs to output how long it took to solve a puzzle.
// Our Program should output to cout what our board looks like before it was solved and after it was solved.




// Step 3: Define your tools, targets, and backup plan
// (Defining your testing/feedback/release strategy)
// (Determining how you will back up your code)

// Plan to write the code and test it going task by task.
// No backup storage plan yet.




// Step 4: Break hard problems down into easy problems

// Example:
// We want to ~ 
// Write report on carrots

// Write report on carrots
//	- Do research on carrots
//  - Write outline
//  - Fill in outline with detailed information about carrots
//  - Add table of contents

// Write report on carrots
//  - Do research on carrots
//     - Go to library and get book on carrots
//     - Look for information about carrots on internet
//     - Take notes on relevant sections from reference material
//  - Write outline
//     - Information about growing
//     - Information about processing
//     - information about nutrition


// Example 2 List Outline
/* 
Solving the problem:

(get from bed to work)

Make a list of the things you do in the morning going from bed to work.

Pick out clothes
Get dressed
Eat breakfast
Drive to work
Brush your teeth
Get out of bed
Prepare breakfast
Get in your car
Take a shower

Organize it into a hierarchy like so

Get from bed to work
	Bedroom things
		Get out of bed
		Pick out clothes
		Get dressed
	Bathroom things
		Take a shower
		Brush your teeth
	Breakfast things
		Prepare breakfast
		Eat breakfast
	Transportation things
		Get in your car
		Drive to work

*/


//	Solve A NuriKabe Puzzle
//	What are some things I need to do to solve a nurikabe puzzle

//	setup the board (create a grid class)
//	create the cells (do I want a cell class?)
//	add the cells to the board.
//		add a multidimensional array of pointers member "cells" to the board class
//		in the constructor of the board allocate the cells on the heap and add them to the boards cells container
//	logic for tracking the color of a cell (black, white, unknown)
//		enum in the cell or board class?
//	logic for tracking which cells are numbered (islands)
//	logic for tracking which cells are connected
//  logic for checking if a given coordinate pair is valid.
//	Make sure there is only one numbered cell in an island
//  Make sure there is no pools (2x2 black cells)
//	Set all cells are unknown at the start
//  Set the numbered cells as numbered 
//	Keep connected cells (coordinate pairs) black/white in a set (so coordinate pairs are unique)
//  Figure out if I want to use a "regions" class for connected black/white cells.
//	Setup the logic for a coordinate pair on the board 
//		This is an implementation detail that I just need to document for myself.
//	Make logic for a way to test if marking a cell a color will force another cell to be a color.
//		Create a copy (temp) of the board in that function and use that to test 
//  A way to iterate over the container of a connected areas coordinate pairs.
//	Keep a container that the connected cells have that keeps track of all the unknown cells they are surrounded by


//  Reserach strategy solving tips on wikipedia 
//		mark cells adjacent to two or more numbers as black.


// TODO: finish filtering these into tasks

Once an island is "complete"—that is, it has all the white cells its number requires—all cells that share a side with it must be black. Obviously, any cells marked with '1' at the outset are complete islands unto themselves, and can be isolated with black at the beginning.

Whenever three black cells form an "elbow"—an L-shape—the cell in the bend (diagonally in from the corner of the L) must be white. (The alternative is a "pool", for lack of a better term.)

All black cells must eventually be connected. If there is a black region with only one possible way to connect to the rest of the board, the sole connecting pathway must be black.
	Corollary: there cannot be a continuous path, using either vertical, horizontal or diagonal steps, of white cells from one cell lying on the edge of the board to a different cell like that, that encloses some black cells inside, because otherwise, the black cells won't be connected.

All white cells must eventually be part of exactly one island. If there is a white region that does not contain a number, and there is only one possible way for it to connect to a numbered white region, the sole connecting pathway must be white.

Some puzzles will require the location of "unreachables"—cells that cannot be connected to any number, being either too far away from all of them or blocked by other numbers. Such cells must be black. Often, these cells will have only one route of connection to other black cells or will form an elbow whose required white cell (see previous bullet) can only reach one number, allowing further progress.


//		If an island of size N already has N-1 white cells identified, and there are only two remaining cells to choose from, and those two cells touch at their corners, then the cell between those two that is on the far side of the island must be black.	
//			function to test (If an island of size N already has N-1 white cells identified) ~ bool function(Island)
//			function that takes a coordinate pair and a functor and runs the functor on all valid neighbors
//				function for determinting what a valid neighbor is to a coordinate pair.
//			function that can iterate over the collection of coordinate pairs of an island that calls 
//			function that determines if an island only has two possible cells left to choose from and only needs one more cell.
//			function that determines if two cells touch at their corners. (diagional to eachother)
//			function that will mark the cell that is between two diagional cells as black
//		A function that looks for 2v2 pools of black and if it find an L of 3 it marks the last cell white.
//		A function that tests if only two islands can connect to a white cell.
//		A function that tests if an island will have no unidentified cells left after connecting to a white cell.
//		A function that sets a white cell to an island cell (when a numbered cell connects to a white cell)
//		A function that tests if two islands would connect to the same white cell at a 90 degree angle
//      A function that sets an unknown cell that is diagionally between two islands to black.


/* Figure out how to test this condition 

Undetermined cells adjacent to a straight row (or a straight column) of black cells can be tested for
being black, because if they are black it will form two elbows, 
and there will be two adjacent white cells which need to be reachable from the islands. 
If they can not be fulfilled within the constraints, it means the cell that was
probed for blackness must be white.
*/








// Step 5: Figure out the sequence of events

/* 


int main()
{
	getOutOfBed();
	pickOutClothes();
	takeAShower();
	getDressed();
	prepareBreakfast();
	eatBreakfast();
	brushTeeth();
	getInCar();
	driveToWork();
}


If we were writing a calculator, we might do things in this order:

Get first number from user
Get mathematical operation from user
Get second number from user
Calculate result
Print result
*/










// Step 6: Figure out the data inputs and outputs for each task
// (Make function prototypes)
// int calculateResult(int input1, int op, int input2);

// This should be possible, since you have a sequence of events, if you solve them in order.
// You will know your inputs and outputs.

//  If you already have the input data from a previous step, that input data will become a parameter. 
// If you are calculating output for use by some other function, that output will generally become a return value.












// Step 7: Write the task details
// In this step, for each task, you will write its actual implementation (implementation for the function)
// If confused break it into smaller tasks.











// Step 8: Connect the data inputs and outputs
// result is a temporary value used to transfer the output of calculateResult()
// into an input of printResult()
// int result = calculateResult(input1, op, input2); // temporarily store the calculated result in result






