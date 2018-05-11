
#include "Grid.h"

#include <iostream>

#include "Cell.h"
#include "Region.h"

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
	//	insert_valid_unknown_neighbors(unknowns, x, y);
	For_All_Valid_Unknown_Neighbors(InCell, 
		[&Unknowns](Cell* NeighborCell) -> auto { Unknowns.insert(NeighborCell); });
	// TODO: Do this next

	auto NewRegion = unique_ptr<Region>(new Region(InCell, Unknowns));

	// Reference it in the cell
	InCell->SetRegion(NewRegion.get());

	// Add it to the set of all regions.
	// The set of all regions will own this.
	m_Regions.insert(std::move(NewRegion));
}



Cell* Grid::operator()(const Coordinate2D& Pos)
{
	if (IsValid(Pos))
	{
		return m_Cells[Pos.GetX()][Pos.GetY()].get();
	}

	return nullptr;
}

const Cell* Grid::operator()(const Coordinate2D& Pos) const
{
	if (IsValid(Pos))
	{
		return m_Cells[Pos.GetX()][Pos.GetY()].get();
	}

	return nullptr;
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
	// For printing the Grid 
	// W == White Cell
	// B == Black Cell
	// X == Unknown Cell
	// A Number (1 - 9) is the number of a numbered Cell
	for (int y = 0; y < m_Height; ++y)
	{
		for (int x = 0; x < m_Width; ++x)
		{
			auto CurrentCell = operator()(Coordinate2D(x, y));

			if (CurrentCell->GetState() == State::White)
			{
				cout << "W" << " ";
			}
			else if (CurrentCell->GetState() == State::Black)
			{
				cout << "B" << " ";
			}
			else if (CurrentCell->GetState() == State::Unknown)
			{
				cout << "X" << " ";
			}
			else
			{
				// Prints the number of a numbered cell
				cout << static_cast<int>(CurrentCell->GetState()) << " ";
			}
		}
		cout << endl; // End of the row or column idk..

	} cout << endl << endl;

}

void Grid::PrintAllCellsInAllRegions() const
{
	int RegionCounter = 1;
	for (const auto& region : m_Regions)
	{
		std::cout << "Region " << RegionCounter << ": ";
		for (auto beg = (*region).Begin(); beg != region->End(); ++beg)
		{
			cout << (*beg)->GetPosition() << endl;
		}
		RegionCounter++;
	}
}

void Grid::PrintAllUnknownsInAllRegions() const
{
	int RegionCounter = 1;
	for (const auto& region : m_Regions)
	{
		std::cout << "Region " << RegionCounter << " Unknowns" << endl;
		for (auto beg = (*region).UnknownsBegin(); beg != region->UnknownsEnd(); ++beg)
		{
			cout << (*beg)->GetPosition() << endl;
		}
		RegionCounter++;
	}
}



void Grid::SolvePuzzle()
{
	// Start by solving using the wikipedia steps
	// - Just organize/refactor using your best judgement

	// Step 1 
	// mark cells adjacent to two or more numbers as black.
	SolveCellsWithTwoAdjacentNumberedCells(); // Verified working.

	// Step 2 
	// Once an island is "complete"—that is, it has all the white cells its number requires—all cells that share a side with it must be black.Obviously, any cells marked with '1' at the outset are complete islands unto themselves, and can be isolated with black at the beginning.
	SolveUpdateCompleteIslands();


	// TODO: OnHold 2x2 black pool verifiction 
	// - OnHold until we have a good black pool to test on our board.
	//		- Whenever three black cells form an "elbow"—an L - shape—the cell in the bend(diagonally in from the corner of the L) must be white. (The alternative is a "pool", for lack of a better term.)
	//			- A function that looks for 2v2 pools of black and if it find an L of 3 it marks the last cell white.

	// TODO: Step 3
	// All black cells must eventually be connected. If there is a black region with only one possible way to connect to the rest of the board, the sole connecting pathway must be black.
	//	Corollary: there cannot be a continuous path, using either vertical, horizontal or diagonal steps, of white cells from one cell lying on the edge of the board to a different cell like that, that encloses some black cells inside, because otherwise, the black cells won't be connected.
	//	A function that checks if all black cells are connected.
	//	A function that checks if there is an unknown cell that needs to be black to connect 2 black squares(that are diagional ? )
	//	A function that sets a cell that needs to be a connecting black square to black.
	

}

set<Region*> Grid::GetAllNumberedRegions() const
{
	set<Region*> Result;

	for (const auto& Reg : m_Regions)
	{
		if (Reg->IsNumbered())
		{
			Result.insert(Reg.get());
		}
	}

	return Result;
}

// A complete region is region->Number() == region->RegionSize()
void Grid::UpdateCompleteRegions(set<Region*> InNumberedRegions) 
{
	for (const auto& Reg : InNumberedRegions)
	{
		if (Reg->GetNumber() == Reg->RegionSize())
		{
			// The numbered region is complete
			// For each cell in the region mark all valid adjacent cells of the cell in that region black
			SetStateOfAllCellsInRegion(Reg, State::Black);
		}
	}
}


void Grid::SetStateOfAllCellsInRegion(Region* InRegion, const State& InState)
{
	if (InState == State::Black || InState == State::White)
	{
		if (InState == State::Black)
		{
			for (auto& CellInRegion : InRegion->GetCellsInRegion())
			{
				For_All_Valid_Neighbors(CellInRegion,
					[this](Cell* NeighborCell) -> auto
				{
					MarkBlack(NeighborCell);
				});
			}
		}
		if (InState == State::White)
		{
			// TODO: Needs implemented
		}
	}
	else
	{
		throw std::logic_error("Trying to set a state that is not black or white");
	}

}

void Grid::SolveUpdateCompleteIslands()
{
	auto NumberedRegions = GetAllNumberedRegions();
	UpdateCompleteRegions(NumberedRegions);
}

void Grid::SolveCellsWithTwoAdjacentNumberedCells()
{
	// For every cell on the board....
	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			auto TestedCell = operator()(Coordinate2D(x, y));

			if (TestedCell->GetState() != State::Unknown)
			{
				continue;
			}

			// if it is an unknown cell...
			std::set<Cell*> NumberedNeighbors;

			For_All_Valid_Neighbors(TestedCell,
				[&NumberedNeighbors](Cell* NeighborCell) -> auto
			{
				//std::cout << NeighborCell->GetPosition() << endl;
				if (static_cast<int>(NeighborCell->GetState()) > 0)
				{
					NumberedNeighbors.insert(NeighborCell);
				}
			});

			if (NumberedNeighbors.size() > 1)
			{
				MarkBlack(TestedCell);
			}
		}
	}
}


void Grid::MarkBlack(Cell* InCell)
{
	if (InCell->GetState() != State::Unknown)
	{
		throw std::logic_error("Trying to mark a cell black that is not set to unknown");
	}

	InCell->SetState(State::Black);
}


// A cell is valid if it's a valid index on the board so x is between [0, width) y is between [0, height)
bool Grid::IsValid(Coordinate2D Cord) const
{
	if (Cord.GetX() >= 0 && Cord.GetX() < m_Width && Cord.GetY() >= 0 && Cord.GetY() < m_Height)
	{
		return true;
	}
	return false;
}


bool Grid::IsValid(Cell* InCell) const
{
	if (!InCell)
		return false;

	if (InCell->GetPosition().GetX() >= 0 && InCell->GetPosition().GetX() < m_Width &&
		InCell->GetPosition().GetY() >= 0 && InCell->GetPosition().GetY() < m_Height)
	{
		return true;
	}
	return false;
}