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

const tc::Buses* RequestHandler::GetBusesToStop(std::string_view stop_name) const {
    const tc::Stop* stop = catalogue_.GetStop(stop_name);

    return stop ? &(catalogue_.GetBusesToStop(stop)) : nullptr;;
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetSVG(catalogue_.GetSortedAllBuses());
}

std::optional<tc::router::RouteInfo> RequestHandler::FindRoute(std::string_view stop_name_from,
  std::string_view stop_name_to) const {
  const tc::Stop *from = catalogue_.GetStop(stop_name_from);
  const tc::Stop *to = catalogue_.GetStop(stop_name_to);

  if (from != nullptr && to != nullptr) {
    return router_.FindRoute(from, to);
  } else {
    return std::nullopt;
  }
}

const tc::BusInfo RequestHandler::GetBusStat(std::string_view bus_name) const {
    const tc::BusInfo bus_info = catalogue_.GetBusInfo(bus_name);
    return bus_info;
}
 


