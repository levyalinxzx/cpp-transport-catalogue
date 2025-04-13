/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include "json_reader.h"
#include "json_builder.h"

#include <sstream>
 
const json::Node& JsonReader::GetBaseRequests() const {
    const json::Dict& map = input_.GetRoot().AsDict();
    auto it = map.find("base_requests"); 
    if (it == map.end()) {
        return dummy_;
    }
    return it->second;
}
  
const json::Node& JsonReader::GetStatRequests() const {
    const auto& map = input_.GetRoot().AsDict();
    auto it = map.find("stat_requests"); 
    if (it == map.end()) {
        return dummy_;
    }
    return it->second;
}
 
const json::Node& JsonReader::GetRenderSettings() const {
    const auto& map = input_.GetRoot().AsDict();
    auto it = map.find("render_settings"); 
    if (it == map.end()) {
        return dummy_;
    }
    return it->second;
}
  
void JsonReader::FillCatalogue(transport_catalogue::TransportCatalogue& catalogue) {
    const json::Array& arr = GetBaseRequests().AsArray();
    const auto& [buses_requests, stops_requests] = SortedRequests(arr);
    AddStops(stops_requests, catalogue);
    FillStopDistances(catalogue, stops_requests);
    AddBuses(buses_requests, catalogue);
}
  
const transport_catalogue::Stop JsonReader::FillStop(const json::Dict& request_map) const {
    std::string stop_name = request_map.at("name").AsString();
    geo::Coordinates coordinates = { request_map.at("latitude").AsDouble(), request_map.at("longitude").AsDouble() };
    return {stop_name, coordinates};
}
  
void JsonReader::FillStopDistances(transport_catalogue::TransportCatalogue& catalogue, const json::Array& stops_requests) const {
    for (const auto& requests : stops_requests) {
        const auto& request_stops_map = requests.AsDict();{
            const transport_catalogue::Stop& stop = FillStop(request_stops_map);
            auto& distances = request_stops_map.at("road_distances").AsDict();
            for (auto& [to_name, dist] : distances) {
                auto from = catalogue.GetStop(stop.name);
                auto to = catalogue.GetStop(to_name);
                catalogue.SetDistance(from, to, dist.AsInt());
            }
        }
    }
}
  
const transport_catalogue::Bus JsonReader::FillRoute(const json::Dict& request_map, transport_catalogue::TransportCatalogue& catalogue) const {
    std::string bus_number = request_map.at("name").AsString();
    std::vector<const transport_catalogue::Stop*> stops;
    bool circular_route = request_map.at("is_roundtrip").AsBool();
    for (const auto& stop : request_map.at("stops").AsArray()) {
        stops.push_back(catalogue.GetStop(stop.AsString()));
    }
      if (!circular_route) {
        stops.insert(stops.end(), std::next(stops.rbegin()), stops.rend());
    }
    return {bus_number, stops, circular_route};
}
 
 
const std::tuple<json::Array, json::Array> JsonReader::SortedRequests(const json::Array& base_request) const {
    json::Array buses_requests;
    json::Array stops_requests;
    for(const auto& requests : base_request) {
        const auto& request_map = requests.AsDict();
        const auto& type = request_map.at("type").AsString();
        if (type == "Stop") {
            stops_requests.push_back(request_map);
        }
        if (type == "Bus") {
            buses_requests.push_back(request_map);
        }
    }
    return {buses_requests, stops_requests};
}
 
void JsonReader::AddStops(const json::Array& stops_requests, transport_catalogue::TransportCatalogue& catalogue) {
    for (const auto& request : stops_requests) {
        const auto& request_stops_map = request.AsDict(); 
            const transport_catalogue::Stop& stop = FillStop(request_stops_map);
            catalogue.AddStop({stop});
    }
}
 
void JsonReader::AddBuses(const json::Array& buses_requests, transport_catalogue::TransportCatalogue& catalogue) {
    for (const auto& request : buses_requests) {
        const auto& request_bus_map = request.AsDict();
        {
            const auto& bus = FillRoute(request_bus_map, catalogue);
            catalogue.AddBus(bus);
        }
    }
}
svg::Color JsonReader::ReadColor(const json::Node &json) const{
    if (json.IsString()) {
        return json.AsString();
    }
    else if (json.IsArray()) {
         const json::Array& underlayer_color = json.AsArray();
        if (underlayer_color.size() == 3) {
            return svg::Rgb(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt());
        }
        else if (underlayer_color.size() == 4) {
            return svg::Rgba(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt(), underlayer_color[3].AsDouble());
        } else {
            throw std::logic_error("wrong underlayer colortype");
        }
    } else {
        throw std::logic_error("wrong underlayer color");
    }
}
 
std::vector<svg::Color> JsonReader::ReadColors(const json::Array &json) const {
    std::vector<svg::Color> colors;
    colors.reserve(json.size());
    for (const auto &item : json) {
       colors.push_back(ReadColor(item));
    }
   
    return colors;
}
renderer::MapRenderer JsonReader::FillRenderSettings(const json::Dict& request_map) const {
    renderer::RenderSettings render_settings;
    render_settings.width = request_map.at("width").AsDouble();
    render_settings.height = request_map.at("height").AsDouble();
    render_settings.padding = request_map.at("padding").AsDouble();
    render_settings.stop_radius = request_map.at("stop_radius").AsDouble();
    render_settings.line_width = request_map.at("line_width").AsDouble();
    render_settings.bus_label_font_size = request_map.at("bus_label_font_size").AsInt();
    const json::Array& bus_label_offset = request_map.at("bus_label_offset").AsArray();
    render_settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };
    render_settings.stop_label_font_size = request_map.at("stop_label_font_size").AsInt();
    const json::Array& stop_label_offset = request_map.at("stop_label_offset").AsArray();
    render_settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };
    render_settings.underlayer_color = ReadColor(request_map.at("underlayer_color"));
    render_settings.underlayer_width = request_map.at("underlayer_width").AsDouble();
    render_settings.color_palette = ReadColors(request_map.at("color_palette").AsArray());
     
    return render_settings;
}
 
void JsonReader::ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const {
    json::Array result;
    for (const auto& request : stat_requests.AsArray()) {
        const auto& request_map = request.AsDict();
        const auto& type = request_map.at("type").AsString();
        if (type == "Stop") {
            result.push_back(PrintStop(request_map, rh).AsDict());
        }
        if (type == "Bus") {
            result.push_back(PrintBus(request_map, rh).AsDict());
        }
        if (type == "Map") {
            result.push_back(PrintMap(request_map, rh).AsDict());
        }
    }
 
    json::Print(json::Document{ result }, std::cout);
}
 
 const json::Node JsonReader::PrintBus(const json::Dict& request_map, RequestHandler& rh) const {
    json::Node result;
    const std::string& route_number = request_map.at("name").AsString();
    const int id = request_map.at("id").AsInt();
     if (!rh.IsBusNumber(route_number)) {
        result = json::Builder{}
        .StartDict()
            .Key("request_id").Value(id)
            .Key("error_message").Value("not found")
        .EndDict()
    .Build();

    }
    else {
        const transport_catalogue::BusInfo route_info = rh.GetBusStat(route_number);
        result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("curvature").Value(route_info.curvature)
                        .Key("route_length").Value(route_info.length_route)
                        .Key("stop_count").Value(static_cast<int>(route_info.size))
                        .Key("unique_stop_count").Value(static_cast<int>(route_info.unique_stops))
                    .EndDict()
                .Build();
    }
    return json::Node{ result };
}
  
const json::Node JsonReader::PrintStop(const json::Dict& request_map, RequestHandler& rh) const {
    json::Node result;
    const std::string& stop_name = request_map.at("name").AsString();
    const int id = request_map.at("id").AsInt();
    if (!rh.IsStopName(stop_name)) {
        result = json::Builder{}
            .StartDict()
                .Key("request_id").Value(id)
                .Key("error_message").Value("not found")
            .EndDict()
        .Build();
    }
    else {
        std::vector<std::string> buses_vector = rh.GetSortedBusesToStop(stop_name);
        json::Array buses;
        for (const auto& bus_name: buses_vector) {
            buses.push_back(bus_name);
        }
        result = json::Builder{}
            .StartDict()
                .Key("request_id").Value(id)
                .Key("buses").Value(buses)
            .EndDict()
        .Build();
    }
  
    return json::Node{ result };
}
 
const json::Node JsonReader::PrintMap(const json::Dict& request_map, RequestHandler& rh) const {
    json::Node result;
    const int id = request_map.at("id").AsInt();
    std::ostringstream strm;
    svg::Document map = rh.RenderMap();
    map.Render(strm);
    result = json::Builder{}
        .StartDict()
            .Key("request_id").Value(id)
            .Key("map").Value(strm.str())
        .EndDict()
    .Build();
 
    return json::Node{result};
}