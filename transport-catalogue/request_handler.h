#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
 
class RequestHandler {
public:
    explicit RequestHandler(tc::TransportCatalogue& catalogue, const renderer::MapRenderer& renderer, const tc::router::TransportRouter& router)
        : renderer_(renderer)
        , catalogue_(catalogue)
        , router_(router)
    {
    }
    const tc::Buses* GetBusesToStop(std::string_view stop_name) const ;
    bool IsBusNumber(const std::string_view bus_number) const ;
    bool IsStopName(const std::string_view stop_name) const;
    const tc::BusInfo GetBusStat(std::string_view bus_name) const;
    std::optional<tc::router::RouteInfo> FindRoute(std::string_view stop_name_from, std::string_view stop_name_to) const;
    
    svg::Document RenderMap() const;
     
    
private:
    const renderer::MapRenderer& renderer_;
    const tc::TransportCatalogue& catalogue_;
    const tc::router::TransportRouter& router_;
};
