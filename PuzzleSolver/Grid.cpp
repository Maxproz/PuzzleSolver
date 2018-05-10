
#include "Grid.h"

#include <iostream>

#include "Cell.h"


using namespace std;


// Unused atm | m_total_black(width * height)


Grid::Grid(int Width, int Height, std::vector<std::pair<Coordinate2D, int>> NumberedCellLocations) :
	m_Width{ Width }, m_Height{ Height }, m_Cells(), m_Regions()
{
	// Validate width and height.
	if (Width < 1) { throw runtime_error("RUNTIME ERROR: Grid::Grid() - width must be at least 1."); }
	if (Height < 1) {throw runtime_error("RUNTIME ERROR: Grid::Grid() - height must be at least 1."); }

	// vector of vectors of Sprites is what you're looking for
	std::vector<std::vector<std::unique_ptr<Cell>>>   map;

	// first x
	for (int c = 0; c < m_Width; ++c)
	{
		// No need to access via index of c. Just append to the column vector itself.
		std::vector<std::unique_ptr<Cell>> col;

		// then y
		for (int r = 0; r < m_Height; ++r)
		{
			col.push_back(std::unique_ptr<Cell>(new Cell(Coordinate2D(c, r), State::Unknown)));
		}

		// Now add the column vector.
		map.push_back(std::move(col));
	}


	m_Cells = std::move(map);


	for (int x = 0; x < Width; ++x)
	{
		for (int y = 0; y < Height; ++y)
		{
			for (const auto& Island : NumberedCellLocations)
			{
				if (x == Island.first.GetX() && y == Island.first.GetY())
				{
					// Set an individual cells state from unknown to an Island if we were told it was supposed to be numbered
					m_Cells[x][y]->SetState(static_cast<State>(Island.second));
					
					// If This is changing off of unknown, we need to also create a region for it.
					AddRegion(m_Cells[x][y].get());
				}
			}
		}
	}

}

// Does a deep copy of the unique_ptr variables, so both Grids have pointers to different Cells and Regions on the Heap
Grid::Grid(const Grid& Copy) :
	m_Width{ Copy.m_Width }, m_Height{ Copy.m_Height }, m_Cells(), m_Regions()
{
	// vector of vectors of Sprites is what you're looking for
	std::vector<std::vector<std::unique_ptr<Cell>>>   Temp;

	// first x
	for (int c = 0; c < m_Width; ++c)
	{
		// No need to access via index of c. Just append to the column vector itself.
		std::vector<std::unique_ptr<Cell>> col;

		// then y
		for (int r = 0; r < m_Height; ++r)
		{
			col.push_back(std::unique_ptr<Cell>(new Cell(Coordinate2D(c, r), Copy(Coordinate2D(c,r))->GetState())));
		}

		// Now add the column vector.
		Temp.push_back(std::move(col));
	}

	m_Cells = std::move(Temp);

	std::set<std::unique_ptr<Region>>  TempRegionVec;

	for (auto AllRegCopy = Copy.m_Regions.begin(); AllRegCopy != Copy.m_Regions.end(); ++AllRegCopy)
	{
		auto RegionPtr = unique_ptr<Region>(new Region((**AllRegCopy)));
		TempRegionVec.insert(std::move(RegionPtr));
		//TempRegionVec.insert(unique_ptr<Region>(new Region((**AllRegCopy))));
	}

	m_Regions = std::move(TempRegionVec);
}

Grid& Grid::operator=(const Grid& CopyAssign)
{
	Grid TempGrid(CopyAssign);
	this->swap(TempGrid);
	return *this;
}

Grid::Grid(Grid&& Move)
{
	this->swap(Move);
}

Grid& Grid::operator=(Grid&& MoveAssign)
{
	this->swap(MoveAssign);
	return *this;
}


// NOTE: If we erase a region from the set of all regions (for example when merging 2 regions)
// - we need to make sure we set the region pointer to nullptr on all the cells that were reference the erased region.
// - otherwise it will be hard to find that bug. probably.


// Called initially in the constructor for all numbered cells
// It will probably be called in other locations later.
void Grid::AddRegion(Cell* InCell) // non-owning param
{
	set<Cell*> Unknowns;
	// //	insert_valid_unknown_neighbors(unknowns, x, y);

	auto NewRegion = unique_ptr<Region>(new Region(InCell, Unknowns));

	// Reference it in the cell
	InCell->SetRegion(NewRegion.get());

	// Add it to the set of all regions.
	// The set of all regions will own this.
	m_Regions.insert(std::move(NewRegion));
}



Cell* Grid::operator()(const Coordinate2D& Pos)
{
	// TODO: if is_valid(Pos)
	return m_Cells[Pos.GetX()][Pos.GetY()].get();
}

const Cell* Grid::operator()(const Coordinate2D& Pos) const
{
	// TODO: if is_valid(Pos)
	return m_Cells[Pos.GetX()][Pos.GetY()].get();
}

void Grid::swap(Grid& other) 
{
	std::swap(m_Width, other.m_Width);
	std::swap(m_Height, other.m_Height);
	std::swap(m_Cells, other.m_Cells);
	std::swap(m_Regions, other.m_Regions);
}


// TODO: This is a temp print function to test output
void Grid::PrintGrid() const
{
	for (int y = 0; y < m_Height; ++y)
	{
		for (int x = 0; x < m_Width; ++x)
		{
			if (static_cast<int>(operator()(Coordinate2D(x, y))->GetState()) < 0) // is not a numbered area
			{
				cout << "X" << " ";
			}
			else
			{
				cout << static_cast<int>(operator()(Coordinate2D(x, y))->GetState()) << " ";
			}
		}
		cout << endl; // End of the row or column idk..

	} cout << endl << endl;

}


