#pragma once


#ifndef Coordinate2D_H

#define Coordinate2D_H

//#include <iosfwd>
//#include <utility> // std::pair (not used anymore)

//class ostream;

class Coordinate2D
{
private:

	bool Equals(Coordinate2D c) const;

public:
	Coordinate2D() = delete;
	Coordinate2D(int x, int y) : m_X{ x }, m_Y{ y } {}
	~Coordinate2D() = default;

	friend bool operator==(const Coordinate2D& lhs, const Coordinate2D& rhs);
	friend bool operator!=(const Coordinate2D& lhs, const Coordinate2D& rhs);
	//friend std::ostream& operator<<(std::ostream& os, const Coordinate2D& rhs);

	int GetX() const { return m_X; }
	int GetY() const { return m_Y; }

private:
	int m_X{ 0 };
	int m_Y{ 0 };
};

bool operator==(const Coordinate2D& lhs, const Coordinate2D& rhs);
bool operator!=(const Coordinate2D& lhs, const Coordinate2D& rhs);
//std::ostream& operator<<(std::ostream& os, const Coordinate2D& rhs);


#endif // !Coordinate2D_H






