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

namespace tc{


class TransportCatalogue {
public:
    void AddStop(const Stop& stop);
    void AddBus(const Bus& bus);
    void SetDistance(const Stop* first, const Stop* second, int distance);
    Stop* GetStop(std::string_view stop) const;
    Bus* GetBus(std::string_view bus) const; 
    const std::deque<Stop>& GetStops() const;
    const std::deque<Bus>& GetBuses() const;
    const std::deque<Bus>& GetSortedAllBuses() const;
    const BusInfo GetBusInfo(std::string_view bus_name) const;
    const Buses& GetBusesToStop(const Stop* stop) const;
    int GetDistance(const Stop* first, const Stop* second) const;
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<std::string_view, Buses> stopname_to_buses_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, Hasher> distance_to_stop;

    size_t GetNumberOfStops(const Bus* bus) const;
    size_t GetUniqueStops(const Bus* bus) const;
    double GetLengthRoute(const Bus* bus) const;
    size_t GetDistanceToBus(const Bus* bus) const;
};
}//namespace transport_catalogue
