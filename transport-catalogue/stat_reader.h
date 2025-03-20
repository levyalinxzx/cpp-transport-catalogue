#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"
namespace transport_catalogue{
namespace detail {
void PrintBus(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output);
void PrintStop(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output);
void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output);


}//namespace detail
}//namespace transport_catalogue
