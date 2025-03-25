#pragma once

#include <string>
#include <deque>
#include <vector>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <utility>

#include "geo.h"

namespace transport_catalogue {
struct Stop
{
	std::string name;
	detail::geo::Coordinates coordinates_;
};


struct Bus {
	std::string id;
	std::vector<const Stop*> stops;	
};

struct Distance{
    const Stop* A;
    const Stop* B;
    int length;
};


struct BusInfo {
    size_t size;
    int unique_stops;
    size_t length_route;
    double curvature;
};

struct DistanceHasher {
    std::hash<const void*> hasher;
    
    std::size_t operator()(const std::pair<const Stop*, const Stop*> pair_stops) const noexcept {
        auto hash_1 = static_cast<const void*>(pair_stops.first);
        auto hash_2 = static_cast<const void*>(pair_stops.second);
        return hasher(hash_1) *  24 + hasher(hash_2);
    }  
};

class TransportCatalogue {
public:
    void AddStop(const Stop& stop);
    void AddBus(const Bus& bus);
    void AddDistance(const std::vector<Distance>& distances);
    const Stop* GetStop(std::string_view stop) const;
    const Bus* GetBus(std::string_view bus) const ;
    BusInfo GetBusInfo(const Bus* bus) const;
    const std::unordered_set<const Bus*>& GetBusesToStop(const Stop* stop) const; 
    size_t GetDistanceToBus(const Bus* bus) const;
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<std::string_view, std::unordered_set<const Bus*>> stopname_to_buses_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher> distance_to_stop;

    size_t GetNumberOfStops(const Bus* bus) const;
    int GetUniqueStops(const Bus* bus) const;
    double GetLengthRoute(const Bus* bus) const;
    size_t GetDistance(const Stop* first, const Stop* second) const;
};
}//namespace transport_catalogue
