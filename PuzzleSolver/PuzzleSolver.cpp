// PuzzleSolver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// https://en.wikipedia.org/wiki/Nurikabe_(puzzle)



// Test how I will setup my multidimensional array

#include <vector>
#include <iterator>
#include <iostream>

using namespace std;

vector<vector<int>> Grid;

// Need a grid class that will define what a region is, and also store a vector<vector<shared_ptr<Cells>>


int main()
{

	int width = 10;
	int height = 9;


	Grid.resize(width, vector<int>(height));
	
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			Grid[x][y] = 0;
		}
	}

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			cout << Grid[x][y] << " ";
		}
		cout << endl;

	} cout << endl << endl;

	return 0;
}