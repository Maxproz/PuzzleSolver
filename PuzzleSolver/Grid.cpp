
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

	} cout << endl;

}

void Grid::PrintAllCellsInAllRegions() const
{
	int RegionCounter = 1;
	for (const auto& region : m_Regions)
	{
		if (region->IsBlack())
		{
			std::cout << "Black";
		}
		if (region->IsWhite())
		{
			std::cout << "White";
		}
		if (region->IsNumbered())
		{
			std::cout << "Numbered ";
			auto RegionNumber = region->GetNumber();
			cout << RegionNumber;
		}
		std::cout << " Region ";
		std::cout << RegionCounter << ": " << endl;
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


	// Step 3
	// All black cells must eventually be connected. If there is a black region with only one possible way to connect to the rest of the board, the sole connecting pathway must be black.
	SolveBlackHasToConnect();


	// TODO: On hold until we have a white region not part of a numbered region
	// All white cells must eventually be part of exactly one island.If there is a white region that does not contain a number, and there is only one possible way for it to connect to a numbered white region, the sole connecting pathway must be white.
	//	A function that checks if there is connected or not connected white cells that are not numbered yet.
	//	A function that checks if there is only one possible way for the white cells to connect to a numbered white region
	//	A function that marks the sole connecting pathway as part of that numbered region

	

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
	for (auto& Reg : InNumberedRegions)
	{
		if (Reg->GetNumber() == Reg->RegionSize())
		{
			// The numbered region is complete
			// For each cell in the region mark all valid adjacent cells of the cell in that region black
			SetStateOfAllNeighborsToCellsInARegion(Reg, State::Black);
		}
	}
}


void Grid::SetStateOfAllNeighborsToCellsInARegion(Region* InRegion, const State& InState)
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
					Mark(NeighborCell, State::Black);
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

void Grid::SolveBlackHasToConnect()
{
	// TODO: There is some nice logic in here that I can use later for needing to test copies of a board
	// - and nested neighbor testing

	// For every cell on the board....
	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			auto TestedCell = operator()(Coordinate2D(x, y));

			// Make sure we are testing an unknown square
			if (TestedCell->GetState() != State::Unknown)
			{
				continue;
			}

			// Create a copy of our Gameboard to test changing this unknown cell to a white square
			Grid TempGameBoard(*this);
			auto TempTestedCell = TempGameBoard.operator()(Coordinate2D(x, y));

			TempGameBoard.Mark(TempTestedCell, State::White);


			// Did marking this cell white make it so its neighbors couldn't connect?
			TempGameBoard.For_All_Valid_Neighbors(TempTestedCell,
				[this, &TempGameBoard, &TestedCell](Cell* NeighborCell) -> auto
			{
				if (NeighborCell->GetState() == State::Black)
				{
					auto HasPath = TempGameBoard.BlackCellHasAtLeastOnePath(NeighborCell);
					if (HasPath == false)
					{
						this->Mark(TestedCell, State::Black);
					}
				}
			});
		}
	}
}

bool Grid::BlackCellHasAtLeastOnePath(Cell* InBlackCell)
{
	bool HasAtLeastOnePossiblePathway = false;
	set<Cell*> Pathways;

	For_All_Valid_Neighbors(InBlackCell, [&Pathways](Cell* NeighborCell) -> auto						
	{ 
		if (NeighborCell->GetState() == State::Black || NeighborCell->GetState() == State::Unknown)
		{
			Pathways.insert(NeighborCell);
		}
	});

	if (Pathways.size() == 0)
	{
		return false;
	}

	return true;
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
				Mark(TestedCell, State::Black);
			}
		}
	}
}




void Grid::Mark(Cell* InCell, const State NewState)
{
	if (InCell->GetState() != State::Unknown)
	{
		// TODO: This would be where I would return with contradiction found or something like that to our "solve" while loop
		//std::cout << InCell->GetPosition() << endl;
		//return;
		throw std::logic_error("Trying to mark a cell black that is not set to unknown");
	}

	if (NewState != State::Black && NewState != State::White)
	{
		throw std::logic_error("Error InCell is not black or white");
	}

	if (NewState == State::Black)
	{
		InCell->SetState(State::Black);
	}
	else if (NewState == State::White)
	{
		InCell->SetState(State::White);
	}
	else
	{

	}

	for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i)
	{
		(*i)->EraseUnknown(InCell);
	}

	//  Marking a cell as white or black could create an independent region,
	//	could be added to an existing region, or could connect 2, 3, or 4 separate regions.
	//	The easiest thing to do is to create a region for this cell,
	//	and then fuse it to any adjacent compatible regions.
	
	AddRegion(InCell);
	
	//	Don't attempt to cache these regions.
	//	Each fusion could change this cell's region or its neighbors' regions.

	For_All_Valid_Neighbors(InCell, [this, &InCell](Cell* NeighborCell)
	{
		FuseRegions(InCell->GetRegion(), NeighborCell->GetRegion());
	});

}

void Grid::FuseRegions(Region* LHSRegion, Region* RHSRegion)
{
	// If we don't have two different regions, we're done. Or one of our regions is nullptr
	// - There would be no point in trying to fuse the same region
	if (!LHSRegion || !RHSRegion || LHSRegion == RHSRegion)
	{
			return;
	}

	// If we're asked to fuse two numbered regions, we've encountered a contradiction.
	// Remember this, so that solve() can report the contradiction.
	//
	//	if (r1->numbered() && r2->numbered()) {
	//		m_sitrep = CONTRADICTION_FOUND;
	//		return;
	//	}
	if (LHSRegion->IsNumbered() && RHSRegion->IsNumbered())
	{
		throw std::logic_error("Tried to fuse two numbered regions, Do I need to allow this for copied grid testin purposes?");
	}

	// A Black Region can't fuse with a non-black region
	if (LHSRegion->IsBlack() != RHSRegion->IsBlack())
	{
		return;
	}

	// We'll use r1 as the "primary" region, to which r2's cells are added.
	// It would be efficient to process as few cells as possible.
	// Therefore, we'd like to use the bigger region as the primary region.
	if (RHSRegion->RegionSize() > LHSRegion->RegionSize())
	{
		std::swap(LHSRegion, RHSRegion);
	}

	//	 However, if the secondary region is numbered, then the primary region
	//	 must be white, so we need to swap them, even if the numbered region is smaller.
	if (RHSRegion->IsNumbered()) 
	{
		std::swap(LHSRegion, RHSRegion);
	}

	// Fuse the secondary region into the primary region.
	// A "Fuse" is "connecting" the cells in one region to another region.
	// - And if we do that we need/can also add that regions unknown cells since they should be the same.
	LHSRegion->Insert(RHSRegion->Begin(), RHSRegion->End());
	LHSRegion->UnknownsInsert(RHSRegion->UnknownsBegin(), RHSRegion->UnknownsEnd());


	// Update the secondary region's cells to point to the primary region.
	for (std::set<Cell*>::iterator i = RHSRegion->Begin(); i != RHSRegion->End(); ++i)
	{
		(*i)->SetRegion(LHSRegion);
		//region(i->first, i->second) = r1; 
	}

	// Erase the secondary region from the set of all regions.
	// When this function returns, the secondary region will be destroyed.
		
	//m_regions.erase(r2);
	for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i)
	{
		if ((*i).get() == RHSRegion)
		{
			m_Regions.erase(i);
			return;
		}
	}
}


//// Note that r1 and r2 are passed by modifiable value. It's convenient to be able to swap them.
//void Grid::fuse_regions(shared_ptr<Region> r1, shared_ptr<Region> r2)
// {
//	// Fuse the secondary region into the primary region.
//


//

// // as of right now, the shared_ptr count for r2 is 1 (scope of r2 in this function)
//} // as of right now, the shared_ptr count for r2 is 0 (region pointed to by r2 deleted on the heap)

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