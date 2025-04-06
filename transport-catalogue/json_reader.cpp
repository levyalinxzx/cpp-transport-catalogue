/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

 #include "json_reader.h"

 #include <sstream>

 const json::Node& JsonReader::GetBaseRequests() const {
     if (!input_.GetRoot().AsMap().count("base_requests")) return dummy_;
     return input_.GetRoot().AsMap().at("base_requests");
 }
 
 const json::Node& JsonReader::GetStatRequests() const {
     if (!input_.GetRoot().AsMap().count("stat_requests")) return dummy_;
     return input_.GetRoot().AsMap().at("stat_requests");
 }

 const json::Node& JsonReader::GetRenderSettings() const {
    if (!input_.GetRoot().AsMap().count("render_settings")) return dummy_;
    return input_.GetRoot().AsMap().at("render_settings");
}
 
 void JsonReader::FillCatalogue(transport_catalogue::TransportCatalogue& catalogue) {
    const json::Array& arr = GetBaseRequests().AsArray();
    const auto& [buses_requests, stops_requests] = SortedRequests(arr);
     for (auto& request : stops_requests) {
        const auto& request_stops_map = request.AsMap(); 
            const transport_catalogue::Stop& stop = FillStop(request_stops_map);
            catalogue.AddStop({stop});
    }
    FillStopDistances(catalogue, stops_requests);
     
    for (auto& request : buses_requests) {
        const auto& request_bus_map = request.AsMap();
        {
            const auto& bus = FillRoute(request_bus_map, catalogue);
            catalogue.AddBus(bus);
        }
    }
}
 
 const transport_catalogue::Stop JsonReader::FillStop(const json::Dict& request_map) const {
    std::string stop_name = request_map.at("name").AsString();
    geo::Coordinates coordinates = { request_map.at("latitude").AsDouble(), request_map.at("longitude").AsDouble() };
    return {stop_name, coordinates};
 }
 
 void JsonReader::FillStopDistances(transport_catalogue::TransportCatalogue& catalogue, const json::Array& stops_requests) const {
    for (auto& requests : stops_requests) {
        const auto& request_stops_map = requests.AsMap();{
            const transport_catalogue::Stop& stop = FillStop(request_stops_map);
            auto& distances = request_stops_map.at("road_distances").AsMap();
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
     for (auto& stop : request_map.at("stops").AsArray()) {
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
    for(auto& requests : base_request) {
        const auto& request_map = requests.AsMap();
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
    
    if (request_map.at("underlayer_color").IsString()) render_settings.underlayer_color = request_map.at("underlayer_color").AsString();
    else if (request_map.at("underlayer_color").IsArray()) {
        const json::Array& underlayer_color = request_map.at("underlayer_color").AsArray();
        if (underlayer_color.size() == 3) {
            render_settings.underlayer_color = svg::Rgb(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt());
        }
        else if (underlayer_color.size() == 4) {
            render_settings.underlayer_color = svg::Rgba(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt(), underlayer_color[3].AsDouble());
        } else throw std::logic_error("wrong underlayer colortype");
    } else throw std::logic_error("wrong underlayer color");
    
    render_settings.underlayer_width = request_map.at("underlayer_width").AsDouble();
    
    const json::Array& color_palette = request_map.at("color_palette").AsArray();
    for (const auto& color_element : color_palette) {
        if (color_element.IsString()) render_settings.color_palette.push_back(color_element.AsString());
        else if (color_element.IsArray()) {
            const json::Array& color_type = color_element.AsArray();
            if (color_type.size() == 3) {
                render_settings.color_palette.push_back(svg::Rgb(color_type[0].AsInt(), color_type[1].AsInt(), color_type[2].AsInt()));
            }
            else if (color_type.size() == 4) {
                render_settings.color_palette.push_back(svg::Rgba(color_type[0].AsInt(), color_type[1].AsInt(), color_type[2].AsInt(), color_type[3].AsDouble()));
            } else throw std::logic_error("wrong color_palette type");
        } else throw std::logic_error("wrong color_palette");
    }
    
    return render_settings;
}

void JsonReader::ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const {
    json::Array result;
    for (auto& request : stat_requests.AsArray()) {
        const auto& request_map = request.AsMap();
        const auto& type = request_map.at("type").AsString();
        if (type == "Stop") result.push_back(PrintStop(request_map, rh).AsMap());
        if (type == "Bus") result.push_back(PrintRoute(request_map, rh).AsMap());
        if (type == "Map") result.push_back(PrintMap(request_map, rh).AsMap());
    }

    json::Print(json::Document{ result }, std::cout);
}
 
 const json::Node JsonReader::PrintRoute(const json::Dict& request_map, RequestHandler& rh) const {
     json::Dict result;
     const std::string& route_number = request_map.at("name").AsString();
     result["request_id"] = request_map.at("id").AsInt();
     if (!rh.IsBusNumber(route_number)) {
         result["error_message"] = json::Node{ static_cast<std::string>("not found") };
     }
     else {
         result["curvature"] = rh.GetBusInfo(route_number).curvature;
         result["route_length"] = rh.GetBusInfo(route_number).length_route;
         result["stop_count"] = static_cast<int>(rh.GetBusInfo(route_number).size);
         result["unique_stop_count"] = static_cast<int>(rh.GetBusInfo(route_number).unique_stops);
     }
     return json::Node{ result };
 }
 
 const json::Node JsonReader::PrintStop(const json::Dict& request_map, RequestHandler& rh) const {
     json::Dict result;
     const std::string& stop_name = request_map.at("name").AsString();
     result["request_id"] = request_map.at("id").AsInt();
     if (!rh.IsStopName(stop_name)) {
         result["error_message"] = json::Node{ static_cast<std::string>("not found") };
     }
     else {
        std::vector<std::string> buses_vector = rh.GetSortedBusesToStop(stop_name);
        json::Array buses;
        for (auto& bus_name: buses_vector) {
            buses.push_back(bus_name);
        }
        result["buses"] = buses;
     }
 
     return json::Node{ result };
 }

 const json::Node JsonReader::PrintMap(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    result["request_id"] = request_map.at("id").AsInt();
    std::ostringstream strm;
    svg::Document map = rh.RenderMap();
    map.Render(strm);
    result["map"] = strm.str();

    return json::Node{result};
}