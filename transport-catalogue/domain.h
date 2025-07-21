#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <set>

namespace tc{
struct Stop
{
    Stop() = default;

    Stop(std::string name, geo::Coordinates coordinates);

	std::string name_;
	geo::Coordinates coordinates_;
};

using Route = std::vector<Stop*>;

struct Bus {
    Bus() = default;

    Bus(std::string name_bus, Route stops, bool is_circle);

    bool operator<(Bus& other) ;

    std::string name_;
    Route stops_;
    bool is_circle_;	
};

struct BusPtrComparator {
    bool operator() (Bus* rhs, Bus* lhs) const ;
};

using Buses = std::set<Bus*, BusPtrComparator>;

struct BusInfo {
    size_t size;
    int unique_stops;
    int length_route;
    double curvature;
};

struct Hasher {
    std::hash<const void*> hasher;
    static const size_t salt = 24; 

    size_t operator()(const Stop *stop) const;
    
    std::size_t operator()(const std::pair<const Stop*, const Stop*> pair_stops) const noexcept;
    
};
}
