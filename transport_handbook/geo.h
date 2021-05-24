#pragma once

#include <cmath>
#include <functional>

namespace Handbook {
    namespace Utilities {

        struct Coordinates {
            double lat;
            double lng;
        };

        bool operator==(const Coordinates &lhs, const Coordinates &rhs);

        bool operator!=(const Coordinates &lhs, const Coordinates &rhs);

        double ComputeDistance(Coordinates from, Coordinates to);

        struct CoordinatesHash {
            size_t operator()(const Coordinates &point) const {
                return d_hasher_(point.lat) + d_hasher_(point.lng) * 47;
            }

        private:
            std::hash<double> d_hasher_;
        };

    } // namespace Utilities

} // namespace Handbook
