
#include "Cell.h"

#include "Region.h"

Region* Cell::GetRegion() const
{
	return m_Region;//  != nullptr ? m_Region : throw std::exception("Tried to get a null region");
}

void Cell::SetRegion(Region * NewRegion)
{
	 m_Region = NewRegion; 
}
