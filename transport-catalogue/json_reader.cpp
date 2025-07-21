
 #include "json_reader.h"
 #include "transport_router.h"
 
 #include <sstream>

 using namespace std::literals;
  
 const json::Node& JsonReader::GetBaseRequests() const {
     const json::Dict& map = input_.GetRoot().AsDict();
     auto it = map.find("base_requests"s); 
     if (it == map.end()) {
         return dummy_;
     }
     return it->second;
 }
   
 const json::Node& JsonReader::GetStatRequests() const {
     const auto& map = input_.GetRoot().AsDict();
     auto it = map.find("stat_requests"s); 
     if (it == map.end()) {
         return dummy_;
     }
     return it->second;
 }
  
 const json::Node& JsonReader::GetRenderSettings() const {
     const auto& map = input_.GetRoot().AsDict();
     auto it = map.find("render_settings"s); 
     if (it == map.end()) {
         return dummy_;
     }
     return it->second;
 }

 const json::Node& JsonReader::GetRoutingSettings() const {
    const auto& map = input_.GetRoot().AsDict();
    auto it = map.find("routing_settings"s); 
    if (it == map.end()) {
        return dummy_;
    }
    return it->second;
}
   
 void JsonReader::FillCatalogue(tc::TransportCatalogue& catalogue) {
    const json::Array& arr = GetBaseRequests().AsArray();
    const auto& [buses_requests, stops_requests] = SortedRequests(arr);
    AddStops(stops_requests, catalogue);
    FillStopDistances(catalogue, stops_requests);
    AddBuses(buses_requests, catalogue);
 }
   
const tc::Stop JsonReader::FillStop(const json::Dict& request_map) const {
     std::string stop_name = request_map.at("name"s).AsString();
     geo::Coordinates coordinates = { request_map.at("latitude"s).AsDouble(), request_map.at("longitude"s).AsDouble() };
     return {stop_name, coordinates};
 }
   
 void JsonReader::FillStopDistances(tc::TransportCatalogue& catalogue, const json::Array& stops_requests) const {
     for (const auto& requests : stops_requests) {
         const auto& request_stops_map = requests.AsDict();{
             const tc::Stop& stop = FillStop(request_stops_map);
             auto& distances = request_stops_map.at("road_distances"s).AsDict();
             for (auto& [to_name, dist] : distances) {
                 auto from = catalogue.GetStop(stop.name_);
                 auto to = catalogue.GetStop(to_name);
                 catalogue.SetDistance(from, to, dist.AsInt());
             }
         }
     }
 }

tc::router::RoutingSettings JsonReader::FillRoutingSettings(const json::Node& settings) const {
    std::chrono::minutes bus_wait_time = std::chrono::minutes(settings.AsDict().at("bus_wait_time"s).AsInt());
    return tc::router::RoutingSettings{bus_wait_time, settings.AsDict().at("bus_velocity"s).AsDouble() };
 }
   
const tc::Bus JsonReader::FillRoute(const json::Dict& request_map, tc::TransportCatalogue& catalogue) const {
     std::string bus_number = request_map.at("name"s).AsString();
     tc::Route stops;
    bool circular_route = request_map.at("is_roundtrip"s).AsBool();
    for (const auto& stop : request_map.at("stops"s).AsArray()) {
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
         const auto& type = request_map.at("type"s).AsString();
         if (type == "Stop"s) {
            stops_requests.push_back(request_map);
         }
         if (type == "Bus"s) {
            buses_requests.push_back(request_map);
         }
     }
     return {buses_requests, stops_requests};
 }
  
 void JsonReader::AddStops(const json::Array& stops_requests, tc::TransportCatalogue& catalogue) {
    for (const auto& request : stops_requests) {
        const auto& request_stops_map = request.AsDict(); 
            const tc::Stop& stop = FillStop(request_stops_map);
            catalogue.AddStop(stop);
    }
 }
  
 void JsonReader::AddBuses(const json::Array& buses_requests, tc::TransportCatalogue& catalogue) {
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
             throw std::logic_error("wrong underlayer colortype"s);
         }
     } else {
         throw std::logic_error("wrong underlayer color"s);
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
     render_settings.width = request_map.at("width"s).AsDouble();
     render_settings.height = request_map.at("height"s).AsDouble();
     render_settings.padding = request_map.at("padding"s).AsDouble();
     render_settings.stop_radius = request_map.at("stop_radius"s).AsDouble();
     render_settings.line_width = request_map.at("line_width"s).AsDouble();
     render_settings.bus_label_font_size = request_map.at("bus_label_font_size"s).AsInt();
     const json::Array& bus_label_offset = request_map.at("bus_label_offset"s).AsArray();
     render_settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };
     render_settings.stop_label_font_size = request_map.at("stop_label_font_size"s).AsInt();
     const json::Array& stop_label_offset = request_map.at("stop_label_offset"s).AsArray();
     render_settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };
     render_settings.underlayer_color = ReadColor(request_map.at("underlayer_color"s));
     render_settings.underlayer_width = request_map.at("underlayer_width"s).AsDouble();
     render_settings.color_palette = ReadColors(request_map.at("color_palette"s).AsArray());
      
     return render_settings;
 }
  
 void JsonReader::ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const {
     json::Builder json_builder;
     
     json_builder.StartArray();
        for (const auto& request : stat_requests.AsArray()) {
         const auto& request_map = request.AsDict();
         json_builder.StartDict()
         .Key("request_id").Value(request_map.at("id"s).AsInt());
         const auto& type = request_map.at("type"s).AsString();
         if (type == "Stop"s) {
            std::string name = request_map.at("name"s).AsString();
            PrintStop(json_builder, request_map, rh);
         }
         if (type == "Bus"s) {
            PrintBus(json_builder, request_map, rh);
         }
         if (type == "Map"s) {
            PrintMap(json_builder, rh);
         }
         if (type == "Route"s) {
            PrintRoute(json_builder, request_map, rh);
        }
        json_builder.EndDict();
     }
    json_builder.EndArray();
  
     json::Print(json::Document{ json_builder.Build() }, std::cout);
 }
  
  void JsonReader::PrintBus(json::Builder& builder, const json::Dict& request_map, RequestHandler& rh) const {
     const std::string& route_number = request_map.at("name"s).AsString();
      if (!rh.IsBusNumber(route_number)) {
         builder.Key("error_message"s).Value("not found"s);
     }
     else {
         const tc::BusInfo route_info = rh.GetBusStat(route_number);
        builder.Key("curvature"s).Value(route_info.curvature)
            .Key("route_length"s).Value(route_info.length_route)
            .Key("stop_count"s).Value(static_cast<int>(route_info.size))
            .Key("unique_stop_count"s).Value(static_cast<int>(route_info.unique_stops));
     }
 }
   
 void JsonReader::PrintStop(json::Builder& builder, const json::Dict& request_map, RequestHandler& rh) const {
     json::Node result;
     const std::string& stop_name = request_map.at("name"s).AsString();
     if (!rh.IsStopName(stop_name)) {
            builder.Key("error_message"s).Value("not found"s);     
    }
     else {
        const auto sorted_buses = rh.GetBusesToStop(stop_name);
         json::Array buses;
         for (const auto& bus_name: *sorted_buses) {
            buses.push_back(bus_name->name_);
         }
            builder.Key("buses"s).Value(buses);
     }
   
 }
  
 void JsonReader::PrintMap(json::Builder& builder, RequestHandler& rh) const {
     std::ostringstream strm;
     svg::Document map = rh.RenderMap();
     map.Render(strm);
     builder.Key("map"s).Value(strm.str());
  
 }

  
void JsonReader::PrintRoute(json::Builder& builder, const json::Dict& request_map, RequestHandler& rh) const {
    json::Node result;
    std::string from = request_map.at("from"s).AsString();
    std::string to = request_map.at("to"s).AsString();
    const auto route = rh.FindRoute(from, to);
    //.Key("error_message"s).Value("not found"s);
    if (!route.has_value()) {
        builder.Key("error_message"s).Value("not found"s);
        return;
    }
    builder.Key("total_time"s).Value(route->total_time.count())
            .Key("items"s).StartArray();

    for (const auto &item : route->items) {
        std::visit(
          [&builder, this](const auto &item) {
            BuildRouteItem(builder, item);
          },
          item);
      }
    builder.EndArray();

}

void JsonReader::BuildRouteItem(json::Builder& builder, const tc::router::RouteInfo::BusItem& item) const {
  
    builder.StartDict()
      .Key("type"s).Value("Bus"s)
      .Key("bus"s).Value(item.bus->name_)
      .Key("time"s).Value(item.time.count())
      .Key("span_count"s).Value(static_cast<int>(item.span_count))
      .EndDict();
}

void JsonReader::BuildRouteItem(json::Builder& builder, const tc::router::RouteInfo::WaitItem& item) const {
  
    builder.StartDict()
      .Key("type"s).Value("Wait"s)
      .Key("stop_name"s).Value(item.stop->name_)
      .Key("time"s).Value(item.time.count())
      .EndDict();
}
