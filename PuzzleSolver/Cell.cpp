
#include "Cell.h"

#include "Region.h"

#include <iostream>
#include <string>


Region* Cell::GetRegion() const
{
	/*if (!m_Region)
		return nullptr;*/
	return m_Region;//  != nullptr ? m_Region : throw std::exception("Tried to get a null region");
}

void Cell::SetRegion(Region * NewRegion)
{
	 m_Region = NewRegion; 
}

std::ostream & operator<<(std::ostream & os, const Cell & RHS)
{

	auto DisplayState = static_cast<int>(RHS.GetState());

	std::string DisplayString;

	if (DisplayState == -3)
	{
		DisplayString = "UNKNOWN";
	}
	else if (DisplayState == -2)
	{
		DisplayString = "WHITE";
	}
	else if (DisplayState == -1)
	{
		DisplayString = "BLACK";
	}
	else
	{
		DisplayString = std::to_string(DisplayState);
	}


	os << RHS.GetPosition() << ", " << "State: " << DisplayString << "\n";
	return os;
}


bool operator==(const Cell& lhs, const Cell& rhs)
{
	if (lhs.m_GridPosition.GetX() == rhs.m_GridPosition.GetX() && lhs.m_GridPosition.GetY() == rhs.m_GridPosition.GetY())
	{
		return true;
	}

	return false;
}

bool operator!=(const Cell& lhs, const Cell& rhs)
{
	return !(lhs == rhs);
}