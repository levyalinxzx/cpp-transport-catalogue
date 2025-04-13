#include "request_handler.h"

#include <optional>
#include <algorithm>
#include <sstream>

bool RequestHandler::IsBusNumber(const std::string_view bus_number) const {
    return catalogue_.GetBus(bus_number);
}

bool RequestHandler::IsStopName(const std::string_view stop_name) const {
    return catalogue_.GetStop(stop_name);
}

std::vector<std::string> RequestHandler::GetSortedBusesToStop(std::string_view stop_name) const {
    const transport_catalogue::Stop* stop = catalogue_.GetStop(stop_name);
    std::vector<std::string> buses_vector;
        for (const auto& bus : catalogue_.GetBusesToStop(stop)) {
            buses_vector.push_back(bus->id);
        }
	    std::sort(buses_vector.begin(), buses_vector.end());
    return buses_vector;
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetSVG(catalogue_.GetSortedAllBuses());
}

const transport_catalogue::BusInfo RequestHandler::GetBusStat(std::string_view bus_name) const {
    const auto& bus_info = catalogue_.GetBusInfo(bus_name);
    return bus_info;
}
 


