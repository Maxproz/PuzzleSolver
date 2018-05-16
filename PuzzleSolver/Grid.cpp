
#include "Grid.h"

#include <iostream>
#include <algorithm> // for std::none_of
#include <queue>	// for the queue used in the unreachable() function

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


	// TODO: SOON: On hold until we have a white region not part of a numbered region
	// All white cells must eventually be part of exactly one island.If there is a white region that does not contain a number, and there is only one possible way for it to connect to a numbered white region, the sole connecting pathway must be white.
	//	A function that checks if there is connected or not connected white cells that are not numbered yet.
	//	A function that checks if there is only one possible way for the white cells to connect to a numbered white region
	//	A function that marks the sole connecting pathway as part of that numbered region

	
	// TODO: On hold, function to test if completing an island by marking a cell white will make it impossible for another 
	// - island to complete itself.
	


	//  (DONE) Implement the code that will check for cells (mainly numbered) that have only 1 possible path to expand, and mark 
	// - that cell white, which should fuse regions and update the grid etc..=
	SolveExpandPartialNumberedRegionsWithOnePath();

	// (DONE) Hmm before SolveStepFour is called, we need to implement the check for creating an "impossibly big white region"
	SolveStepFourUnreachableCells();


	// (DONE): Scan for 2x2 pools with 3 black 1 unknown and mark the unknown white. (Update Complete Regions etc...)
	SolveCheckFor2x2Pools();

	
	// NEXT: 
	//		If an island of size N already has N-1 white cells identified, and there are only two remaining cells to choose from, and those two cells touch at their corners, then the cell between those two that is on the far side of the island must be black.	
	//			function to test (If an island of size N already has N-1 white cells identified) ~ bool function(Island)
	//			function that takes a coordinate pair and a functor and runs the functor on all valid neighbors
	//				function for determinting what a valid neighbor is to a coordinate pair.
	//			function that can iterate over the collection of coordinate pairs of an island that calls 
	//			function that determines if an island only has two possible cells left to choose from and only needs one more cell.
	//			function that determines if two cells touch at their corners. (diagional to eachother)
	//			function that will mark the cell that is between two diagional cells as black
	//		A function that tests if only two islands can connect to a white cell.
	//		A function that tests if an island will have no unidentified cells left after connecting to a white cell.
	//		A function that sets a white cell to an island cell (when a numbered cell connects to a white cell)
	//		A function that tests if two islands would connect to the same white cell at a 90 degree angle
	//      A function that sets an unknown cell that is diagionally between two islands to black.
	SolveStepFiveNSizeTwoChoices();
}

void Grid::SolveStepFiveNSizeTwoChoices()
{
	// TODO: Implement:


}

void Grid::SolveCheckFor2x2Pools()
{
	// Start at the top left and move down and to the right looking for black cells.
	// if the cell is black test the other 3 in the square to see if 2 of them are black and one is unknown.
	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			auto CurrentCell = operator()(Coordinate2D(x, y));

			set<Cell*> Blacks;
			set<Cell*> Unknown;
			set<Cell*> CellsInSquare;

			if (CurrentCell->GetState() == State::Black)
			{
				CellsInSquare.insert(CurrentCell);

				auto RightCell = operator()(Coordinate2D(x + 1, y));
				if (!RightCell) continue;
				CellsInSquare.insert(RightCell);

				auto BottomRightCell = operator()(Coordinate2D(x + 1, y + 1));
				if (!BottomRightCell) continue;
				CellsInSquare.insert(BottomRightCell);

				auto BottomCell = operator()(Coordinate2D(x, y + 1));
				if (!BottomCell) continue;
				CellsInSquare.insert(BottomCell);


				std::cout << "CellInSquare size " << CellsInSquare.size() << endl;

				for (auto i = CellsInSquare.begin(); i != CellsInSquare.end(); ++i)
				{
					if ((*i)->GetState() == State::Black)
					{
						Blacks.insert(*i);
					}
					

					if ((*i)->GetState() == State::Unknown)
					{
						Unknown.insert(*i);
					}
				}
			}
			else
			{
				continue;
			}

			//std::cout << Blacks.size() << endl;
			//cout << Unknown.size() << endl;
			if (Blacks.size() == 3 && Unknown.size() == 1)
			{
				Mark((*Unknown.begin()), State::White);
			}
		}
	}
	SolveUpdateCompleteIslands();
}



bool Grid::Impossibly_big_white_region(const int N) const
{
	auto Regions = std::set<Region*>{};
	for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i)
	{
		Regions.insert((*i).get());
	}

		return none_of(Regions.begin(), Regions.end(),
			[N](auto PtrToRegion) 
		{
			// Add one because a bridge would be needed.
			return PtrToRegion->IsNumbered() && PtrToRegion->RegionSize() + N + 1 <= PtrToRegion->GetNumber();
		});

	return false;
}

void Grid::SolveExpandPartialNumberedRegionsWithOnePath()
{
	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			auto CurrentCell = operator()(Coordinate2D(x, y));

			if (CurrentCell->GetState() != State::Unknown)
			{
				auto CellRegion = CurrentCell->GetRegion();
				if (CellRegion->IsNumbered())
				{
					if (CellRegion->GetCellsInRegion().size() < CellRegion->GetNumber())
					{
						if (CellRegion->GetUnknownsAroundRegion().size() == 1)
						{
							Mark(*CellRegion->GetUnknownsAroundRegion().begin(), State::White);
						}
					}
				}
			}
		}
	}
}




void Grid::SolveStepFourUnreachableCells()
{
	set<Cell*> Mark_as_black;
	set<Cell*> Mark_as_white;

	for (int x = 0; x < m_Width; ++x) 
	{
		for (int y = 0; y < m_Height; ++y) 
		{
			auto CurrentCell = operator()(Coordinate2D(x, y));
			if (Unreachable(CurrentCell))
			{
				Mark_as_black.insert(CurrentCell);
			}
		}
	}
	
	for (auto i = Mark_as_black.begin(); i != Mark_as_black.end(); ++i)
	{
		Mark((*i), State::Black);
	}

	
	// Should this be called here, just to make sure it happens?
	//SolveUpdateCompleteIslands();

		//if (process(verbose, mark_as_black, mark_as_white, "Unreachable cells blackened.")) {
		//	return m_sitrep;
		//}
}



bool Grid::Unreachable(Cell* InCell, set<Cell*> discovered) 
{
	// We're interested in unknown cells.
	
	if (InCell->GetState() != State::Unknown)
	{
		return false;
	}
	
	// See http://en.wikipedia.org/wiki/Breadth-first_search
	
	std::queue<pair<Cell*, int>> q;
	
	q.push(make_pair(InCell, 1));
	discovered.insert(InCell);
	
	while (!q.empty())
	{
		const int x_curr = q.front().first->GetPosition().GetX();
		const int y_curr = q.front().first->GetPosition().GetY();
		const int n_curr = q.front().second;

		q.pop();

		set<Region*> White_regions;
		set<Region*> Numbered_regions;

		For_All_Valid_Neighbors(operator()(Coordinate2D(x_curr, y_curr)), [&](Cell* NeighborCell)
		{
			Region* r = NeighborCell->GetRegion();

			if (r && r->IsWhite())
			{
				White_regions.insert(r);
			}
			else if (r && r->IsNumbered())
			{
				Numbered_regions.insert(r);
			}
		});

		int size = 0;

		for (auto i = White_regions.begin(); i != White_regions.end(); ++i)
		{
			size += (*i)->RegionSize();
		}

		for (auto i = Numbered_regions.begin(); i != Numbered_regions.end(); ++i)
		{
			size += (*i)->RegionSize();
		}

		if (Numbered_regions.size() > 1)
		{
			continue;
		}
		//
		if (Numbered_regions.size() == 1)
		{
			const int num = (*Numbered_regions.begin())->GetNumber();

			if (n_curr + size <= num)
			{
				return false;
			}
			else
			{
				continue;
			}
		}
		
		if (!White_regions.empty())
		{
			cout << "White regions is not empty but cant test for impossibly big regions yet" << endl;
			// TODO: Implement a test for this

			if (Impossibly_big_white_region(n_curr + size))
			{
				cout << "Impossibly big white region, continuing";
				continue;
			}
			else 
			{
				return false;
			}
		}

		For_All_Valid_Neighbors(operator()(Coordinate2D(x_curr, y_curr)),
			[&](Cell* NeighborCell)
		{
			if (NeighborCell->GetState() == State::Unknown && discovered.insert(NeighborCell).second)
			{
				//std::cout << "Test";
				q.push(make_pair(NeighborCell, n_curr + 1));
			}
		});

	}
	
	return true;

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
			SetStateOfAllUnknownNeighborsToCellsInARegion(Reg, State::Black);
		}
	}
}


void Grid::SetStateOfAllUnknownNeighborsToCellsInARegion(Region* InRegion, const State& InState)
{
	if (InState == State::Black || InState == State::White)
	{
		if (InState == State::Black)
		{
			for (auto& CellInRegion : InRegion->GetCellsInRegion())
			{
				For_All_Valid_Unknown_Neighbors(CellInRegion,
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



#include <string>


void Grid::Mark(Cell* InCell, const State NewState)
{
	if (InCell->GetState() != State::Unknown)
	{
		// TODO: This would be where I would return with contradiction found or something like that to our "solve" while loop
		//std::cout << InCell->GetPosition() << endl;
		//return;
		throw std::logic_error("Trying to mark a cell black that is not set to unknown " + std::to_string(InCell->GetPosition().GetX()) + 
			+ ", " + std::to_string(InCell->GetPosition().GetY()));
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