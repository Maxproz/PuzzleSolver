#pragma once



#ifndef Region_H

#define Region_H

#include <set> // need for data member 

enum class State; // forward declare enum.  Just tells compiler that enum class State exists. (its inside Cell.h)
class Cell;


class Region
{
public:

	// To make a region you are given a Cell on the Grid 
	// To create a valid region you need to store the given Cells (State, Coordinates, Surrounding Unknowns)

	Region() = delete;
	Region(Cell* InCell, const std::set<Cell*>& InUnknowns);
	~Region() = default; // None of these Cell pointers should be owners of the Cells


	// TODO: Need to make iterator class to make accessing the std::sets easier
	std::set<Cell*>& GetCellsInRegion() { return m_Cells; }
	std::set<Cell*>& GetUnknownsAroundRegion() { return m_Unknowns; }

private:

	

private:
	std::set<Cell*> m_Unknowns;
	std::set<Cell*> m_Cells; // The cells that make up this region.
	State m_RegionState; // The state of the entire region

};




#endif // !Region_H