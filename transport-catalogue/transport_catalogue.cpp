#include "transport_catalogue.h"

#include <algorithm>
#include <execution>
#include <iostream>

namespace transport_catalogue {
void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.push_back(stop);
    stopname_to_stop_.insert({stops_.back().name, &stops_.back()});
}
void TransportCatalogue::AddBus(const Bus& bus) {
    buses_.push_back(bus);
    busname_to_bus_.insert({buses_.back().id, &buses_.back()});
    for (const auto& stop : buses_.back().stops) {
        stopname_to_buses_[stop->name].insert(&buses_.back());
    }
}	
const Stop* TransportCatalogue::GetStop(std::string_view stop)const {
    auto it = stopname_to_stop_.find(stop);
    if(it == stopname_to_stop_.end()) {
        return nullptr;
    }
    return it->second;    
}

const Bus* TransportCatalogue::GetBus(std::string_view bus) const {
    auto it = busname_to_bus_.find(bus);
    if(it == busname_to_bus_.end()) {
        return nullptr;
    }
    return it->second;    
}

BusInfo TransportCatalogue::GetBusInfo(const Bus* bus) const {
    size_t size = GetNumberOfStops(bus);
    int unique_stops = GetUniqueStops(bus);
    double length_route = GetLengthRoute(bus);
    return {size, unique_stops, length_route};
}

std::unordered_set<const Bus*> TransportCatalogue::GetBusesToStop(const Stop* stop) const {
   static std::unordered_set<const Bus*> empty_set = {};
   auto it = stopname_to_buses_.find(stop->name);
   if (it == stopname_to_buses_.end()) {
       return empty_set;
   }
   return it->second;
}

size_t TransportCatalogue::GetNumberOfStops(const Bus* bus) const {
    return bus->stops.size();
}

int TransportCatalogue::GetUniqueStops(const Bus* bus) const {
    std::unordered_set<const Stop*> unique_stops;
    unique_stops.insert(bus->stops.begin(), bus->stops.end());
    return int(unique_stops.size());
}

double TransportCatalogue::GetLengthRoute(const Bus* bus) const { 
    return transform_reduce(next(bus->stops.begin()),bus->stops.end(), bus->stops.begin(), 0.0, std::plus<>{}, [](const Stop* lhs, const Stop* rhs) { 
        return detail::geo::ComputeDistance((*lhs).coordinates_, (*rhs).coordinates_);
                            });
}


}//namespace tranport_catalogue
