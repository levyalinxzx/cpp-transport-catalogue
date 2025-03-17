#include "transport_catalogue.h"

#include <algorithm>
#include <execution>
#include <iostream>

namespace transport_catalogue{
void TransportCatalogue::AddStop(stop_detail::Stop&& stop) {

    stops_to_buses.insert(std::move(stop));

}
void TransportCatalogue::AddBus(bus_detail::Bus&& bus) {
    
    buses_.push_back(std::move(bus));
}
const stop_detail::Stop* TransportCatalogue::GetStop(std::string_view stop)const {
    for(const auto& stop_ : stops_to_buses) {
        if (stop_.name == stop) {
            return &stop_;
        }
    }
    return nullptr;
}

const bus_detail::Bus* TransportCatalogue::GetBus(std::string_view bus) const {
    for(const auto& bus_ : buses_) {
        if (bus_.id == bus) {
            return &bus_;
        }
    }
    return nullptr;
}

int TransportCatalogue::GetUniqeStops(const bus_detail::Bus* bus) const {
    std::unordered_set<const stop_detail::Stop*, bus_detail::Bus__hash> uniqe_stops;
    uniqe_stops.insert(bus->stops.begin(), bus->stops.end());
    return int(uniqe_stops.size());
}

double TransportCatalogue::GetLongRoute(const bus_detail::Bus* bus) const { 
    return transform_reduce(next(bus->stops.begin()),bus->stops.end(), bus->stops.begin(), 0.0, std::plus<>{}, [](const stop_detail::Stop* lhs, const stop_detail::Stop* rhs) { 
        return detail::geo::ComputeDistance((*lhs).coordinates_, (*rhs).coordinates_);
                            });
}

std::unordered_set<std::string> TransportCatalogue::GetBusesToStop(const stop_detail::Stop* stop) const {
    std::unordered_set<std::string> result;
    for (const auto& bus : buses_) {
        const auto& route_bus = bus.stops;
        const auto it = std::find(route_bus.cbegin(), route_bus.cend(), stop);
        if (it == route_bus.cend()) {
            continue;
        }
        result.insert(bus.id);
    }
    return result;
}
}//namespace tranport_catalogue
