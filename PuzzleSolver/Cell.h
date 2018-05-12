#pragma once



#ifndef Cell_H

#define Cell_H

#include "Coordinate2D.h" // data member for the cells position on the grid

// Cell is a composition that is made up of parts
// part 1: state ~ enum class? white, black, unknown, numbered
// part 2: Position on the grid ~ Coordinate2D
// part 3: The same type of cells it is connected to (region) it is apart of. (shared_ptr) (maybe use a raw pointer) ?? needed??

class Region;

enum class State : int
{
	Unknown = -3,
	White = -2,
	Black = -1
	// Numbered cells are represented as an int as > 0
};

class Cell
{
public:
	Cell() = delete;
	Cell(Coordinate2D Pos, State InState) : m_GridPosition(Pos), m_State(InState) {}
	
	//Cell(int X, int Y, State InState) : m_GridPosition(Coordinate2D(X, Y)), m_State(InState) {}
	~Cell() = default;
	Cell(const Cell&) = default;
	Cell(Cell&&) = default;


	Coordinate2D GetPosition() const { return m_GridPosition; }
	State GetState() const { return m_State; }
	Region* GetRegion() const; 

	void SetState(State NewState) { m_State = NewState; }
	void SetRegion(Region* NewRegion);

private:
	Coordinate2D m_GridPosition{ 0, 0 };
	State m_State{ State::Unknown };
	Region* m_Region{ nullptr }; // The region that this cell is apart of. (Will be nullptr for an unknown cell)

};

#endif // !Cell_H