// PuzzleSolver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// https://en.wikipedia.org/wiki/Nurikabe_(puzzle)



// Test how I will setup my multidimensional array

#include <vector>
#include <iterator>
#include <iostream>
#include <sstream>   // for TimerCode ~ ostringstream variable
#include "Windows.h" // for TimerCode ~ LARGE_INTEGER and QueryPerformanceCounter(&li);

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


// TODO: Get the code timer working for the solving code in main().

long long counter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}

long long frequency() 
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	return li.QuadPart;
}

string format_time(const long long start, const long long finish)
{
	ostringstream oss;

	if ((finish - start) * 1000 < frequency())
	{
		oss << (finish - start) * 1000000.0 / frequency() << " microseconds";
	}
	else if (finish - start < frequency())
	{
		oss << (finish - start) * 1000.0 / frequency() << " milliseconds";
	}
	else
	{
		oss << (finish - start) * 1.0 / frequency() << " seconds";
	}

	return oss.str();
}


int main()
{

	const long long start = counter();

	// Create our Grid and correctly initalize it.
	Grid GameBoard(Width, Height, NumberedIslandCells);
	
	// TODO: Add a function (something like GameBoard.Solve(); )
	// - that I can use to time how long it takes to solve a puzzle
		// while (g.solve() == Grid::KEEP_GOING) {}

	const long long finish = counter();

	// As of right now all I am timing is how long it takes to construct the puzzle (allocate Cells on Heap etc..)
	cout << "Timed Code Result: " << ": " << format_time(start, finish) << ", " << endl;


	GameBoard.PrintGrid(); // Working


	//cout << endl;
	//GameBoard.PrintAllCellsInAllRegions(); // Working
	//cout << endl;

	//cout << endl;
	//GameBoard.PrintAllUnknownsInAllRegions();
	//cout << endl;



	return 0;
}