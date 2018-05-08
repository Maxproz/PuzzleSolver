#pragma once


#ifndef Coordinate2D_H

#define Coordinate2D_H

#include <iosfwd>

class ostream;

class Coordinate2D
{
private:

	bool Equals(Coordinate2D c) const;

public:
	Coordinate2D() = delete;
	constexpr Coordinate2D(double x, double y) : m_X{ x }, m_Y{ y } {}
	~Coordinate2D() = default;

	friend bool operator==(const Coordinate2D& lhs, const Coordinate2D& rhs);
	friend bool operator!=(const Coordinate2D& lhs, const Coordinate2D& rhs);
	friend std::ostream& operator<<(std::ostream& os, const Coordinate2D& rhs);


private:
	int m_X{ 0 };
	int m_Y{ 0 };
};

bool operator==(const Coordinate2D& lhs, const Coordinate2D& rhs);
bool operator!=(const Coordinate2D& lhs, const Coordinate2D& rhs);
std::ostream& operator<<(std::ostream& os, const Coordinate2D& rhs);


#endif // !Coordinate2D_H






