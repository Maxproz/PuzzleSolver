#include "Region.h"

#include "Cell.h"



Region::Region(Cell* InCell, const std::set<Cell*>& InUnknowns)
{
	m_RegionState = InCell->GetState();
	m_Unknowns = InUnknowns;

	m_Cells.insert(InCell);
}
