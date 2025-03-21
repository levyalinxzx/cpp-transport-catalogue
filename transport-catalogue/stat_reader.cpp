#include "stat_reader.h"

#include <iostream>
#include <algorithm>
#include <vector>
namespace transport_catalogue{
namespace detail { 

void PrintBus(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) { 
    const Bus* bus = transport_catalogue.GetBus(request); 
    if (bus != nullptr) {
        auto bus_info = transport_catalogue.GetBusInfo(bus);
        output << "Bus "  << bus->id << ": " << bus_info.size << " stops on route, "
        << bus_info.unique_stops << " unique stops, " 
        << bus_info.length_route << " route length" << std::endl;
    } else {
        output << "Bus " << request << ": not found" << std::endl;
    }
}

void PrintStop(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    const Stop* stop = transport_catalogue.GetStop(request);
    if(!stop) {
        output << "Stop " << request << ": not found" << std::endl;
    }else {
        const auto& buses = transport_catalogue.GetBusesToStop(stop);
        if(buses.empty()) {
            output << "Stop " << request << ": no buses" << std::endl;
        }else {
            std::vector<const Bus*> buses_vector(buses.begin(), buses.end());
            std::sort(buses_vector.begin(), buses_vector.end(), [](const Bus* lhs, const Bus* rhs) {
                return lhs->id < rhs->id;
            });
            output << "Stop " << request << ": buses";
            for (const auto& bus : buses_vector) {
                output << " " << bus->id;
            }
            output << std::endl;
        }
    }
}

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
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
        PrintBus(transport_catalogue, parse_request, output);
    }
    if (command == "Stop") {
        PrintStop(transport_catalogue, parse_request, output);
    }
}
}//namespace detail
}//namespace transport_catalogue
