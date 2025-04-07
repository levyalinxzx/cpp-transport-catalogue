#pragma once

#include "json.h"
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

    void FillCatalogue(transport_catalogue::TransportCatalogue& catalogue);
    renderer::MapRenderer FillRenderSettings(const json::Dict& request_map) const;

    const json::Node PrintBus(const json::Dict& request_map, RequestHandler& rh) const;
    const json::Node PrintStop(const json::Dict& request_map, RequestHandler& rh) const;
    const json::Node PrintMap(const json::Dict& request_map, RequestHandler& rh) const;

    void ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const;

private:
    json::Document input_;
    json::Node dummy_ = nullptr;

    const transport_catalogue::Stop FillStop(const json::Dict& request_map) const;
    void FillStopDistances(transport_catalogue::TransportCatalogue& catalogue, const json::Array& stops_requests) const;
    const transport_catalogue::Bus FillRoute(const json::Dict& request_map, transport_catalogue::TransportCatalogue& catalogue) const;
    const std::tuple<json::Array, json::Array> SortedRequests(const json::Array& base_request) const;
    void AddStops(const json::Array& stops_requests, transport_catalogue::TransportCatalogue& catalogue);
    void AddBuses(const json::Array& buses_requests, transport_catalogue::TransportCatalogue& catalogue);
    std::vector<svg::Color> ReadColors(const json::Array &json) const ;
    svg::Color ReadColor(const json::Node &json) const ;
};
