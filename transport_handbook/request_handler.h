#pragma once

#include "json.h"
#include "transport_router.h"
#include "transport_catalogue.h"
#include <string>

namespace Handbook {
    namespace Views {
        json::Document GetData(const json::Document &stat, const Handbook::Data::TransportCatalogue *t_q,
                               const transport::RouteFinder *r_f);
    } // namespace Views
} // namespace Handbook
