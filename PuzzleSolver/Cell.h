#pragma once



#ifndef Cell_H

#define Cell_H

#include "Coordinate2D.h" // data member for the cells position on the grid

// Cell is a composition that is made up of parts
// part 1: state ~ enum class? white, black, unknown, numbered
// part 2: Position on the grid ~ Coordinate2D
// part 3: The same type of cells it is connected to (region) it is apart of. (shared_ptr) (maybe use a raw pointer)


class Cell
{
public:
	Cell() = delete;
	constexpr Cell(Coordinate2D Pos) : m_GridPosition(Pos) {}
	constexpr Cell(int x, int y) : m_GridPosition(Coordinate2D(x, y)) {}
	~Cell() = default;


private:
	Coordinate2D m_GridPosition{ 0, 0 };

};

#endif // !Cell_H