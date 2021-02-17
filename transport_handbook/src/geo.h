#pragma once

#include <cmath>

struct Coordinates
{
	double lat;
	double lng;
	friend bool operator==(const Coordinates& l, const Coordinates& r)
	{
		return AreSame(l.lat, r.lat) && AreSame(r.lng, r.lng);
	}

	static bool AreSame(double a, double b)
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