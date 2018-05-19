#pragma once



#ifndef Grid_H

#define Grid_H


#include <vector> // Used as a data member
#include <memory> // Used as a data member
#include <random> // Used as a data member
#include <set>
#include <map>

#include "Region.h" // would need to use pimpl to remove header


class Coordinate2D; // need name for function parameters

// Grid is a composition of the parts...
// Part 1: A 2D array of Cell pointers (lets try unique then shared if it doesnt work)
// Part 2: An container of pointers to regions (unique_ptr for now) that control the logic of connecting cells (numbered, whites) 
//				- Regions should have containers of the unknown cells that they are surrounded by.
//				- Regions should have containers of the Cells that are apart of that region

// TODO: Each region should keep track of its state, white, black or numbered.
// TODO: This will be seperate from a "Cells" state of being (white, black, unknown, numbered)
// So each Cell will know if its white, black, numbered, or unknown.
// And each Region will hold raw pointers to those cells (maybe convert to shared_ptr later) and the entire region will have its own state.
// - This is how we will know if white cells are connected to numbered cells.
// - Each region is black, white, or numbered. This allows us to
// - remember when white cells are connected to numbered cells,
// - as the whole region is marked as numbered.
// - Each region keeps track of the coordinates that it occupies.
// - Each region also keeps track of the unknown cells that it's surrounded by.

// TODO TEST: After Marking a cell black or white first create a new region using it.
// TODO TEST: After marking a cell (black or white), we should iterate all of its valid neighbors and attempt to fuse regions

enum class SolveStatus;

class Grid
{
public:
	// TODO: Think about which type of operations I will need here

	// Grid should be constructed with a given width, height, and set of Coordinates with a number for the island(numbered cell) locations.
	Grid() = delete;
	Grid(int Width, int Height, std::vector<std::pair<Coordinate2D, int>> NumberedCellLocations);
	~Grid() = default; // using smart pointers, default destructor is fine.


	// TODO: How the hell do I get the deep copy with 2D arrays working.
	//Grid(const Grid& Copy) = delete;
	Grid(const Grid& Copy);
	//Grid& operator=(const Grid& CopyAssign) = delete;
	Grid& operator=(const Grid& CopyAssign);

	Grid(Grid&& Move);
	Grid& operator=(Grid&& MoveAssign); 



	void PrintGrid() const;
	void PrintAllCellsInAllRegions() const;
	void PrintAllUnknownsInAllRegions() const;

	// Where most of the logic will happen
	SolveStatus SolvePuzzle(bool Verbose = true, bool Guessing = true);//	SitRep solve(););

private:

	// Helper functions that were created for solving step 2 (can be reused if needed in other spots later)
	std::set<Region*> GetAllNumberedRegions() const;
	bool UpdateCompleteRegions(std::set<Region*>& InNumberedRegions, bool Verbose);
	bool SetStateOfAllUnknownNeighborsToCellsInARegion(Region* InRegion, const State& InState, bool Verbose);

	// TODO: Figure out a better way to name these solving steps...
	//void SolveBlackHasToConnect();
	bool BlackCellHasAtLeastOnePath(Cell* InBlackCell);

	bool SolveUpdateCompleteIslands(bool Verbose);
	bool SolveCellsWithTwoAdjacentNumberedCells(bool Verbose);
	
	bool SolveStepFourUnreachableCells(bool Verbose);
	bool Unreachable(Cell* InCell, std::set<Cell*> discovered = std::set<Cell*>());
	bool Impossibly_big_white_region(const int N) const;

	bool SolveExpandPartialNumberedRegionsWithOnePath(bool Verbose);

	bool SolveCheckFor2x2Pools(bool Verbose);

	bool SolveStepFiveNSizeTwoChoices(bool Verbose);

	// Added a bunch a logic that really hasn't been useful yet here.
	//bool DoesMarkBlackCreateAPool(Cell* InCell);

	bool SolvePartialWhiteRegionsWithOnlyOnePath(bool Verbose);
	bool SolvePartialBlackRegionsWithOnlyOnePath(bool Verbose);

	//void SolveDoesWhiteExpandMakeUnconnectableRegion();
	bool SolvePreventPoolsTwoBlackTwoUnknown(bool Verbose);

	bool SolveConfinementAnalysis(bool Verbose);
	bool Confined(Region* r,
		std::map<Region*, std::set<Cell*>>& cache,
		const std::set<Cell*>& forbidden = std::set<Cell*>());
	
	std::string DetectContradictions(std::map<Region*, std::set<Cell*>>& Cache, bool Verbose);
	
	
	bool SolveIsolatedUnknownCells(bool Verbose);


	SolveStatus SolveGuessingRemaining(bool IsVerbose = true, bool IsGuessing = true);

	// mostly unused function
	bool Process(const bool verbose, const std::set<Cell*>& mark_as_black,
		const std::set<Cell*>& mark_as_white, const std::string& s);

	int Known() const;

	void Mark(Cell* InCell, const State NewState);

	void AddRegion(Cell* InCell);
	void FuseRegions(Region* LHSRegion, Region* RHSRegion);

	void swap(Grid& other);

	Cell* operator()(const Coordinate2D& Pos);
	const Cell* operator()(const Coordinate2D& Pos) const;
	

	// A cell is valid if it's a valid index on the board so x is between [0, width) y is between [0, height)
	// TODO: Is having 2 versions here really necessary
	bool IsValid(Coordinate2D Cord) const;
	bool IsValid(Cell* InCell) const;


	template <typename Functor>
	void For_All_Valid_Neighbors(Cell* InCell, Functor Func)
	{
		// You were given a cell check all of its neighbors and if its a valid cell on the board
		// - (not outside of the grid) run the functor
		auto InX = InCell->GetPosition().GetX();
		auto InY = InCell->GetPosition().GetY();

		// When checking valid neighboring cells lets start at the top then go clockwise 
		auto TopCell = operator()(Coordinate2D(InX, InY - 1));

		if (IsValid(TopCell))
		{
			Func(TopCell);
		}

		auto RightCell = operator()(Coordinate2D(InX + 1, InY));

		if (IsValid(RightCell))
		{
			Func(RightCell);
		}

		auto BottomCell = operator()(Coordinate2D(InX, InY + 1));

		if (IsValid(BottomCell))
		{
			Func(BottomCell);
		}

		auto LeftCell = operator()(Coordinate2D(InX - 1, InY));

		if (IsValid(LeftCell))
		{
			Func(LeftCell);
		}
	}

	template <typename Functor>
	void For_All_Valid_Unknown_Neighbors(Cell* InCell, Functor Func)
	{
		// You were given a cell check all of its neighbors and if its a valid cell on the board
		// - (not outside of the grid) run the functor
		auto InX = InCell->GetPosition().GetX();
		auto InY = InCell->GetPosition().GetY();

		// When checking valid neighboring cells lets start at the top then go clockwise 
		auto TopCell = operator()(Coordinate2D(InX, InY - 1));

		if (IsValid(TopCell))
		{
			if (TopCell->GetState() == State::Unknown)
			{
				Func(TopCell);
			}
			else
			{
				//return;
			}
		}

		auto RightCell = operator()(Coordinate2D(InX + 1, InY));

		if (IsValid(RightCell))
		{
			if (RightCell->GetState() == State::Unknown)
			{
				Func(RightCell);
			}
			else
			{
				//return;
			}
		}


		auto BottomCell = operator()(Coordinate2D(InX, InY + 1));

		if (IsValid(BottomCell))
		{
			if (BottomCell->GetState() == State::Unknown)
			{
				Func(BottomCell);
			}
			else
			{
				//return;
			}
		}

		auto LeftCell = operator()(Coordinate2D(InX - 1, InY));

		if (IsValid(LeftCell))
		{
			if (LeftCell->GetState() == State::Unknown)
			{
				Func(LeftCell);
			}
			else
			{
				//return;
			}
		}

	}

	

private:
	int m_Width{ 0 };
	int m_Height{ 0 };
	
	int m_total_black{ 0 };

	std::map<Region*, std::set<Cell*>> m_Cache;

	// This is initially KEEP_GOING.
	// If an attempt is made to fuse two numbered regions, or to mark an already known cell,
	// this is set to CONTRADICTION_FOUND.
	SolveStatus m_sitrep;
	
	// This is used to guess cells in a deterministic but pseudorandomized order.
	std::mt19937 m_prng;


	// TODO: m_cells should be allocated in the constructor of grid.
	std::vector<std::vector<std::unique_ptr<Cell > > > m_Cells;

	// The set of all regions ~ created inside of AddRegion probably.
	// Set is preventing having multiple pointers to the same region in here.
	std::set<std::unique_ptr<Region> > m_Regions; 

};



#endif // !Grid_H