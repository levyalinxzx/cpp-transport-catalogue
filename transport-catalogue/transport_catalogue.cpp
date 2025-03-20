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
        if (!stopname_to_buses_.count(stop->name)) {
            stopname_to_buses_.insert({stop->name, {}});
        }
        stopname_to_buses_.at(stop->name).insert(&buses_.back());
    }
}	
const Stop* TransportCatalogue::GetStop(std::string_view stop)const {
    return stopname_to_stop_.count(stop) ? stopname_to_stop_.at(stop) : nullptr;
}

const Bus* TransportCatalogue::GetBus(std::string_view bus) const {
    return busname_to_bus_.count(bus) ? busname_to_bus_.at(bus) : nullptr;
}

BusInfo TransportCatalogue::GetBusInfo(const Bus* bus) const {
    int unique_stops = GetUniqueStops(bus);
    double length_route = GetLengthRoute(bus);
    return {unique_stops, length_route};
}

std::unordered_set<const Bus*> TransportCatalogue::GetBusesToStop(const Stop* stop) const {
    if (stopname_to_buses_.count(stop->name)) {
        return stopname_to_buses_.at(stop->name);
    }
    return {};
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
