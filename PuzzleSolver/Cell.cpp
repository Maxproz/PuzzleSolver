
#include "Cell.h"

#include "Region.h"

#include <iostream>

Region* Cell::GetRegion() const
{
	return m_Region;//  != nullptr ? m_Region : throw std::exception("Tried to get a null region");
}

void Cell::SetRegion(Region * NewRegion)
{
	 m_Region = NewRegion; 
}

std::ostream & operator<<(std::ostream & os, const Cell & RHS)
{

	os << RHS.GetPosition() << ", " << "State: " << static_cast<int>(RHS.GetState());
	return os;
}
