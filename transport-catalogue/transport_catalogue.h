#pragma once

#include <string>
#include <deque>
#include <vector>
#include <unordered_set>
#include <set>
#include <unordered_map>

#include "geo.h"

namespace transport_catalogue {
namespace stop_detail{
struct Stop
{
	std::string name;
	detail::geo::Coordinates coordinates_;
    //std::deque<const Bus*> buses;

    auto tuple() const {
        return tie(name, coordinates_);
    }
 
    bool operator==(const Stop& other) const {
        return tuple() == other.tuple();
    }

	size_t hash() const {
        std::hash<double> hasher;
        auto hasher_latitude = hasher(coordinates_.lat);
        auto hasher_longitude = hasher(coordinates_.lng);
        return size_t(hasher_latitude * 17 + hasher_longitude);
    }  
};
}//namespace stop_detail
namespace bus_detail {
struct Bus {
	std::string id;
	std::deque<const stop_detail::Stop*> stops;	
};


struct Bus_hash {
    size_t operator()(const stop_detail::Stop& stop) const {
        return size_t(stop.hash());
    }
};
 
struct Bus__hash {
    size_t operator()(const stop_detail::Stop* stop) const {
        return size_t((*stop).hash());
    }
};
}//bus_detail

class TransportCatalogue {
	public:
	    void AddStop(stop_detail::Stop&& stop);
	    void AddBus(bus_detail::Bus&& bus);
        const stop_detail::Stop* GetStop(std::string_view stop) const;
        const bus_detail::Bus* GetBus(std::string_view bus) const ;
		int GetUniqeStops(const bus_detail::Bus* bus) const;
        double GetLongRoute(const bus_detail::Bus* bus) const;
        std::unordered_set<std::string> GetBusesToStop(const stop_detail::Stop* stop) const; 
	private:
        std::unordered_set<stop_detail::Stop, bus_detail::Bus_hash> stops_to_buses;
		std::deque<bus_detail::Bus> buses_;
};
}//namespace transport_catalogue
