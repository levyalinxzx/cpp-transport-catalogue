#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace transport_catalogue{
struct Stop
{
    std::string name;
    geo::Coordinates coordinates_;
};


struct Bus {
    std::string id;
    std::vector<const Stop*> stops;	
    bool is_circle;
};

struct BusInfo {
    size_t size;
    int unique_stops;
    int length_route;
    double curvature;
};
}
