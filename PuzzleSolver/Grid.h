#pragma once



#ifndef Grid_H

#define Grid_H


#include <vector> // Used as a data member
#include <memory> // Used as a data member


// Grid is a composition of the parts...
// Part 1: A 2D array of Cell pointers (lets try unique then shared if it doesnt work)
// Part 2: An container of pointers to regions (unique_ptr for now) that control the logic of connecting cells (numbered, whites) 
//				- Regions should have containers of the unknown cells that they are surrounded by.
//				- Regions should have containers of the Cells that are apart of that region

// TODO: Each region should keep track of its state, white, black or numbered.
// TODO: This will be seperate from a "Cells" state of being (white, black, unknown, numbered)
// So each Cell will know if its white, black, numbered, or unknown.
// And each Region will hold raw pointers to those cells (maybe convert to shared_ptr later) and the entire region will have its own state.
// - This is how we will know if white cells are connected to numbered cells.
// - Each region is black, white, or numbered. This allows us to
// - remember when white cells are connected to numbered cells,
// - as the whole region is marked as numbered.
// - Each region keeps track of the coordinates that it occupies.
// - Each region also keeps track of the unknown cells that it's surrounded by.

// TODO TEST: After Marking a cell black or white first create a new region using it.
// TODO TEST: After marking a cell (black or white), we should iterate all of its valid neighbors and attempt to fuse regions

class Cell; // need name for data member
// class Coordinate2D; // TODO: Remove if not needed. (Think I need this name for func parameters later on)


class Grid
{
public:

	// TODO: Think about which type of operations I will need here

private:
	// TODO: m_cells should be allocated in the constructor of grid.
	std::vector<std::vector<std::unique_ptr<Cell > > > m_Cells;

};



#endif // !Grid_H