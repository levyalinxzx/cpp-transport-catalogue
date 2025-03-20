#pragma once

#include <string>
#include <deque>
#include <vector>
#include <unordered_set>
#include <set>
#include <unordered_map>

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


struct BusInfo {
    int unique_stops;
    double length_route;
};


class TransportCatalogue {
public:
    void AddStop(const Stop& stop);
    void AddBus(const Bus& bus);
    const Stop* GetStop(std::string_view stop) const;
    const Bus* GetBus(std::string_view bus) const ;
    BusInfo GetBusInfo(const Bus* bus) const;
    std::unordered_set<const Bus*> GetBusesToStop(const Stop* stop) const; 
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<std::string_view, std::unordered_set<const Bus*>> stopname_to_buses_;

    int GetUniqueStops(const Bus* bus) const;
    double GetLengthRoute(const Bus* bus) const;
};
}//namespace transport_catalogue
