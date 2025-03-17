#include "stat_reader.h"

#include <iostream>
#include <algorithm>
#include <vector>
namespace transport_catalogue{
namespace detail { 
void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto start = request.find_first_not_of(' ');                    
    if (start == request.npos) {
        return;
    } 
    request = request.substr(start, request.find_last_not_of(' ') + 1);                    
    auto space = request.find(' ');
    std::string_view command = request.substr(start, space - start);
    request = request.substr(space);
    start = request.find_first_not_of(' ');
    std::string_view parse_request = request.substr(start);
    
    if(command == "Bus") {
        const bus_detail::Bus* bus = tansport_catalogue.GetBus(parse_request);
        if (bus != nullptr) {
            output << "Bus "  << parse_request << ": " << bus->stops.size() << " stops on route, " << tansport_catalogue.GetUniqeStops(bus)
            << " unique stops, " << tansport_catalogue.GetLongRoute(bus) << " route length" << std::endl;
        } else {
            output << "Bus " << parse_request << ": not found" << std::endl;
        }
    }
    if (command == "Stop") {
        const stop_detail::Stop* stop = tansport_catalogue.GetStop(parse_request);
        if(!stop) {
            output << "Stop " << parse_request << ": not found" << std::endl;
        }else {
            const auto& buses = tansport_catalogue.GetBusesToStop(stop);
            if(buses.empty()) {
                output << "Stop " << parse_request << ": no buses" << std::endl;
            }else {
                std::vector<std::string>buses_vector(buses.begin(), buses.end());
                std::sort(buses_vector.begin(), buses_vector.end());
                output << "Stop " << parse_request << ": buses";
                for (const auto& bus : buses) {
                    output << " " << bus;
                }
                output << std::endl;
            }

        }
    }
}
}//namespace detail
}//namespace transport_catalogue
