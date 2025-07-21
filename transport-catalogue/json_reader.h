#pragma once

//#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <iostream>

class JsonReader {
public:
    JsonReader(std::istream& input)
        : input_(json::Load(input))
    {}

    const json::Node& GetBaseRequests() const;
    const json::Node& GetStatRequests() const;
    const json::Node& GetRenderSettings() const;
    const json::Node& GetRoutingSettings() const;

    void FillCatalogue(tc::TransportCatalogue& catalogue);
    renderer::MapRenderer FillRenderSettings(const json::Dict& request_map) const;
    tc::router::RoutingSettings FillRoutingSettings(const json::Node& settings) const;

    void ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const;

private:
    json::Document input_;
    json::Node dummy_ = nullptr;

    const tc::Stop FillStop(const json::Dict& request_map) const;
    void FillStopDistances(tc::TransportCatalogue& catalogue, const json::Array& stops_requests) const;
    const tc::Bus FillRoute(const json::Dict& request_map, tc::TransportCatalogue& catalogue) const;
    const std::tuple<json::Array, json::Array> SortedRequests(const json::Array& base_request) const;
    void AddStops(const json::Array& stops_requests, tc::TransportCatalogue& catalogue);
    void AddBuses(const json::Array& buses_requests, tc::TransportCatalogue& catalogue);
    std::vector<svg::Color> ReadColors(const json::Array &json) const ;
    svg::Color ReadColor(const json::Node &json) const ;
    void PrintBus(json::Builder& builder, const json::Dict& request_map, RequestHandler& rh) const;
    void PrintStop(json::Builder& builder, const json::Dict& request_map, RequestHandler& rh) const;
    void PrintMap(json::Builder& builder, RequestHandler& rh) const;
    void PrintRoute(json::Builder& builder, const json::Dict& request_map, RequestHandler& rh) const;
    void BuildRouteItem(json::Builder& builder, const tc::router::RouteInfo::WaitItem& item) const;
    void BuildRouteItem(json::Builder& builder, const tc::router::RouteInfo::BusItem& item) const;
    
};
