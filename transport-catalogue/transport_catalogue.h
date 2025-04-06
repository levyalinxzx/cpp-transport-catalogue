#pragma once

#include <string>
#include <deque>
#include <vector>
#include <map>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <utility>

#include "geo.h"
#include "domain.h"

namespace transport_catalogue {

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
    void SetDistance(const Stop* first, const Stop* second, int distance);
    const Stop* GetStop(std::string_view stop) const;
    const Bus* GetBus(std::string_view bus) const ; 
    std::vector<std::string> GetSortedBusesToStop(const Stop* stop) const ;
    const std::map<std::string_view, const Bus*> GetSortedAllBuses() const;
    size_t GetNumberOfStops(const Bus* bus) const;
    int GetUniqueStops(const Bus* bus) const;
    double GetLengthRoute(const Bus* bus) const;
    int GetDistanceToBus(const Bus* bus) const;
    const std::unordered_set<const Bus*>& GetBusesToStop(const Stop* stop) const;
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<std::string_view, std::unordered_set<const Bus*>> stopname_to_buses_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher> distance_to_stop;

    int GetDistance(const Stop* first, const Stop* second) const;
};
}//namespace transport_catalogue
}//namespace transport_catalogue
