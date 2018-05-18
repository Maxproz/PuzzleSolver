#pragma once



#ifndef Region_H

#define Region_H

#include <set> // need for data member 

enum class State; // forward declare enum.  Just tells compiler that enum class State exists. (its inside Cell.h)
class Cell;

// TODO: Add a bool member variable that will be set to true after a number region is complete and had all of its adjacent unknowns set to black.
// TODO: Make a function that friends std::ostream<< and will print all the cells in that region and also tell us if its complete or not.

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

	std::set<Cell*>::const_iterator Begin() const;
	std::set<Cell*>::const_iterator End() const;
	int RegionSize() const;

	std::set<Cell*>::const_iterator UnknownsBegin() const;
	std::set<Cell*>::const_iterator UnknownsEnd() const;
	int UnknownsSize() const;

	bool IsWhite() const;
	bool IsBlack() const;
	bool IsNumbered() const;

	int GetNumber() const;

	// once a cell has been confirmed enough to be added to a regions set of contained cooridnates
	// - we should need to remove it, so we don't need a remove function for that std::set of cell pointers

	bool Contains(Cell* InCellPtr) const;

	void EraseUnknown(Cell* InUnknownCell);


	// We use these two functions when fusing two regions together
	template <typename InIter>
	void Insert(InIter first, InIter last)
	{
		m_Cells.insert(first, last);
	}

	template <typename InIt>
	void UnknownsInsert(InIt first, InIt last)
	{
		m_Unknowns.insert(first, last);
	}


	friend std::ostream& operator<<(std::ostream& os, const Region& RHS);

private:
	std::set<Cell*> m_Unknowns;
	std::set<Cell*> m_Cells; // The cells that make up this region.
	State m_RegionState; // The state of the entire region

};


std::ostream& operator<<(std::ostream& os, const Region& RHS);


#endif // !Region_H