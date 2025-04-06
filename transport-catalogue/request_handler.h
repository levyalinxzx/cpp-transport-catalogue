
 #pragma once

 #include "json.h"
 #include "transport_catalogue.h"
 #include "map_renderer.h"
 
 class RequestHandler {
 public:
    explicit RequestHandler(transport_catalogue::TransportCatalogue& catalogue, const renderer::MapRenderer& renderer)
        : renderer_(renderer)
        , catalogue_(catalogue)
    {
    }
    std::vector<std::string> GetSortedBusesToStop(std::string_view stop_name) const ;
    transport_catalogue::BusInfo GetBusInfo(std::string_view bus_name) const;
    bool IsBusNumber(const std::string_view bus_number) const ;
    bool IsStopName(const std::string_view stop_name) const;
    svg::Document RenderMap() const;
     
    
 private:
    const transport_catalogue::TransportCatalogue& catalogue_;
    const renderer::MapRenderer& renderer_;

 };