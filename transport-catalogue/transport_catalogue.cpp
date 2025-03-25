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

void TransportCatalogue::AddDistance(const std::vector<Distance>& distances) {
    for (const auto& distance : distances) {
        auto pair_distance = std::make_pair(distance.A, distance.B);
        distance_to_stop.insert({pair_distance, distance.length});
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
    double curvature = GetDistanceToBus(bus)/GetLengthRoute(bus);
    size_t length_route = GetDistanceToBus(bus);
    return {size, unique_stops, length_route, curvature};
}

std::unordered_set<const Bus*> TransportCatalogue::GetBusesToStop(const Stop* stop) const {
   static std::unordered_set<const Bus*> empty_set = {};
   auto it = stopname_to_buses_.find(stop->name);
   if (it == stopname_to_buses_.end()) {
       return empty_set;
   }
   return it->second;
}

size_t TransportCatalogue::GetDistanceToBus(const Bus* bus) const {
    size_t distance = 0;
    auto stops_size = bus->stops.size() - 1;
    for (int i = 0; i < stops_size; i++) {
        distance += GetDistance(bus->stops[i], bus->stops[i+1]);
    }
    return distance;
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


size_t TransportCatalogue::GetDistance(const Stop* first, const Stop* second) const {
    try {
        auto distance_pair = std::make_pair(first, second);
        return distance_to_stop.at(distance_pair);
    } catch(const std::out_of_range &e) {
        try{
            auto distance_pair = std::make_pair(second, first);
            return distance_to_stop.at(distance_pair);
        }
        catch(const std::out_of_range &e) {
            return 0;
        }
    }
}

}//namespace tranport_catalogue

