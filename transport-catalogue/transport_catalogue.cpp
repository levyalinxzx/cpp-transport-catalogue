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

void TransportCatalogue::SetDistance(const Stop* first, const Stop* second, int distance) {
    auto pair_distance = std::make_pair(first, second);
    distance_to_stop.insert({pair_distance, distance});
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

const std::deque<Bus>& TransportCatalogue::GetSortedAllBuses() const {
    static std::deque<Bus> buses{buses_};
    std::sort(buses.begin(), buses.end(), [](const Bus& lhs, const Bus& rhs) {
        return lhs.id < rhs.id;
    });
    return buses;
}


int TransportCatalogue::GetDistanceToBus(const Bus* bus) const {
    int distance = 0;
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
        return geo::ComputeDistance((*lhs).coordinates_, (*rhs).coordinates_);
                            });
}

int TransportCatalogue::GetDistance(const Stop* first, const Stop* second) const {
        auto distance_pair = std::make_pair(first, second);
	auto search = distance_to_stop.find(distance_pair);
	if (search != distance_to_stop.end()) {
	    return search->second;
	}
        distance_pair = std::make_pair(second, first);
	search = distance_to_stop.find(distance_pair);
	if (search != distance_to_stop.end()) {
	    return search->second;
	}
        return 0;
}

const std::unordered_set<const Bus*>& TransportCatalogue::GetBusesToStop(const Stop* stop) const {
    static std::unordered_set<const Bus*> empty_set = {};
    auto it = stopname_to_buses_.find(stop->name);
    if (it == stopname_to_buses_.end()) {
        return empty_set;
    }
    return it->second;
}

}//namespace tranport_catalogue
