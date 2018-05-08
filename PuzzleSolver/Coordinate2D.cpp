
#include "Coordinate2D.h"

#include <iostream>


bool Coordinate2D::Equals(Coordinate2D c) const
{
	return (m_x == c.m_x && m_y == c.m_y);
}

bool operator==(const Coordinate2D & lhs, const Coordinate2D & rhs)
{
	return lhs.Equals(rhs);
}

bool operator!=(const Coordinate2D & lhs, const Coordinate2D & rhs)
{
	return !(lhs.Equals(rhs));
}

std::ostream & operator<<(std::ostream & os, const Coordinate2D & rhs)
{
	// TODO: insert return statement here
	os << rhs.m_x << "," << rhs.m_y;
	return os;
}
