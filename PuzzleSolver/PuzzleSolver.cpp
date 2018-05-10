// PuzzleSolver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// https://en.wikipedia.org/wiki/Nurikabe_(puzzle)



// Test how I will setup my multidimensional array

#include <vector>
#include <iterator>
#include <iostream>

#include "Grid.h"
#include "Coordinate2D.h"
#include "Cell.h"

using namespace std;


int Width = 10;
int Height = 9;

// TODO: Filter into function
vector<pair<Coordinate2D, int> > NumberedIslandCells
{
	make_pair(Coordinate2D(0, 0), 2),
	make_pair(Coordinate2D(0, 6), 2),
	make_pair(Coordinate2D(1, 2), 2),
	make_pair(Coordinate2D(1, 8), 1),
	make_pair(Coordinate2D(2, 5), 2),
	make_pair(Coordinate2D(3, 6), 4),
	make_pair(Coordinate2D(4, 2), 7),
	make_pair(Coordinate2D(6, 1), 2),
	make_pair(Coordinate2D(6, 4), 3),
	make_pair(Coordinate2D(6, 8), 2),
	make_pair(Coordinate2D(7, 5), 3),
	make_pair(Coordinate2D(8, 4), 3),
	make_pair(Coordinate2D(8, 8), 4),
	make_pair(Coordinate2D(9, 0), 2)
};



int main()
{

	// TODO: Get a better system for printing the board working.
	// TODO: Filter into function
	// Create our Grid and correctly initalize it.
	Grid GameBoard(Width, Height, NumberedIslandCells);
	GameBoard.PrintGrid(); 


	return 0;
}