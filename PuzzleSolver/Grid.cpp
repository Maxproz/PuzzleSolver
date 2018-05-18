
#include "Grid.h"

#include <iostream>
#include <algorithm> // for std::none_of
#include <queue>	// for the queue used in the unreachable() function
#include <string>  // std::to_string
#include <array> // for the array we use in SolvePreventPoolsTwoBlackTwoUnknown

#include "Cell.h"
#include "Region.h"

using namespace std;


// Unused atm | m_total_black(width * height)


Grid::Grid(int Width, int Height, std::vector<std::pair<Coordinate2D, int>> NumberedCellLocations) :
	m_Width{ Width }, m_Height{ Height }, m_Cells(), m_Regions(), m_total_black(Width * Height)
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

					//				// m_total_black is width * height - (sum of all numbered cells).
					//				// Calculating this allows us to determine whether black regions
					//				// are partial or complete with a simple size test.
					//				// Humans are capable of this but it's not convenient for them.
					//				// We will show the meatbags that silicon is the superior element!
					//
					m_total_black -= Island.second;
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
	// TODO: I need to call this function using a condition from a "while (Not Solved)" do these steps over in order etc...
	// TODO: I wish it was more organized in this function

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


	// NEXT: (mostly done): 
	//		If an island of size N already has N-1 white cells identified, and there are only two remaining cells to choose from, and those two cells touch at their corners, then the cell between those two that is on the far side of the island must be black.	
	//			(DONE) function to test (If an island of size N already has N-1 white cells identified) ~ bool function(Island)
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
	SolveExpandPartialNumberedRegionsWithOnePath();
	SolveUpdateCompleteIslands();
	SolveCheckFor2x2Pools();
	SolveUpdateCompleteIslands();
	SolveCheckFor2x2Pools();
	SolveBlackHasToConnect(); // This pushes the black cell at the bottom forward // NOTE: Not even sure why
	//SolvePartialWhiteRegionsWithOnlyOnePath(); // Connect the white region with the 4 in the bottom
	
	// TODO: FIX: Expand and solve the 2 island in the top left
	//SolveExpandPartialNumberedRegionsWithOnePath();
	//SolveUpdateCompleteIslands();

	//SolveBlackHasToConnect();// Push the black cell at the bottom forward again.
	SolvePartialBlackRegionsWithOnlyOnePath();
	SolveStepFiveNSizeTwoChoices();

	SolveExpandPartialNumberedRegionsWithOnePath();
	SolveUpdateCompleteIslands();

	SolveCheckFor2x2Pools();

	SolveUpdateCompleteIslands();

	SolveCheckFor2x2Pools();

	SolvePartialWhiteRegionsWithOnlyOnePath(); // Connect the white region with the 4 in the bottom

	SolvePartialBlackRegionsWithOnlyOnePath();

	// Solve Top Left 2
	SolveExpandPartialNumberedRegionsWithOnePath();
	SolveUpdateCompleteIslands();


	// Push top left black path
	SolvePartialBlackRegionsWithOnlyOnePath();



	// NEXT: UPDATE: (STUCK) Start working on solving the bottom left stuff
	// (Making the 3 island complete by going right, will make the 4 unable to be completed)
	// (Which will also allow us to solve the 2 in the bottom near the 4 once we see the result from the previous solve)


	// First thing, clean up the debug code so you can actually make sense of what is going on.
	SolvePreventPoolsTwoBlackTwoUnknown();


	SolveStepFourUnreachableCells();

	
	// The last two functions I should realisticly need are "confinment analysis" and a "guessing" function
	// Attempted to solve the confinment analysis one myself, and failed so I will just need to copypasta, convert, and study existing code
	
	const string s = DetectContradictions(m_Cache);
	if (!s.empty())
	{
		cout << s << endl;
	}

	 SolveConfinementAnalysis();
	// uses Confined(); member func
	 SolveExpandPartialNumberedRegionsWithOnePath();
	 SolveUpdateCompleteIslands();
	
	 
	 SolveExpandPartialNumberedRegionsWithOnePath();
	 SolveUpdateCompleteIslands();
	 
	 SolvePartialBlackRegionsWithOnlyOnePath();


	 SolveExpandPartialNumberedRegionsWithOnePath();
	 SolveUpdateCompleteIslands();


	 SolveCheckFor2x2Pools();
	 
	 
	 // I was wrong. I also need a function that well find "Isolated Unknown Cells"
	 SolveIsolatedUnknownCells();
	 
	 
	 // SolveGuessingRemaining();



}

void Grid::SolveIsolatedUnknownCells()
{
	// TODO: Need to filter this

	cout << "SolveIsolatedUnknownCells" << endl;

	//	// Look for isolated unknown regions.
	//
	//	{
	//		const bool any_black_regions = any_of(m_regions.begin(), m_regions.end(),
	//			[](const shared_ptr<Region>& r) { return r->black(); });
	//
	//		set<pair<int, int>> analyzed;
	//
	//		for (int x = 0; x < m_width; ++x) {
	//			for (int y = 0; y < m_height; ++y) {
	//				if (cell(x, y) == UNKNOWN && analyzed.find(make_pair(x, y)) == analyzed.end()) {
	//					bool encountered_black = false;
	//
	//					set<pair<int, int>> open;
	//					set<pair<int, int>> closed;
	//
	//					open.insert(make_pair(x, y));
	//
	//					while (!open.empty()) {
	//						const pair<int, int> p = *open.begin();
	//						open.erase(open.begin());
	//
	//						switch (cell(p.first, p.second)) {
	//							case UNKNOWN:
	//								if (closed.insert(p).second) {
	//									insert_valid_neighbors(open, p.first, p.second);
	//								}
	//
	//								break;
	//
	//							case BLACK:
	//								encountered_black = true;
	//								break;
	//
	//							default:
	//								break;
	//						}
	//					}
	//
	//					if (!encountered_black && (
	//						any_black_regions || static_cast<int>(closed.size()) < m_total_black)) {
	//
	//						mark_as_white.insert(closed.begin(), closed.end());
	//					}
	//
	//					analyzed.insert(closed.begin(), closed.end());
	//				}
	//			}
	//		}
	//	}
	//
	//	if (process(verbose, mark_as_black, mark_as_white, "Isolated unknown regions found.")) {
	//		return m_sitrep;
	//	}


}

void Grid::SolveConfinementAnalysis()
{
	cout << "SolveConfinementAnalysis" << endl;



		// A region would be "confined" if it could not be completed.
		// Black regions need to consume m_total_black cells.
		// White regions need to escape to a number.
		// Numbered regions need to consume N cells.
	
		// Confinement analysis consists of imagining what would happen if a particular unknown cell
		// were black or white. If that would cause any region to be confined, the unknown cell
		// must be the opposite color.
	
		// Black cells can't confine black regions, obviously.
		// Black cells can confine white regions, by isolating them.
		// Black cells can confine numbered regions, by confining them to an insufficiently large space.
	
		// White cells can confine black regions, by confining them to an insufficiently large space.
		//   (Humans look for isolation here, i.e. permanently separated black regions.
		//   That's harder for us to detect, but counting cells is similarly powerful.)
		// White cells can't confine white regions.
		//   (This is true for freestanding white cells, white cells added to other white regions,
		//   and white cells added to numbered regions.)
		// White cells can confine numbered regions, when added to other numbered regions.
		//   This is the most complicated case to analyze. For example:
		//   ####3
		//   #6 xXx
		//   #.  x
		//   ######
		//   Imagining cell 'X' to be white additionally prevents region 6 from consuming
		//   three 'x' cells. (This is true regardless of what other cells region 3 would
		//   eventually occupy.)
	

	std::set<Cell*> mark_as_white;
	std::set<Cell*> mark_as_black;

	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			auto CurrentCell = operator()(Coordinate2D(x, y));

			if (CurrentCell->GetState() == State::Unknown)
			{
				set<Cell*> ForbiddenCells;
				ForbiddenCells.insert(CurrentCell);

				for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i)
				{
					const Region& r = **i;

					if (Confined((*i).get(), m_Cache, ForbiddenCells))
					{
						if (r.IsBlack())
						{
							//mark_as_black.insert(make_pair(x, y));
							mark_as_black.insert(operator()(Coordinate2D(x, y)));

						}
						else
						{
							//mark_as_white.insert(make_pair(x, y));
							mark_as_white.insert(operator()(Coordinate2D(x, y)));
						}
					}
				}
			}
		}
	}
	
	
	for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i)
	{
		const Region& r = **i;

		if (r.IsNumbered() && r.RegionSize() < r.GetNumber()) 
		{
			for (auto u = r.UnknownsBegin(); u != r.UnknownsEnd(); ++u) 
			{
				set<Cell*> ForbiddenCells;
				ForbiddenCells.insert(*u);

				//insert_valid_unknown_neighbors(ForbiddenCells, u->first, u->second);
				For_All_Valid_Unknown_Neighbors(operator()(Coordinate2D((*u)->GetPosition().GetX(),
					(*u)->GetPosition().GetY())), [&ForbiddenCells](Cell* NeighborCell) -> auto
				{
					ForbiddenCells.insert(NeighborCell);
				});

				for (auto k = m_Regions.begin(); k != m_Regions.end(); ++k)
				{
					if (k != i && (*k)->IsNumbered() && Confined((*k).get(), m_Cache, ForbiddenCells)) 
					{
						mark_as_black.insert(*u);
					}
				}
			}
		}
	}
	
	cout << "MarkAsBlackSize() == " << mark_as_black.size() << endl;
	
	for (auto& CellToMarkBlack : mark_as_black)
	{
		Mark(CellToMarkBlack, State::Black);
	}

	cout << "MarkAsWhiteSize() == " << mark_as_white.size() << endl;

	for (auto& CellToMarkWhite : mark_as_white)
	{
		Mark(CellToMarkWhite, State::White);
	}

	//if (process(verbose, mark_as_black, mark_as_white, "Confinement analysis succeeded.")) {
	//	return m_sitrep;
	//}

}

bool Grid::Confined(Region* r, std::map<Region*, std::set<Cell*>>& cache, const std::set<Cell*>& forbidden)
{

	// When we look for contradictions, we run confinement analysis (A) without forbidden cells.
	// This gives us an opportunity to accelerate later confinement analysis (B)
	// when we have forbidden cells.
	// During A, we'll record the unknown cells that we consumed.
	// During B, if none of the forbidden cells are ones that we consumed,
	// then the forbidden cells can't confine us.

	if (!forbidden.empty())
	{
		const auto i = cache.find(r);

		if (i == cache.end())
		{
			return false; // We didn't consume any unknown cells.
		}

		const auto& consumed = i->second;

		if (none_of(forbidden.begin(), forbidden.end(), [&](Cell* p)
		{
			return consumed.find(p) != consumed.end();
		}))
		{
			return false;
		}
	}

	// The open set contains cells that we're considering adding to the region.
	set<Cell*> open(r->UnknownsBegin(), r->UnknownsEnd());

	// The closed set contains cells that we've hypothetically added to the region.
	set<Cell*> closed(r->Begin(), r->End());

	// While we have cells to consider and we need to consume more cells...
	while (!open.empty()
		&& (r->IsBlack() && static_cast<int>(closed.size()) < m_total_black
			|| r->IsWhite()
			|| r->IsNumbered() && static_cast<int>(closed.size()) < r->GetNumber()))
	{

		// Consider cell p.
		Cell* p = *open.begin();
		open.erase(open.begin());

		// If it's forbidden or we've already consumed it, discard it.
		if (forbidden.find(p) != forbidden.end() || closed.find(p) != closed.end())
		{
			continue;
		}

		// We need to compare our region r with p's region (if any).
		//const auto& area = region(p.first, p.second);
		const auto& area = p->GetRegion();

		if (r->IsBlack())
		{
			if (!area)
			{
				// Keep going. A black region can consume an unknown cell.
			}
			else if (area->IsBlack())
			{
				// Keep going. A black region can consume another black region.
			}
			else // area->white() || area->numbered()
			{
				continue; // We can't consume this. Discard it.
			}
		}
		else if (r->IsWhite())
		{
			if (!area) {
				// Keep going. A white region can consume an unknown cell.
			}
			else if (area->IsBlack()) {
				continue; // We can't consume this. Discard it.
			}
			else if (area->IsWhite()) {
				// Keep going. A white region can consume another white region.
			}
			else // area->numbered()
			{
				return false; // Yay! Our region r escaped to a numbered region.
			}
		}
		else // r->numbered()
		{
			if (!area)
			{
				// A numbered region can't consume an unknown cell
				// that's adjacent to another numbered region.
				bool rejected = false;

				//for_valid_neighbors(p.first, p.second, [&](const int x, const int y) 
				//{
				//	const auto& other = region(x, y);

				//	if (other && other->numbered() && other != r) {
				//		rejected = true;
				//	}
				//});

				Cell* InCell = operator()(Coordinate2D(p->GetPosition().GetX(), p->GetPosition().GetY()));
				For_All_Valid_Neighbors(InCell, [&](Cell* NeighborCell)
				{
					const auto& other = NeighborCell->GetRegion();

					if (other && other->IsNumbered() && other != r)
					{
						rejected = true;
					}
				});

				if (rejected) {
					continue;
				}

				// Keep going. This unknown cell is okay to consume.
			}
			else if (area->IsBlack())
			{
				continue; // We can't consume this. Discard it.
			}
			else if (area->IsWhite())
			{
				// Keep going. A numbered region can consume a white region.
			}
			else // area->numbered()
			{
				throw logic_error("LOGIC ERROR: Grid::confined() - "
					"I was confused and thought two numbered regions would be adjacent.");
			}
		}

		if (!area) // Consume an unknown cell.
		{
			closed.insert(p);

			// insert_valid_neighbors(open, p.first, p.second);

			For_All_Valid_Neighbors(operator()(Coordinate2D((p)->GetPosition().GetX(),
				(p)->GetPosition().GetY())), [&open](Cell* NeighborCell) -> auto
			{
				open.insert(NeighborCell);
			});


			if (forbidden.empty())
			{
				cache[r].insert(p);


			}
		}
		else // Consume a whole region.
		{
			closed.insert(area->Begin(), area->End());
			open.insert(area->UnknownsBegin(), area->UnknownsEnd());
		}
	}

	// We're confined if we still need to consume more cells.
	return r->IsBlack() && static_cast<int>(closed.size()) < m_total_black
		|| r->IsWhite()
		|| r->IsNumbered() && static_cast<int>(closed.size()) < r->GetNumber();
}

	//return false;


std::string Grid::DetectContradictions(std::map<Region*, set<Cell*>>& Cache)
{
	// Think this first part logic is covered somewhere else 

	//string Grid:: const {
	//	for (int x = 0; x < m_width - 1; ++x) {
	//		for (int y = 0; y < m_height - 1; ++y) {
	//			if (cell(x, y) == BLACK
	//				&& cell(x + 1, y) == BLACK
	//				&& cell(x, y + 1) == BLACK
	//				&& cell(x + 1, y + 1) == BLACK) {
	//
	//				return "Contradiction found! Pool detected.";
	//			}
	//		}
	//	}
	//
	int black_cells = 0;
	int white_cells = 0;

	for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i)
	{
		Region& r = **i;

		// We don't need to look for gigantic black regions because
		// counting black cells is strictly more powerful.

		if (r.IsWhite() && Impossibly_big_white_region(r.RegionSize())
			|| r.IsNumbered() && r.RegionSize() > r.GetNumber())
		{
			return "Contradiction found! Gigantic region detected.";
		}

		(r.IsBlack() ? black_cells : white_cells) += r.RegionSize();


		if (Confined((*i).get(), Cache))
		{
			return "Contradiction found! Confined region detected.";
		}
	}

	if (black_cells > m_total_black)
	{
		return "Contradiction found! Too many black cells detected.";
	}

	if (white_cells > m_Width * m_Height - m_total_black)
	{
		return "Contradiction found! Too many white/numbered cells detected.";
	}

	return "";
}



void Grid::SolveGuessingRemaining()
{
	cout << "SolveGuessingRemaining" << endl;

	//	if (guessing) {
	//		vector<pair<int, int>> v;
	//
	//		for (int x = 0; x < m_width; ++x) {
	//			for (int y = 0; y < m_height; ++y) {
	//				if (cell(x, y) == UNKNOWN) {
	//					v.push_back(make_pair(x, y));
	//				}
	//			}
	//		}
	//
	//
	//		// Guess cells in a deterministic but pseudorandomized order.
	//		// This attempts to avoid repeatedly guessing cells that won't get us anywhere.
	//
	//		auto dist = [this](const ptrdiff_t n) {
	//			// random_shuffle() provides n > 0. It wants [0, n).
	//			// uniform_int_distribution's ctor takes a and b with a <= b. It produces [a, b].
	//			return uniform_int_distribution<ptrdiff_t>(0, n - 1)(m_prng);
	//		};
	//
	//		random_shuffle(v.begin(), v.end(), dist);
	//
	//
	//		for (auto u = v.begin(); u != v.end(); ++u) {
	//			const int x = u->first;
	//			const int y = u->second;
	//
	//			for (int i = 0; i < 2; ++i) {
	//				const State color = i == 0 ? BLACK : WHITE;
	//				auto& mark_as_diff = i == 0 ? mark_as_white : mark_as_black;
	//				auto& mark_as_same = i == 0 ? mark_as_black : mark_as_white;
	//
	//				Grid other(*this);
	//
	//				other.mark(color, x, y);
	//
	//				SitRep sr = KEEP_GOING;
	//
	//				while (sr == KEEP_GOING) {
	//					sr = other.solve(false, false);
	//				}
	//
	//				if (sr == CONTRADICTION_FOUND) {
	//					mark_as_diff.insert(make_pair(x, y));
	//					process(verbose, mark_as_black, mark_as_white,
	//						"Hypothetical contradiction found.");
	//					return m_sitrep;
	//				}
	//
	//				if (sr == SOLUTION_FOUND) {
	//					mark_as_same.insert(make_pair(x, y));
	//					process(verbose, mark_as_black, mark_as_white,
	//						"Hypothetical solution found.");
	//					return m_sitrep;
	//				}
	//
	//				// sr == CANNOT_PROCEED
	//			}
	//		}
	//	}
	
}


void Grid::SolvePreventPoolsTwoBlackTwoUnknown()
{
	// Look for squares of one unknown and three black cells, or two unknown and two black cells.
	std::set<Cell*> mark_as_white;


	for (int x = 0; x < m_Width - 1; ++x)
	{
		for (int y = 0; y < m_Height - 1; ++y)
		{
			struct XYState
			{
				int x;
				int y;
				State state;
			};

			//auto CurrentCell = operator()(Coordinate2D(x, y));

			array<XYState, 4> a =
			{
				{
				{ x, y, operator()(Coordinate2D(x, y))->GetState() },
				{ x + 1, y, operator()(Coordinate2D(x + 1, y))->GetState() },
				{ x, y + 1, operator()(Coordinate2D(x, y + 1))->GetState() },
				{ x + 1, y + 1, operator()(Coordinate2D(x + 1, y + 1))->GetState() }
				}

			};

			static_assert(State::Unknown < State::Black, "This code assumes that UNKNOWN < BLACK.");


			sort(a.begin(), a.end(), [](const XYState& l, const XYState& r)
			{
				return l.state < r.state;
			});

			// This logic is already covered in another function
			//if (a[0].state == State::Unknown
			//	&& a[1].state == State::Black
			//	&& a[2].state == State::Black
			//	&& a[3].state == State::Black)
			//{

			//	//mark_as_white.insert(make_pair(a[0].x, a[0].y));
			//	//mark_as_white.insert(operator()(Coordinate2D(a[0].x, a[0].y)));

			//}
			// TODO: Study how this is passing imagine_black over.
			/*else*/ if (a[0].state == State::Unknown
				&& a[1].state == State::Unknown
				&& a[2].state == State::Black
				&& a[3].state == State::Black)
			{

				for (int i = 0; i < 2; ++i)
				{
					set<Cell*> imagine_black;

					imagine_black.insert(operator()(Coordinate2D(a[0].x, a[0].y)));

					if (Unreachable(operator()(Coordinate2D(a[1].x, a[1].y)), imagine_black))
					{
						//mark_as_white.insert(make_pair(a[0].x, a[0].y));
						mark_as_white.insert(operator()(Coordinate2D(a[0].x, a[0].y)));
					}

					std::swap(a[0], a[1]);
				}
			}
		}
	}
	
	for (auto& CellToMarkWhite : mark_as_white)
	{
		Mark(CellToMarkWhite, State::White);
	}

	//if (process(verbose, mark_as_black, mark_as_white, "Whitened cells to prevent pools."))
	//{
	//	return m_sitrep;
	//}


}


void Grid::SolveStepFiveNSizeTwoChoices()
{
	cout << "SolveStepFiveNSizeTwoChoices" << endl;
	// TODO: Implement:

	// Iterate over all numbered regions
	// Find all numbered regions of Size N that have N - 1 identified 
	auto Regions = std::set<Region*>{};
	for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i)
	{
		Regions.insert((*i).get());
	}

	auto RegionsToTest = set<Region*>{};
	for (auto i = Regions.begin(); i != Regions.end(); ++i)
	{
		if ((*i)->IsNumbered())
		{
			auto RegionNumber = (*i)->GetNumber();

			if ((*i)->RegionSize() == RegionNumber - 1)
			{
				RegionsToTest.insert(*i);
			}
		}
	}

	// Now that we have the Numbered Regions we want to test, we also need to check if there is only two unknowns in the regions.
	for (auto i = Regions.begin(); i != Regions.end(); ++i)
	{
		if ((*i)->UnknownsSize() != 2)
		{
			RegionsToTest.erase(*i);
		}
	}



	// Now we have narrowed down our options even more.
	// Next we want to iterate over the regions we have left
	// for each region, we want to test if the two unknowns in the region touch at the corners
	// Now that we have the Numbered Regions we want to test, we also need to check if there is only two unknowns in the regions.
	for (auto i = RegionsToTest.begin(); i != RegionsToTest.end(); ++i)
	{
		auto UnknownCellsAroundReg = std::vector<Cell*>{};
		UnknownCellsAroundReg.reserve(100);

		auto UnknownCellsSet = (*i)->GetUnknownsAroundRegion();

		for (auto i = UnknownCellsSet.begin(); i != UnknownCellsSet.end(); ++i)
		{
			UnknownCellsAroundReg.push_back(*i);
		}

		if (UnknownCellsAroundReg.size() <= 1)
		{
			continue;
		}


		auto FirstUnknownCell = UnknownCellsAroundReg[0];
		auto SecondUnknownCell = UnknownCellsAroundReg[1];



		Coordinate2D TopRight = Coordinate2D((FirstUnknownCell)->GetPosition().GetX() + 1, (FirstUnknownCell)->GetPosition().GetY() - 1);
		Coordinate2D BottomRight = Coordinate2D((FirstUnknownCell)->GetPosition().GetX() + 1, (FirstUnknownCell)->GetPosition().GetY() + 1);
		Coordinate2D BottomLeft = Coordinate2D((FirstUnknownCell)->GetPosition().GetX() - 1, (FirstUnknownCell)->GetPosition().GetY() + 1);
		Coordinate2D TopLeft = Coordinate2D((FirstUnknownCell)->GetPosition().GetX() - 1, (FirstUnknownCell)->GetPosition().GetY() - 1);

		if (SecondUnknownCell->GetPosition() == TopRight)
		{
			//std::cout << *FirstUnknownCell << endl;
			//std::cout << *SecondUnknownCell << endl;
			
			auto MaybeCellToSet =
				operator()(Coordinate2D(FirstUnknownCell->GetPosition().GetX(), FirstUnknownCell->GetPosition().GetY() - 1));

			auto MaybeOtherCellToSet =
				operator()(Coordinate2D(FirstUnknownCell->GetPosition().GetX() + 1, FirstUnknownCell->GetPosition().GetY()));

			if (MaybeCellToSet->GetState() == State::White || static_cast<int>(MaybeCellToSet->GetState()) > 0)
			{
				if (MaybeOtherCellToSet->GetState() == State::Unknown)
				{
					std::cout << "Two Unknowns Touch at corners trying to mark black " << *MaybeOtherCellToSet << endl;
					Mark(MaybeOtherCellToSet, State::Black);
				}
				
			}
			else
			{
				if (MaybeCellToSet->GetState() == State::Unknown)
				{
					std::cout << "Two Unknowns Touch at corners trying to mark black " << *MaybeCellToSet << endl;
					Mark(MaybeCellToSet, State::Black);
				}
			}

		}
		else if (SecondUnknownCell->GetPosition() == BottomRight)
		{
			
			auto MaybeCellToSet =
				operator()(Coordinate2D(FirstUnknownCell->GetPosition().GetX() + 1, FirstUnknownCell->GetPosition().GetY()));

			auto MaybeOtherCellToSet =
				operator()(Coordinate2D(FirstUnknownCell->GetPosition().GetX(), FirstUnknownCell->GetPosition().GetY() + 1));

			if (MaybeCellToSet->GetState() == State::White || static_cast<int>(MaybeCellToSet->GetState()) > 0)
			{
				if (MaybeOtherCellToSet->GetState() == State::Unknown)
				{
					std::cout << "Two Unknowns Touch at corners trying to mark black " << *MaybeOtherCellToSet << endl;
					Mark(MaybeOtherCellToSet, State::Black);
				}

			}
			else
			{
				if (MaybeCellToSet->GetState() == State::Unknown)
				{
					std::cout << "Two Unknowns Touch at corners trying to mark black " << *MaybeCellToSet << endl;
					Mark(MaybeCellToSet, State::Black);
				}
			}
		}
		else if (SecondUnknownCell->GetPosition() == BottomLeft)
		{
			auto MaybeCellToSet =
				operator()(Coordinate2D(FirstUnknownCell->GetPosition().GetX() - 1, FirstUnknownCell->GetPosition().GetY()));

			auto MaybeOtherCellToSet =
				operator()(Coordinate2D(FirstUnknownCell->GetPosition().GetX(), FirstUnknownCell->GetPosition().GetY() + 1));

		
			if (MaybeCellToSet->GetState() == State::White || static_cast<int>(MaybeCellToSet->GetState()) > 0)
			{
				if (MaybeOtherCellToSet->GetState() == State::Unknown)
				{
					std::cout << "Two Unknowns Touch at corners trying to mark black " << *MaybeOtherCellToSet << endl;
					Mark(MaybeOtherCellToSet, State::Black);
				}

			}
			else
			{
				if (MaybeCellToSet->GetState() == State::Unknown)
				{
					std::cout << "Two Unknowns Touch at corners trying to mark black " << *MaybeCellToSet << endl;
					Mark(MaybeCellToSet, State::Black);
				}
			}
		}
		else if (SecondUnknownCell->GetPosition() == TopLeft)
		{
			auto MaybeCellToSet =
				operator()(Coordinate2D(FirstUnknownCell->GetPosition().GetX() - 1, FirstUnknownCell->GetPosition().GetY()));

			auto MaybeOtherCellToSet =
				operator()(Coordinate2D(FirstUnknownCell->GetPosition().GetX(), FirstUnknownCell->GetPosition().GetY() - 1));

			if (MaybeCellToSet->GetState() == State::White || static_cast<int>(MaybeCellToSet->GetState()) > 0)
			{
				if (MaybeOtherCellToSet->GetState() == State::Unknown)
				{
					std::cout << "Two Unknowns Touch at corners trying to mark black " << *MaybeOtherCellToSet << endl;
					Mark(MaybeOtherCellToSet, State::Black);
				}

			}
			else
			{
				if (MaybeCellToSet->GetState() == State::Unknown)
				{
					std::cout << "Two Unknowns Touch at corners trying to mark black " << *MaybeCellToSet << endl;
					Mark(MaybeCellToSet, State::Black);
				}
			}
		}
		else
		{
			cout << "End of testing of regions of size 2 with 2 unknown paths" << endl;
		}
	}
}

	
bool Grid::DoesMarkBlackCreateAPool(Cell* InCell)
{
	if (InCell->GetState() != State::Unknown)
	{
		return false;

		//throw std::logic_error("Trying to mark a cell black that is not unknown");
	}

	Grid GameBoardCopy(*this);

	auto CurrentCell = GameBoardCopy.operator()(Coordinate2D(InCell->GetPosition().GetX(), InCell->GetPosition().GetY()));
	auto x = InCell->GetPosition().GetX();
	auto y = InCell->GetPosition().GetY();

	set<Cell*> Blacks;
	set<Cell*> CellsInSquare;

	//if (CurrentCell->GetState() != State::Black)
		GameBoardCopy.Mark(CurrentCell, State::Black);


	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			auto CurrentCell = GameBoardCopy.operator()(Coordinate2D(x, y));

			set<Cell*> Blacks;
			set<Cell*> CellsInSquare;

			if (CurrentCell->GetState() == State::Black)
			{
				CellsInSquare.insert(CurrentCell);

				auto RightCell = GameBoardCopy.operator()(Coordinate2D(x + 1, y));
				if (!RightCell) continue;
				CellsInSquare.insert(RightCell);

				auto BottomRightCell = GameBoardCopy.operator()(Coordinate2D(x + 1, y + 1));
				if (!BottomRightCell) continue;
				CellsInSquare.insert(BottomRightCell);

				auto BottomCell = GameBoardCopy.operator()(Coordinate2D(x, y + 1));
				if (!BottomCell) continue;
				CellsInSquare.insert(BottomCell);

				for (auto i = CellsInSquare.begin(); i != CellsInSquare.end(); ++i)
				{
					if ((*i)->GetState() == State::Black)
					{
						Blacks.insert(*i);
					}
				}

				if (Blacks.size() == 4)
				{
					return true;
				}
			}
		}
	}

	return false;
}



void Grid::SolveCheckFor2x2Pools()
{
	cout << "SolveCheckFor2x2Pools" << endl;

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


				//std::cout << "CellInSquare size " << CellsInSquare.size() << endl;

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
	// TODO: Is this needed?
	//SolveUpdateCompleteIslands();
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
	cout << "SolveExpandPartialNumberedRegionsWithOnePath" << endl;

	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			auto CurrentCell = operator()(Coordinate2D(x, y));
			if (!CurrentCell) continue;

			if (CurrentCell->GetState() != State::Unknown)
			{
				auto CellRegion = CurrentCell->GetRegion();
				if (!CellRegion) continue;

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


// NOTE: I think I accidently cover this logic for black cells in "SolveBlackHasToConnect();"
void Grid::SolvePartialWhiteRegionsWithOnlyOnePath()
{
	cout << "SolvePartialWhiteRegionsWithOnlyOnePath" << endl; 
	

	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			auto CurrentCell = operator()(Coordinate2D(x, y));
			if (!CurrentCell) continue;

			if (CurrentCell->GetState() != State::Unknown)
			{
				auto CellRegion = CurrentCell->GetRegion();
				if (!CellRegion) continue;

				if (CellRegion->IsWhite())
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


// TODO: Is it really okay to expland a black cell. STOP: I just realized I am checking the REGION of blacks for unknowns
// 
void Grid::SolvePartialBlackRegionsWithOnlyOnePath()
{
	cout << "SolvePartialBlackRegionsWithOnlyOnePath" << endl;

	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			auto CurrentCell = operator()(Coordinate2D(x, y));

			if (CurrentCell->GetState() != State::Unknown)
			{
				auto CellRegion = CurrentCell->GetRegion();
				if (CellRegion->IsBlack())
				{

					if (CellRegion->GetUnknownsAroundRegion().size() == 1)
					{
						Mark(*CellRegion->GetUnknownsAroundRegion().begin(), State::Black);
					}
				}
			}
		}
	}
}


void Grid::SolveStepFourUnreachableCells()
{
	cout << "SolveStepFourUnreachableCells" << endl;

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
			//	cout << "White regions is not empty but cant test for impossibly big regions yet" << endl;
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

	//for (auto& Reg : m_Regions)
	for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i)
	{
		if ((*i)->IsNumbered())
		{
			Result.insert((*i).get());
		}
	}

	return Result;
}

// A complete region is region->Number() == region->RegionSize()
void Grid::UpdateCompleteRegions(set<Region*>& InNumberedRegions) 
{
	//for (auto& Reg : InNumberedRegions)
	for (auto i = InNumberedRegions.begin(); i != InNumberedRegions.end(); ++i)
	{
		if ((*i)->GetNumber() == (*i)->RegionSize())
		{
			// The numbered region is complete
			// For each cell in the region mark all valid adjacent cells of the cell in that region black
			SetStateOfAllUnknownNeighborsToCellsInARegion(*i, State::Black);
		}
	}
}


void Grid::SetStateOfAllUnknownNeighborsToCellsInARegion(Region* InRegion, const State& InState)
{
	//cout << "Test" << endl;
	//if (!InRegion) return;

	if (InState == State::Black || InState == State::White)
	{
		if (InState == State::Black)
		{
			//cout << "HELLO" << endl;
			std::vector<Cell*> UnsToSetBlk;


			for (auto i = InRegion->GetUnknownsAroundRegion().begin(); i != InRegion->GetUnknownsAroundRegion().end(); ++i)
			{
				UnsToSetBlk.push_back(*i);
			}
			cout << "HELLO ADDED" << endl;
			cout << UnsToSetBlk.size() << endl;
	


			for (auto& UnkCell : UnsToSetBlk)
			{
				cout << "HELLO MARKING " << *UnkCell << endl;
			
				Mark(UnkCell, State::Black);
			}

			//MarkRegUnknownsBlack(InRegion);

			//for (auto i = UnsToSetBlk.begin(); i != UnsToSetBlk.end(); ++i)
			//{
			//	cout << UnsToSetBlk.size() << endl;
			//	//if ((*i)->GetState() != State::Black)
			//		Mark(*i, State::Black);

			//}
			//for (auto& CellInRegion : InRegion->GetUnknownsAroundRegion())
			//{
			//	//For_All_Valid_Unknown_Neighbors(CellInRegion,
			//	//	[this](Cell* NeighborCell) -> auto
			//	//{
			//		Mark(UnknownCellAroundRegion-, State::Black);
			//	//});
			//}
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

// TODO: This function turned out to be garbage.
// UPDATE: Found another issue with expanding black cells that makes me want to rework this function even more.
// - I didn't consider the fact that the neighbor function we were testing could have already had a connecting path.
// - At this point it only covers the logic for black cells that didn't already have a path like a corner square etc..
// - instead of rewriting the function I am thinking I will just add another function that covers the other logic....
void Grid::SolveBlackHasToConnect()
{
	//cout << "SolveBlackHasToConnect" << endl;

	//// TODO: There is some nice logic in here that I can use later for needing to test copies of a board
	//// - and nested neighbor testing

	//// For every cell on the board....
	//for (int x = 0; x < m_Width; ++x)
	//{
	//	for (int y = 0; y < m_Height; ++y)
	//	{
	//		auto TestedCell = operator()(Coordinate2D(x, y));

	//		cout << *TestedCell << endl;

	//		// Make sure we are testing an unknown square
	//		if (TestedCell->GetState() != State::Unknown)
	//		{
	//			continue;
	//		}

	//		// Create a copy of our Gameboard to test changing this unknown cell to a white square
	//		Grid TempGameBoard(*this);
	//		auto TempTestedCell = TempGameBoard.operator()(Coordinate2D(x, y));

	//		TempGameBoard.Mark(TempTestedCell, State::White);

	//		//set<Cell*> AlreadyTakenPath;
	//		//TempGameBoard.For_All_Valid_Neighbors(TempTestedCell, [](Cell* NeighborCells)
	//		//{
	//		//	if (NeighborCells->GetState() == State::Black)
	//		//	{
	//		//		AlreadyTakenPath 
	//		//	}
	//		//});

	//		// Did marking this cell white make it so its neighbors couldn't connect?
	//		TempGameBoard.For_All_Valid_Neighbors(TempTestedCell,
	//			[this, &TempGameBoard, &TestedCell, &TempTestedCell, x, y](Cell* NeighborCell) -> auto
	//		{
	//			if (NeighborCell->GetState() == State::Black)
	//			{
	//				auto HasPath = TempGameBoard.BlackCellHasAtLeastOnePath(NeighborCell);
	//				cout << "Test" << endl;
	//				if (HasPath == false) // we should have a counter of all black cells 
	//					// that would make this really easy 
	//					// we could check if ( unknowns == 0 ) and 
	//					// if ( blackregion.size() < amount of black cells on the board ) etc....
	//				{
	//					Grid TempGameBoardCopy(*this);
	//					auto TempTestedCellCopy = TempGameBoardCopy.operator()(Coordinate2D(x, y));

	//					if (TempGameBoardCopy.DoesMarkBlackCreateAPool(TempTestedCellCopy) == false)
	//					{
	//						if (TestedCell->GetState() != State::Black)
	//							Mark(TestedCell, State::Black);
	//					}
	//				}
	//			}
	//		});
	//	}

	//}
}

bool Grid::BlackCellHasAtLeastOnePath(Cell* NeighborCell)
{
	//bool HasAtLeastOnePossiblePathway = false;
	//set<Cell*> Pathways;

	//For_All_Valid_Neighbors(InBlackCell, [&Pathways](Cell* NeighborCell) -> auto						
	//{ 
	//	auto NeighborRegion = NeighborCell->GetRegion();


	//	//if (NeighborCell->GetState() == State::Black || NeighborCell->GetState() == State::Unknown)
	//	//{
	//	//	Pathways.insert(NeighborCell);
	//	//}

	//	
	//});

	//	auto NeighborRegion = NeighborCell->GetRegion();
	auto NeighborRegion = NeighborCell->GetRegion();
	if (!NeighborRegion)
	{
		cout << "Cell " << *NeighborCell << " has no region" << endl;
		return false;
	}

	if (NeighborRegion->GetUnknownsAroundRegion().size() == 0)
	{
		return false;
	}

	//if (Pathways.size() == 0)
	//{
	//	return false;
	//}

	return true;
}

void Grid::SolveUpdateCompleteIslands()
{
	cout << "SolveUpdateCompleteIslands" << endl;

	auto NumberedRegions = GetAllNumberedRegions();
	UpdateCompleteRegions(NumberedRegions);
}

void Grid::SolveCellsWithTwoAdjacentNumberedCells()
{
	cout << "SolveCellsWithTwoAdjacentNumberedCells" << endl;
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
	// If we don't have two different regions, we're done. 
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
		std::cout << "Contradiction FOUND" << std::endl;
		return;
		//throw std::logic_error("Tried to fuse two numbered regions, Do I need to allow this for copied grid testin purposes?");
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
		//operator()(Coordinate2D((*i)->GetPosition().GetX(), (*i)->GetPosition().GetY()))->SetRegion(LHSRegion);
		//region(i->first, i->second) = r1; 
	}

	// Erase the secondary region from the set of all regions.
	// When this function returns, the secondary region will be destroyed.
		
	//m_regions.erase(r2);
	for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i)
	{
		if ((*i).get() == RHSRegion)
		{
			//std::cout << *RHSRegion << endl;
			m_Regions.erase(i);
			return;
		}
	}
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