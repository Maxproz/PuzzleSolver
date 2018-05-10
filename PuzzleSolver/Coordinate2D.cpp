
#include "Coordinate2D.h"

#include <iostream>

// #include <utility> (not used anymore)

bool Coordinate2D::Equals(Coordinate2D c) const
{
	return (m_X == c.m_X && m_Y == c.m_Y);
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
	os << "(" << rhs.m_X << "," << rhs.m_Y << ")";
	return os;
}