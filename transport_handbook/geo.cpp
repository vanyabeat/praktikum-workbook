#include "geo.h"
#include <cmath>

namespace {
    static bool AreSame(double a, double b) {
        return std::fabs(a - b) < std::numeric_limits<double>::epsilon();
    }
} // namespace

bool Handbook::Utilities::operator==(const Handbook::Utilities::Coordinates &lhs,
                                     const Handbook::Utilities::Coordinates &rhs) {
    return (&lhs == &rhs) || (::AreSame(lhs.lat, rhs.lat) && ::AreSame(lhs.lng, rhs.lng));
}

double
Handbook::Utilities::ComputeDistance(Handbook::Utilities::Coordinates from, Handbook::Utilities::Coordinates to) {
    using namespace std;
    const double dr = M_PI / 180.0;
    return acos(sin(from.lat * dr) * sin(to.lat * dr) +
                cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr)) *
           6371000;
}

bool Handbook::Utilities::operator!=(const Handbook::Utilities::Coordinates &lhs,
                                     const Handbook::Utilities::Coordinates &rhs) {
    return !(lhs == rhs);
}
