#pragma once

#include <cmath>

/// структура разместите в пространстве имен
struct Coordinates
{
	double lat;
	double lng;

	bool operator==(const Coordinates& r) const
	{
		return AreSame(lat, r.lat) && AreSame(lng, r.lng);
	}

	bool operator !=(const Coordinates& r) const
	{
		return !(*this == r);
	}

	bool AreSame(double a, double b) const
	{
		return std::fabs(a - b) < std::numeric_limits<double>::epsilon();
	}
};

inline double ComputeDistance(Coordinates from, Coordinates to)
{
	using namespace std;
	static const double dr = 3.1415926535 / 180.;
	return acos(sin(from.lat * dr) * sin(to.lat * dr) +
				cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr)) *
		   6371000;
}