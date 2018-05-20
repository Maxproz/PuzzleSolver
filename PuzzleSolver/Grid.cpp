
#include "Grid.h"

#include <iostream>
#include <algorithm> // for std::none_of
#include <queue>	// for the queue used in the unreachable() function
#include <string>  // std::to_string
#include <array> // for the array we use in SolvePreventPoolsTwoBlackTwoUnknown
#include <random> // for uniform_int_distribution in Guessing() function
#include <iterator> // for st

#include "Cell.h"
#include "Region.h"

using namespace std;

enum class SolveStatus
{
	KEEP_GOING, 
	CONTRADICTION_FOUND,
	SOLUTION_FOUND,
	CANNOT_PROCEED
};

// Unused atm | m_total_black(width * height)




Grid::Grid(int Width, int Height, std::vector<std::pair<Coordinate2D, int>> NumberedCellLocations) :
	m_Width{ Width }, m_Height{ Height }, m_Cells(), m_Regions(), m_total_black(Width * Height),
	m_prng(1251456), m_sitrep(SolveStatus::KEEP_GOING) 

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
	m_Width{ Copy.m_Width }, m_Height{ Copy.m_Height }, m_Cells(), m_Regions(),
	m_total_black(Copy.m_total_black), m_prng(Copy.m_prng), m_sitrep(Copy.m_sitrep)
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



	////std::set<std::unique_ptr<Region>>  TempRegionVec;
	////std::map<Region*, std::set<Cell*>> TempCacheVec;
	//std::vector<std::pair<Region*, std::set<Cell*>>> TempCacheVec;

	//for (auto AllCacheCopy = Copy.m_Cache.begin(); AllCacheCopy != Copy.m_Cache.end(); ++AllCacheCopy)
	//{

	//	
	//	//auto NewObj = std::make_pair<Region*, std::set<Cell*>>(make_pair(
	//	//	new Region((*AllCacheCopy).first), new ((**AllRegCopy)));
	//	cout << *(AllCacheCopy->first) << endl;
	//	auto RegionToInsert = new Region(*(AllCacheCopy->first));
	//	//auto Reg = new Region()

	//	auto SetToInsert = std::set<Cell*>();
	//	for (auto cell : (*AllCacheCopy).second)
	//	{
	//		SetToInsert.insert(new Cell(*cell));
	//	}

	//	TempCacheVec.push_back(make_pair(RegionToInsert, SetToInsert));
	//	//TempCacheMap.insert(make_pair(RegionToInsert, SetToInsert));
	//}

	//for (auto CacheCopy = TempCacheVec.begin(); CacheCopy != TempCacheVec.end(); ++CacheCopy)
	//{
	//	m_Cache.insert(*CacheCopy);
	//}
	//m_Cache = std::move(Copy.m_Cache);

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
	
	
	std::swap(m_prng, other.m_prng);
	std::swap(m_sitrep, other.m_sitrep);
	std::swap(m_total_black, other.m_total_black);

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

SolveStatus Grid::SolvePuzzle(bool Verbose, bool Guessing)
{

	std::map<Region*, set<Cell*>> cache;
	//std::map<Region*, std::set<Cell*>> m_Cache;



	{
		// Look for contradictions. Do this first.
		const string z = DetectContradictions(cache);

		if (!z.empty())
		{
			if (Verbose)
			{
				cout << z << endl;// print(s);
			}

			return SolveStatus::CONTRADICTION_FOUND;
		}
	}
	//m_Cache = std::move(cache);

		// See if we're done. Do this second.

	if (Known() == m_Width * m_Height)
	{
		if (Verbose)
		{
			cout << " I'm done" << endl;// print("I'm done!");
		}

		return SolveStatus::SOLUTION_FOUND;
	}

	std::set<Cell*> mark_as_white;
	std::set<Cell*> mark_as_black;
	//auto res12 = SolveUpdateCompleteIslands(Verbose);
	//if (res12)
	//{
	//	return m_sitrep;
	//}


		// Look for complete islands.
	
		for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i) {
			const Region& r = **i;
	
			if (r.IsNumbered() && r.RegionSize() == r.GetNumber())
			{
				mark_as_black.insert(r.UnknownsBegin(), r.UnknownsEnd());
			}
		}
	
		if (Process(Verbose, mark_as_black, mark_as_white, "Complete islands found. x1")) 
		{
			return m_sitrep;
		}



		//auto res9 = SolveStepFiveNSizeTwoChoices(Verbose);
		//if (res9)
		//{
		//	cout << "x3" << endl;
		//	return m_sitrep;
		//}


		// Look for partial regions that can expand into only one cell. They must expand.
	
	for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i) {
		 Region* r = (*i).get();

		const bool partial =
			r->IsBlack() && r->RegionSize() < m_total_black
			|| r->IsWhite()
			|| r->IsNumbered() && r->RegionSize() < r->GetNumber();

		if (partial && r->UnknownsSize() == 1)
		{
			if (r->IsBlack())
			{
				//if ((*r.UnknownsBegin())->GetState() != State::Unknown) continue;
				mark_as_black.insert(*r->UnknownsBegin());
			}
			else
			{
				//if ((*r.UnknownsBegin())->GetState() != State::Unknown) continue;
				mark_as_white.insert(*r->UnknownsBegin());
			}
		}
	}
	
	if (Process(Verbose, mark_as_black, mark_as_white,
		"Expanded partial regions with only one liberty. x2"))
	{
		return m_sitrep;
	}



	//SolveCellsWithTwoAdjacentNumberedCells(Verbose);





	//auto res11111 = SolvePartialWhiteRegionsWithOnlyOnePath(Verbose);
	//if (res11111)
	//{
	//	return m_sitrep;
	//}




		// Look for N - 1 islands with exactly two diagonal liberties.
	
	for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i) {
		 Region* r = (*i).get();

		if (r->IsNumbered() && r->RegionSize() == r->GetNumber() - 1 && r->UnknownsSize() == 2)
		{
			const int x1 = (*r->UnknownsBegin())->GetPosition().GetX();
			const int y1 = (*r->UnknownsBegin())->GetPosition().GetY();
			const int x2 = next((*r->UnknownsBegin()))->GetPosition().GetX();
			const int y2 = next((*r->UnknownsBegin()))->GetPosition().GetY();

			auto FirstUnKCell = (*r->UnknownsBegin());
			auto SecondUnKCell = next((*r->UnknownsBegin()));

			if (abs(FirstUnKCell->GetPosition().GetX() - SecondUnKCell->GetPosition().GetX()) == 1 &&
				abs(FirstUnKCell->GetPosition().GetY() - SecondUnKCell->GetPosition().GetY()) == 1)
			{
				pair<int, int> p;

				if (r->Contains(operator()(Coordinate2D(FirstUnKCell->GetPosition().GetX(), SecondUnKCell->GetPosition().GetY()))))
				{
					p = make_pair(x2, y1);
				}
				else
				{
					p = make_pair(x1, y2);
				}

				// The far cell might already be black, in which case there's nothing to do.
				// It could even be white/numbered (if it's part of this island), in which case
				// there's still nothing to do.
				// (If it's white/numbered and not part of this island,
				// we'll eventually detect a contradiction.)

				if (operator()(Coordinate2D(p.first, p.second))->GetState() == State::Unknown)
				{
					mark_as_black.insert(operator()(Coordinate2D(p.first, p.second)));
				}
			}
		}
	}

	if (Process(Verbose, mark_as_black, mark_as_white,
		"N - 1 islands with exactly two diagonal liberties found. x3")) 
	{
		return m_sitrep;
	}
	




	//auto res0 = SolveStepFourUnreachableCells(Verbose);
	//if (res0)
	//{
	//	return m_sitrep;
	//}

	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			//auto CurrentCell = operator()(Coordinate2D(x, y));
			if (Unreachable(operator()(Coordinate2D(x, y))))
			{
				mark_as_black.insert(operator()(Coordinate2D(x, y)));
			}
		}
	}

	if (Process(Verbose, mark_as_black, mark_as_white,
		"SolveStepFourUnreachableCells x4"))
	{
		return m_sitrep;
	}






	//auto res8 = SolveCheckFor2x2Pools(Verbose);
	//if (res8)
	//{
	//	return m_sitrep;
	//}

	//// First thing, clean up the debug code so you can actually make sense of what is going on.
	//auto res1 = SolvePreventPoolsTwoBlackTwoUnknown(Verbose);
	//if (res1)
	//{
	//	return m_sitrep;
	//}












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
			if (a[0].state == State::Unknown
				&& a[1].state == State::Black
				&& a[2].state == State::Black
				&& a[3].state == State::Black)
			{

				//mark_as_white.insert(make_pair(a[0].x, a[0].y));
				mark_as_white.insert(operator()(Coordinate2D(a[0].x, a[0].y)));

			}
			//TODO: Study how this is passing imagine_black over.
			else if (a[0].state == State::Unknown
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


	if (Process(Verbose, mark_as_black, mark_as_white, "Whitened cells to prevent pools. x5")) {
		return m_sitrep;
	}


	//for (auto& CellToMarkWhite : mark_as_white)
	//{
	//	Mark(CellToMarkWhite, State::White);
	//}


	//if (Process(Verbose, mark_as_black, mark_as_white, "Whitened cells to prevent pools."))
	//{
	//	return true;// m_sitrep;
	//}
	//else
	//{
	//	return false;
	//}




	//// I was wrong. I also need a function that well find "Isolated Unknown Cells"
	//auto res3 = SolveIsolatedUnknownCells(Verbose);
	//if (res3)
	//{
	//	return m_sitrep;
	//}

	auto RegionsVec = std::vector<Region*>{};
	for (auto i = m_Regions.begin(); i != m_Regions.end(); ++i)
	{
		RegionsVec.push_back((*i).get());
	}

	const bool any_black_regions = any_of(RegionsVec.begin(), RegionsVec.end(), [](Region* r) { return r->IsBlack(); });


	//set<pair<int, int>> analyzed;

	set<Cell*> analyzed;

	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			//auto InCell = );

			//if (cell(x, y) == State::Unknown && analyzed.find(make_pair(x, y)) == analyzed.end()) 
			if (operator()(Coordinate2D(x, y))->GetState() == State::Unknown && analyzed.find(operator()(Coordinate2D(x, y))) == analyzed.end())
			{
				bool encountered_black = false;

				//set<pair<int, int>> open;
				//set<pair<int, int>> closed;

				set<Cell*> open;
				set<Cell*> closed;

				open.insert(operator()(Coordinate2D(x, y)));

				while (!open.empty())
				{
					//const pair<int, int> p = *open.begin();
					Cell* p = *open.begin();
					open.erase(open.begin());


					//switch (cell(p.first, p.second)) 
					switch (operator()(Coordinate2D(p->GetPosition().GetX(), p->GetPosition().GetY()))->GetState())
					{
						case State::Unknown:
							if (closed.insert(p).second)
							{
								//insert_valid_neighbors(open, p.first, p.second);

								For_All_Valid_Neighbors(operator()(Coordinate2D((p)->GetPosition().GetX(),
									(p)->GetPosition().GetY())), [&open](Cell* NeighborCell) -> auto
								{
									open.insert(NeighborCell);
								});
							}

							break;

						case State::Black:
							encountered_black = true;
							break;

						default:
							break;
					}
				}

				if (!encountered_black && (
					any_black_regions || static_cast<int>(closed.size()) < m_total_black)) {

					mark_as_white.insert(closed.begin(), closed.end());
				}

				analyzed.insert(closed.begin(), closed.end());
			}
		}
	}

	if (Process(Verbose, mark_as_black, mark_as_white, "SolveIsolatedUnknownCells x6")) {
		return m_sitrep;
	}




	//auto res2 = SolveConfinementAnalysis(cache, Verbose);
	//if (res2)
	//{
	//	return m_sitrep;
	//}



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

					if (Confined((*i).get(), cache, ForbiddenCells))
					{
						if (r.IsBlack())
						{
							//mark_as_black.insert(make_pair(x, y));
							mark_as_black.insert(operator()(Coordinate2D(x, y)));
							//mark_as_black.insert(CurrentCell);

						}
						else
						{
							//mark_as_white.insert(make_pair(x, y));
							mark_as_white.insert(operator()(Coordinate2D(x, y)));
							//mark_as_white.insert(CurrentCell);
							//CurrentCell
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
					if (k != i && (*k)->IsNumbered() && Confined((*k).get(), cache, ForbiddenCells))
					{
						mark_as_black.insert(*u);
					}
				}
			}
		}
	}


	if (Process(Verbose, mark_as_black, mark_as_white, "SolveConfinementAnalysis x7")) {
		return m_sitrep;
	}

	//SolveConfinementAnalysis


	//auto res11 = SolvePartialBlackRegionsWithOnlyOnePath(Verbose);
	//if (res11)
	//{
	//	return m_sitrep;
	//}



	//auto res1111 = SolveExpandPartialNumberedRegionsWithOnePath(Verbose);
	//if (res1111)
	//{
	//	return m_sitrep;
	//}



	//
	//// I was wrong. I also need a function that well find "Isolated Unknown Cells"
	//auto res3 = SolveIsolatedUnknownCells(Verbose);
	//if (res3)
	//{
	//	return m_sitrep;
	//}





	if (Guessing)
	{


		//vector<pair<int, int>> v;

		vector<Cell*> v;

		for (int x = 0; x < m_Width; ++x)
		{
			for (int y = 0; y < m_Height; ++y)
			{
				auto CurrentCell = operator()(Coordinate2D(x, y));

				if (CurrentCell->GetState() == State::Unknown)
				{
					v.push_back(CurrentCell);//make_pair(CurrentCell->GetPosition().GetX(), CurrentCell->GetPosition().GetY()));
				}
			}
		}


		// Guess cells in a deterministic but pseudorandomized order.
		// This attempts to avoid repeatedly guessing cells that won't get us anywhere.

		auto dist = [this](const ptrdiff_t n) {
			// random_shuffle() provides n > 0. It wants [0, n).
			// uniform_int_distribution's ctor takes a and b with a <= b. It produces [a, b].
			return uniform_int_distribution<ptrdiff_t>(0, n - 1)(m_prng);
		};

		random_shuffle(v.begin(), v.end(), dist);


		for (auto u = v.begin(); u != v.end(); ++u)
		{
			const auto x = (*u)->GetPosition().GetX();
			const auto y = (*u)->GetPosition().GetY();


			for (int i = 0; i < 2; ++i)
			{
				const State color = i == 0 ? State::Black : State::White;
				auto& mark_as_diff = i == 0 ? mark_as_white : mark_as_black;
				auto& mark_as_same = i == 0 ? mark_as_black : mark_as_white;
				//auto& mark_as_same = i == 0 ? mark_as_white : mark_as_black;
				//auto& mark_as_diff = i == 0 ? mark_as_black : mark_as_white;
				
				//mark_as_same
				Grid other(*this);

				//auto OthersCurrentCell = other.operator()(Coordinate2D(x, y));

				other.Mark(other.operator()(Coordinate2D(x, y)), color);//(*u).first, (*u).second)), color);

				SolveStatus sr = SolveStatus::KEEP_GOING;

				while (sr == SolveStatus::KEEP_GOING)
				{
					sr = other.SolvePuzzle(false, false);
					//cout << "Priting other grid" << endl;
					//other.PrintGrid();

				}

				if (sr == SolveStatus::CONTRADICTION_FOUND)
				{
					//mark_as_diff.insert(make_pair(x, y));

					mark_as_diff.insert(operator()(Coordinate2D(x, y)));
					//mark_as_diff.insert(*u);
					//mark_as_diff.insert(operator()(Coordinate2D((*u)->GetPosition().GetX(), (*u)->GetPosition().GetY())));

					//mark_as_diff.insert(operator()(Coordinate2D((*u).first, (*u).second)));

					Process(Verbose, mark_as_black, mark_as_white,
						"Hypothetical contradiction found. x8");



					//for (auto& CellToMarkBlack : mark_as_black)
					//{
					//	Mark(CellToMarkBlack, State::Black);
					//}

					//cout << "MarkAsWhiteSize() == " << mark_as_white.size() << endl;

					//for (auto& CellToMarkWhite : mark_as_white)
					//{
					//	Mark(CellToMarkWhite, State::White);
					//}

					//other.m_sitrep = SolveStatus::CONTRADICTION_FOUND;
					//return SolveStatus::CONTRADICTION_FOUND;
					return m_sitrep;
				}

				if (sr == SolveStatus::SOLUTION_FOUND)
				{
					//mark_as_same.insert(make_pair(x, y));

					mark_as_same.insert(operator()(Coordinate2D(x, y)));
					//mark_as_same.insert(operator()(Coordinate2D((*u).first, (*u).second)));
					//other.Mark(other.operator()(Coordinate2D(, color);


					Process(Verbose, mark_as_black, mark_as_white,
						"Hypothetical solution found. x9");

					//for (auto& CellToMarkBlack : mark_as_black)
					//{
					//	Mark(CellToMarkBlack, State::Black);
					//}

					//cout << "MarkAsWhiteSize() == " << mark_as_white.size() << endl;

					//for (auto& CellToMarkWhite : mark_as_white)
					//{
					//	Mark(CellToMarkWhite, State::White);
					//}
					//m_sitrep = SolveStatus::SOLUTION_FOUND;
					return m_sitrep;


					//return m_sitrep;
				}

				// sr == CANNOT_PROCEED
			}
		}
	}

	//m_sitrep = SolveStatus::CANNOT_PROCEED;
	//return m_sitrep;



	if (Verbose)
	{
		cout << ("I'm stumped!") << endl;
	}

	return SolveStatus::CANNOT_PROCEED;



	// TODO: In the "Should black region expand function
	// make a check that will iterate over all the unknowns in a region
	// if marking an unknown cell in that region black. will merge every black cell into one region, then do it.



}



bool Grid::Process(const bool verbose, const set<Cell*>& mark_as_black, const set<Cell*>& mark_as_white, const string & s)
{
	if (mark_as_black.empty() && mark_as_white.empty()) 
	{
		return false;
	}
	
	for (auto i = mark_as_black.begin(); i != mark_as_black.end(); ++i)
	{
		Mark(operator()(Coordinate2D((*i)->GetPosition().GetX(), (*i)->GetPosition().GetY())), State::Black);
	}
	
	for (auto i = mark_as_white.begin(); i != mark_as_white.end(); ++i)
	{
		Mark(operator()(Coordinate2D((*i)->GetPosition().GetX(), (*i)->GetPosition().GetY())), State::White);
	}
	
	
	//PrintGrid();

	if (verbose)
	{
		set<Cell*> updated(mark_as_black);
		updated.insert(mark_as_white.begin(), mark_as_white.end());

		string t = s;

		if (m_sitrep == SolveStatus::CONTRADICTION_FOUND) 
		{
			t += " (Contradiction found! Attempted to fuse two numbered regions"
				" or mark an already known cell.)";
		}

		//print(t, updated);
		cout << t << std::endl;
	}
	
	return true;
}

int Grid::Known() const
{
	int ret = 0;
	
	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			auto CurrentCell = operator()(Coordinate2D(x, y));
			
			if (CurrentCell->GetState() != State::Unknown)
			{
				++ret;
			}
		}
	}
	
	return ret;
}

SolveStatus Grid::SolveGuessingRemaining(bool IsVerbose, bool IsGuessing)
{
	//cout << "SolveGuessingRemaining" << endl;

	std::set<Cell*> mark_as_white;
	std::set<Cell*> mark_as_black;


	if (IsGuessing)
	{
		//vector<pair<int, int>> v;

		vector<Cell*> v;

		for (int x = 0; x < m_Width; ++x)
		{
			for (int y = 0; y < m_Height; ++y) 
			{
				auto CurrentCell = operator()(Coordinate2D(x, y));

				if (CurrentCell->GetState() == State::Unknown)
				{
					v.push_back(CurrentCell);
				}
			}
		}


		// Guess cells in a deterministic but pseudorandomized order.
		// This attempts to avoid repeatedly guessing cells that won't get us anywhere.

		auto dist = [this](const ptrdiff_t n) {
			// random_shuffle() provides n > 0. It wants [0, n).
			// uniform_int_distribution's ctor takes a and b with a <= b. It produces [a, b].
			return uniform_int_distribution<ptrdiff_t>(0, n - 1)(m_prng);
		};

		random_shuffle(v.begin(), v.end(), dist);


		for (auto u = v.begin(); u != v.end(); ++u) 
		{
			//const auto x = (*u)->GetPosition().GetX();
			//const auto y = (*u)->GetPosition().GetY();


			for (int i = 0; i < 2; ++i) 
			{
				const State color = i == 0 ? State::Black : State::White;
				auto& mark_as_diff = i == 0 ? mark_as_white : mark_as_black;
				auto& mark_as_same = i == 0 ? mark_as_black : mark_as_white;

				Grid other(*this);

				//auto OthersCurrentCell = other.operator()(Coordinate2D(x, y));

				other.Mark(other.operator()(Coordinate2D((*u)->GetPosition().GetX(), (*u)->GetPosition().GetY())), color);

				SolveStatus sr = SolveStatus::KEEP_GOING;

				while (sr == SolveStatus::KEEP_GOING) 
				{
					sr = other.SolvePuzzle(false, false);
					cout << "Priting other grid" << endl;
					other.PrintGrid();

				}

				if (sr == SolveStatus::CONTRADICTION_FOUND)
				{
					//mark_as_diff.insert(make_pair(x, y));

					//mark_as_diff.insert(operator()(Coordinate2D(x, y)));
					//mark_as_diff.insert(*u);
					mark_as_diff.insert(operator()(Coordinate2D((*u)->GetPosition().GetX(), (*u)->GetPosition().GetY())));

					Process(IsVerbose, mark_as_black, mark_as_white,
						"Hypothetical contradiction found.");



					//for (auto& CellToMarkBlack : mark_as_black)
					//{
					//	Mark(CellToMarkBlack, State::Black);
					//}

					//cout << "MarkAsWhiteSize() == " << mark_as_white.size() << endl;

					//for (auto& CellToMarkWhite : mark_as_white)
					//{
					//	Mark(CellToMarkWhite, State::White);
					//}

					//other.m_sitrep = SolveStatus::CONTRADICTION_FOUND;
					//return SolveStatus::CONTRADICTION_FOUND;
					return m_sitrep;
				}

				if  (sr == SolveStatus::SOLUTION_FOUND)
				{
					//mark_as_same.insert(make_pair(x, y));
				
					//mark_as_same.insert(operator()(Coordinate2D(x, y)));
					mark_as_same.insert(operator()(Coordinate2D((*u)->GetPosition().GetX(), (*u)->GetPosition().GetY())));

					Process(IsVerbose, mark_as_black, mark_as_white,
						"Hypothetical solution found.");

					//for (auto& CellToMarkBlack : mark_as_black)
					//{
					//	Mark(CellToMarkBlack, State::Black);
					//}

					//cout << "MarkAsWhiteSize() == " << mark_as_white.size() << endl;

					//for (auto& CellToMarkWhite : mark_as_white)
					//{
					//	Mark(CellToMarkWhite, State::White);
					//}
					//other.m_sitrep = SolveStatus::SOLUTION_FOUND;
					return m_sitrep;


					//return m_sitrep;
				}

				// sr == CANNOT_PROCEED
			}
		}
	}

	m_sitrep = SolveStatus::CANNOT_PROCEED;
	return m_sitrep;

	//cout << "MarkAsBlackSize() == " << mark_as_black.size() << endl;

	//for (auto& CellToMarkBlack : mark_as_black)
	//{
	//	Mark(CellToMarkBlack, State::Black);
	//}

	//cout << "MarkAsWhiteSize() == " << mark_as_white.size() << endl;

	//for (auto& CellToMarkWhite : mark_as_white)
	//{
	//	Mark(CellToMarkWhite, State::White);
	//}


}
//



bool Grid::SolveIsolatedUnknownCells(bool Verbose)
{
	// TODO: Need to filter this

	//cout << "SolveIsolatedUnknownCells" << endl;

	// Look for isolated unknown regions.

	std::set<Cell*> mark_as_white;
	std::set<Cell*> mark_as_black;


	const bool any_black_regions = any_of(m_Regions.begin(), m_Regions.end(),
		[](auto& r) { return r->IsBlack(); });


	//set<pair<int, int>> analyzed;

	set<Cell*> analyzed;

	for (int x = 0; x < m_Width; ++x) 
	{
		for (int y = 0; y < m_Height; ++y) 
		{
			//auto InCell = );

			//if (cell(x, y) == State::Unknown && analyzed.find(make_pair(x, y)) == analyzed.end()) 
			if (operator()(Coordinate2D(x, y))->GetState() == State::Unknown && analyzed.find(operator()(Coordinate2D(x, y))) == analyzed.end())
			{
				bool encountered_black = false;

				//set<pair<int, int>> open;
				//set<pair<int, int>> closed;

				set<Cell*> open;
				set<Cell*> closed;

				open.insert(operator()(Coordinate2D(x, y)));

				while (!open.empty())
				{
					//const pair<int, int> p = *open.begin();
					Cell* p = *open.begin();
					open.erase(open.begin());


					//switch (cell(p.first, p.second)) 
					switch (operator()(Coordinate2D(p->GetPosition().GetX(), p->GetPosition().GetY()))->GetState())
					{
						case State::Unknown:
							if (closed.insert(p).second) 
							{
								//insert_valid_neighbors(open, p.first, p.second);

								For_All_Valid_Neighbors(operator()(Coordinate2D((p)->GetPosition().GetX(),
									(p)->GetPosition().GetY())), [&open](Cell* NeighborCell) -> auto
								{
									open.insert(NeighborCell);
								});
							}

							break;

						case State::Black:
							encountered_black = true;
							break;

						default:
							break;
					}
				}

				if (!encountered_black && (
					any_black_regions || static_cast<int>(closed.size()) < m_total_black)) {

					mark_as_white.insert(closed.begin(), closed.end());
				}

				analyzed.insert(closed.begin(), closed.end());
			}
		}
	}



	//cout << "MarkAsBlackSize() == " << mark_as_black.size() << endl;

	//for (auto& CellToMarkBlack : mark_as_black)
	//{
	//	Mark(CellToMarkBlack, State::Black);
	//}

	//cout << "MarkAsWhiteSize() == " << mark_as_white.size() << endl;

	//for (auto& CellToMarkWhite : mark_as_white)
	//{
	//	Mark(CellToMarkWhite, State::White);
	//}

	//const bool Verbose = true;
	if (Process(Verbose, mark_as_black, mark_as_white, "Isolated unknown regions found."))
	{
		return true;// m_sitrep;
	}
	else
	{
		return false;
	}

}





bool Grid::SolveConfinementAnalysis(std::map<Region*, set<Cell*>>& cache, bool Verbose)
{
	//cout << "SolveConfinementAnalysis" << endl;



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

					if (Confined((*i).get(), cache, ForbiddenCells))
					{
						if (r.IsBlack())
						{
							//mark_as_black.insert(make_pair(x, y));
							mark_as_black.insert(operator()(Coordinate2D(x, y)));
							//mark_as_black.insert(CurrentCell);
						
						}
						else
						{
							//mark_as_white.insert(make_pair(x, y));
							mark_as_white.insert(operator()(Coordinate2D(x, y)));
							//mark_as_white.insert(CurrentCell);
							//CurrentCell
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
					if (k != i && (*k)->IsNumbered() && Confined((*k).get(), cache, ForbiddenCells)) 
					{
						mark_as_black.insert(*u);
					}
				}
			}
		}
	}
	
	//cout << "MarkAsBlackSize() == " << mark_as_black.size() << endl;
	//
	//for (auto& CellToMarkBlack : mark_as_black)
	//{
	//	Mark(CellToMarkBlack, State::Black);
	//}

	//cout << "MarkAsWhiteSize() == " << mark_as_white.size() << endl;

	//for (auto& CellToMarkWhite : mark_as_white)
	//{
	//	Mark(CellToMarkWhite, State::White);
	//}

	//const bool Verbose = true;
	if (Process(Verbose, mark_as_black, mark_as_white, "Confinement analysis succeeded.")) 
	{
		return true; // m_sitrep;
	}
	else
	{
		return false;
	}

}

bool Grid::Confined(Region* r, std::map<Region*, std::set<Cell*>>& cache, const std::set<Cell*>& forbidden)
{

	// When we look for contradictions, we run confinement analysis (A) without forbidden cells.
	// This gives us an opportunity to accelerate later confinement analysis (B)
	// when we have forbidden cells.
	// During A, we'll record the unknown cells that we consumed.
	// During B, if none of the forbidden cells are ones that we consumed,
	// then the forbidden cells can't confine us.

	//cache = m_Cache;

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
			if (!area)
			{
				// Keep going. A white region can consume an unknown cell.
			}
			else if (area->IsBlack())
			{
				continue; // We can't consume this. Discard it.
			}
			else if (area->IsWhite()) 
			{
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
	//auto hasPool = SolveCheckFor2x2Pools(Verbose);
	//auto HasPools = SolvePreventPoolsTwoBlackTwoUnknown(Verbose);

	for (int x = 0; x < m_Width - 1; ++x)
	{
		for (int y = 0; y < m_Height - 1; ++y)
		{
			if (operator()(Coordinate2D(x, y))->GetState() == State::Black
				&& operator()(Coordinate2D(x + 1, y))->GetState() == State::Black
				&& operator()(Coordinate2D(x, y + 1))->GetState() == State::Black
				&& operator()(Coordinate2D(x + 1, y + 1))->GetState() == State::Black)
			{

				return "Contradiction found! Pool detected.";
			}
		}
	}


	//string temp; 
	//
	//if (hasPool || HasPools)
	//{
	//	return "Contradiction found! Pool detected.";
	//}



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





bool Grid::SolvePreventPoolsTwoBlackTwoUnknown(bool Verbose)
{
	// Look for squares of one unknown and three black cells, or two unknown and two black cells.
	std::set<Cell*> mark_as_white;
	std::set<Cell*> mark_as_black;

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
			if (a[0].state == State::Unknown
				&& a[1].state == State::Black
				&& a[2].state == State::Black
				&& a[3].state == State::Black)
			{

				//mark_as_white.insert(make_pair(a[0].x, a[0].y));
				mark_as_white.insert(operator()(Coordinate2D(a[0].x, a[0].y)));

			}
			 //TODO: Study how this is passing imagine_black over.
			else if (a[0].state == State::Unknown
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
	
	//for (auto& CellToMarkWhite : mark_as_white)
	//{
	//	Mark(CellToMarkWhite, State::White);
	//}



	if (Process(Verbose, mark_as_black, mark_as_white, "Whitened cells to prevent pools."))
	{
		return true;// m_sitrep;
	}
	else
	{
		return false;
	}


}


bool Grid::SolveStepFiveNSizeTwoChoices(bool Verbose)
{
	std::set<Cell*> mark_as_white;
	std::set<Cell*> mark_as_black;




	//cout << "SolveStepFiveNSizeTwoChoices" << endl;
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


					PrintGrid();

					//Mark(MaybeOtherCellToSet, State::Black);
					mark_as_black.insert(MaybeOtherCellToSet);

					if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolveStepFiveNSizeTwoChoices."))
					{
						return true;// m_sitrep;
					}
					else
					{
						return false;
					}
				}
				
			}
			else
			{
				if (MaybeCellToSet->GetState() == State::Unknown)
				{


					std::cout << "Two Unknowns Touch at corners trying to mark black " << *MaybeCellToSet << endl;

					PrintGrid();

					//Mark(MaybeCellToSet, State::Black);
					mark_as_black.insert(MaybeCellToSet);

					if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolveStepFiveNSizeTwoChoices."))
					{
						return true;// m_sitrep;
					}
					else
					{
						return false;
					}
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

					PrintGrid();

					//Mark(MaybeOtherCellToSet, State::Black);
					mark_as_black.insert(MaybeOtherCellToSet);

					if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolveStepFiveNSizeTwoChoices."))
					{
						return true;// m_sitrep;
					}
					else
					{
						return false;
					}
				}

			}
			else
			{
				if (MaybeCellToSet->GetState() == State::Unknown)
				{
					std::cout << "Two Unknowns Touch at corners trying to mark black " << *MaybeCellToSet << endl;

					PrintGrid();

					//Mark(MaybeCellToSet, State::Black);
					mark_as_black.insert(MaybeCellToSet);


					if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolveStepFiveNSizeTwoChoices."))
					{
						return true;// m_sitrep;
					}
					else
					{
						return false;
					}
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
					//Mark(MaybeOtherCellToSet, State::Black);
					mark_as_black.insert(MaybeOtherCellToSet);


					if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolveStepFiveNSizeTwoChoices."))
					{
						return true;// m_sitrep;
					}
					else
					{
						return false;
					}
				}

			}
			else
			{
				if (MaybeCellToSet->GetState() == State::Unknown)
				{
					std::cout << "Two Unknowns Touch at corners trying to mark black " << *MaybeCellToSet << endl;
					//Mark(MaybeCellToSet, State::Black);
					mark_as_black.insert(MaybeCellToSet);



					if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolveStepFiveNSizeTwoChoices."))
					{
						return true;// m_sitrep;
					}
					else
					{
						return false;
					}
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
					//Mark(MaybeOtherCellToSet, State::Black);
					mark_as_black.insert(MaybeOtherCellToSet);


					if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolveStepFiveNSizeTwoChoices."))
					{
						return true;// m_sitrep;
					}
					else
					{
						return false;
					}
				}

			}
			else
			{
				if (MaybeCellToSet->GetState() == State::Unknown)
				{
					std::cout << "Two Unknowns Touch at corners trying to mark black " << *MaybeCellToSet << endl;
					//Mark(MaybeCellToSet, State::Black);
					mark_as_black.insert(MaybeCellToSet);


					if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolveStepFiveNSizeTwoChoices."))
					{
						return true;// m_sitrep;
					}
					else
					{
						return false;
					}
				}
			}
		}
		else
		{
			cout << "End of testing of regions of size 2 with 2 unknown paths" << endl;
		}
	}


	if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolveStepFiveNSizeTwoChoices."))
	{
		return true;// m_sitrep;
	}
	else
	{
		return false;
	}

}

//	
//bool Grid::DoesMarkBlackCreateAPool(Cell* InCell)
//{
//	if (InCell->GetState() != State::Unknown)
//	{
//		return false;
//
//		//throw std::logic_error("Trying to mark a cell black that is not unknown");
//	}
//
//	Grid GameBoardCopy(*this);
//
//	auto CurrentCell = GameBoardCopy.operator()(Coordinate2D(InCell->GetPosition().GetX(), InCell->GetPosition().GetY()));
//	auto x = InCell->GetPosition().GetX();
//	auto y = InCell->GetPosition().GetY();
//
//	set<Cell*> Blacks;
//	set<Cell*> CellsInSquare;
//
//	//if (CurrentCell->GetState() != State::Black)
//	GameBoardCopy.Mark(CurrentCell, State::Black);
//
//
//	for (int x = 0; x < m_Width; ++x)
//	{
//		for (int y = 0; y < m_Height; ++y)
//		{
//			auto CurrentCell = GameBoardCopy.operator()(Coordinate2D(x, y));
//
//			set<Cell*> Blacks;
//			set<Cell*> CellsInSquare;
//
//			if (CurrentCell->GetState() == State::Black)
//			{
//				CellsInSquare.insert(CurrentCell);
//
//				auto RightCell = GameBoardCopy.operator()(Coordinate2D(x + 1, y));
//				if (!RightCell) continue;
//				CellsInSquare.insert(RightCell);
//
//				auto BottomRightCell = GameBoardCopy.operator()(Coordinate2D(x + 1, y + 1));
//				if (!BottomRightCell) continue;
//				CellsInSquare.insert(BottomRightCell);
//
//				auto BottomCell = GameBoardCopy.operator()(Coordinate2D(x, y + 1));
//				if (!BottomCell) continue;
//				CellsInSquare.insert(BottomCell);
//
//				for (auto i = CellsInSquare.begin(); i != CellsInSquare.end(); ++i)
//				{
//					if ((*i)->GetState() == State::Black)
//					{
//						Blacks.insert(*i);
//					}
//				}
//
//				if (Blacks.size() == 4)
//				{
//					return true;
//				}
//			}
//		}
//	}
//
//	return false;
//}



bool Grid::SolveCheckFor2x2Pools(bool Verbose)
{
	std::set<Cell*> mark_as_white;
	std::set<Cell*> mark_as_black;


	//cout << "SolveCheckFor2x2Pools" << endl;

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

				//Mark((*Unknown.begin()), State::White);
				mark_as_white.insert((*Unknown.begin()));
			}
		}
	}
	// TODO: Is this needed?
	//SolveUpdateCompleteIslands(Verbose);


	if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolveCheckFor2x2Pools."))
	{
		return true;// m_sitrep;
	}
	else
	{
		return false;
	}
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

bool Grid::SolveExpandPartialNumberedRegionsWithOnePath(bool Verbose)
{
	std::set<Cell*> mark_as_white;
	std::set<Cell*> mark_as_black;

	//cout << "SolveExpandPartialNumberedRegionsWithOnePath" << endl;

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
							//Mark(*CellRegion->GetUnknownsAroundRegion().begin(), State::White);
							mark_as_white.insert(*CellRegion->GetUnknownsAroundRegion().begin());
						}
					}
				}
			}
		}
	}


	if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolveExpandPartialNumberedRegionsWithOnePath."))
	{
		return true;// m_sitrep;
	}
	else
	{
		return false;
	}
}


// NOTE: I think I accidently cover this logic for black cells in "SolveBlackHasToConnect();"
bool Grid::SolvePartialWhiteRegionsWithOnlyOnePath(bool Verbose)
{
	std::set<Cell*> mark_as_white;
	std::set<Cell*> mark_as_black;


	//cout << "SolvePartialWhiteRegionsWithOnlyOnePath" << endl; 
	
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
						//Mark(*CellRegion->GetUnknownsAroundRegion().begin(), State::White);
						mark_as_white.insert(*CellRegion->GetUnknownsAroundRegion().begin());
					}
				}
			}
		}
	}


	if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolvePartialWhiteRegionsWithOnlyOnePath."))
	{
		return true;// m_sitrep;
	}
	else
	{
		return false;
	}


}


// TODO: Is it really okay to expland a black cell. STOP: I just realized I am checking the REGION of blacks for unknowns
// 
bool Grid::SolvePartialBlackRegionsWithOnlyOnePath(bool Verbose)
{
	std::set<Cell*> mark_as_white;
	std::set<Cell*> mark_as_black;


	//cout << "SolvePartialBlackRegionsWithOnlyOnePath" << endl;

	for (int x = 0; x < m_Width; ++x)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			auto CurrentCell = operator()(Coordinate2D(x, y));

			if (!CurrentCell) continue;

			if (CurrentCell->GetState() != State::Unknown)
			{
				auto CellRegion = CurrentCell->GetRegion();
				if (!CellRegion) continue;;

				if (CellRegion->IsBlack())
				{
					if (CellRegion->GetUnknownsAroundRegion().size() == 1)
					{
						//Mark(*CellRegion->GetUnknownsAroundRegion().begin(), State::Black);
						mark_as_black.insert(*CellRegion->GetUnknownsAroundRegion().begin());
					}
				}
			}
		}
	}

	if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SolvePartialBlackRegionsWithOnlyOnePath."))
	{
		return true;// m_sitrep;
	}
	else
	{
		return false;
	}
}


bool Grid::SolveStepFourUnreachableCells(bool Verbose)
{
	//cout << "SolveStepFourUnreachableCells" << endl;

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
	
	//for (auto i = Mark_as_black.begin(); i != Mark_as_black.end(); ++i)
	//{
	//	Mark((*i), State::Black);
	//}

	
	// Should this be called here, just to make sure it happens?
	//SolveUpdateCompleteIslands();

	if (Process(Verbose, Mark_as_black, Mark_as_white, "Unreachable cells blackened."))
	{
		return true;// m_sitrep;
	}
	else
	{
		return false;
	}
}



bool Grid::Unreachable(Cell* InCell, set<Cell*>& discovered) 
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
bool Grid::UpdateCompleteRegions(set<Region*>& InNumberedRegions, bool Verbose)
{
	int WasAddedCtr = 0;
	//for (auto& Reg : InNumberedRegions)
	for (auto i = InNumberedRegions.begin(); i != InNumberedRegions.end(); ++i)
	{
		if ((*i)->IsNumbered() && (*i)->GetNumber() == (*i)->RegionSize())
		{
			// The numbered region is complete
			// For each cell in the region mark all valid adjacent cells of the cell in that region black
			if ((*i)->IsBlack()) continue;

			auto res = SetStateOfAllUnknownNeighborsToCellsInARegion(*i, State::Black, Verbose);
			if (res)
				WasAddedCtr++;
		}
	}

	if (WasAddedCtr > 0)
	{
		return true;
	}

	return false;
}


bool Grid::SetStateOfAllUnknownNeighborsToCellsInARegion(Region* InRegion, const State& InState, bool Verbose)
{
	//cout << "Test" << endl;
	if (!InRegion) return false;

	std::set<Cell*> mark_as_white;
	std::set<Cell*> mark_as_black;


	if (InState == State::Black || InState == State::White)
	{
		if (InState == State::Black)
		{
			//cout << "HELLO" << endl;
			std::vector<Cell*> UnsToSetBlk;


			for (auto i = InRegion->UnknownsBegin(); i != InRegion->UnknownsEnd(); ++i)
			{
				UnsToSetBlk.push_back(*i);
			}
			//cout << "HELLO ADDED" << endl;
			//cout << UnsToSetBlk.size() << endl;
	


			for (auto& UnkCell : UnsToSetBlk)
			{
				//cout << "HELLO MARKING " << *UnkCell << endl;
			
				//Mark(UnkCell, State::Black);
				mark_as_black.insert(UnkCell);
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



	if (Process(Verbose, mark_as_black, mark_as_white, "PROCESS SetStateOfAllUnknownNeighborsToCellsInARegion."))
	{
		return true;// m_sitrep;
	}
	else
	{
		return false;
	}
}

// TODO: This function turned out to be garbage.
// UPDATE: Found another issue with expanding black cells that makes me want to rework this function even more.
// - I didn't consider the fact that the neighbor function we were testing could have already had a connecting path.
// - At this point it only covers the logic for black cells that didn't already have a path like a corner square etc..
// - instead of rewriting the function I am thinking I will just add another function that covers the other logic....
//void Grid::SolveBlackHasToConnect()
//{
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
//}

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

bool Grid::SolveUpdateCompleteIslands(bool Verbose)
{
	//cout << "SolveUpdateCompleteIslands" << endl;

	auto NumberedRegions = GetAllNumberedRegions();
	auto res = UpdateCompleteRegions(NumberedRegions, Verbose);

	if (res)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Grid::SolveCellsWithTwoAdjacentNumberedCells(bool Verbose)
{
	std::set<Cell*> mark_as_white;
	std::set<Cell*> mark_as_black;



	//cout << "SolveCellsWithTwoAdjacentNumberedCells" << endl;
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
				//Mark(TestedCell, State::Black);
				mark_as_black.insert(TestedCell);
			}
		}
	}



	if (Process(Verbose, mark_as_black, mark_as_white, "SolveCellsWithTwoAdjacentNumberedCells."))
	{
		return true;// m_sitrep;
	}
	else
	{
		return false;
	}
}







void Grid::Mark(Cell* InCell, const State NewState)
{
	//if (InCell && InCell->GetPosition() == Coordinate2D(4, 0))
	//{
	//	cout << *InCell << endl;
	//}

	//if (InCell && InCell->GetPosition() == Coordinate2D(3, 0))
	//{
	//	cout << *InCell << endl;
	//}

	if (NewState != State::Black && NewState != State::White)
	{
		throw std::logic_error("Error InCell is not black or white");
	}



	//if (InCell->GetState() != State::Unknown)
	//{
	//	// TODO: This would be where I would return with contradiction found or something like that to our "solve" while loop
	//	//std::cout << InCell->GetPosition() << endl;
	//	//return;
	//	//cout << *InCell->GetRegion() << endl;
	//	throw std::logic_error("Trying to mark a cell black that is not set to unknown " + std::to_string(InCell->GetPosition().GetX()) + 
	//		+ ", " + std::to_string(InCell->GetPosition().GetY()));
	//}


	//	// If we're asked to mark an already known cell, we've encountered a contradiction.
	//	// Remember this, so that solve() can report the contradiction.
	//
	if (InCell->GetState() != State::Unknown) 
	{
		m_sitrep = SolveStatus::CONTRADICTION_FOUND;
		return;
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
		//std::cout << "Contradiction FOUND" << std::endl;
		m_sitrep = SolveStatus::CONTRADICTION_FOUND;
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