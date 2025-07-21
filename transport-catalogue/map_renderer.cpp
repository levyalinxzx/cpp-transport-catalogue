
#include "map_renderer.h"


namespace renderer {
 
bool IsZero(double value) {
     return std::abs(value) < EPSILON;
}
 
std::vector<svg::Polyline> MapRenderer::GetRouteLines(const std::deque<tc::Bus>& buses, const SphereProjector& sp) const {
    std::vector<svg::Polyline> result;
    size_t color_num = 0;
    for (const auto& bus: buses) {
        if (bus.stops_.empty()) {
            continue;
        }
        svg::Polyline line;
        for (const auto& stop : bus.stops_) {
            line.AddPoint(sp(stop->coordinates_));
        }
        line.SetStrokeColor(render_settings_.color_palette[color_num]);
        line.SetFillColor("none");
        line.SetStrokeWidth(render_settings_.line_width);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
         
        if (color_num < (render_settings_.color_palette.size() - 1)) {
            ++color_num;
        }
        else {
            color_num = 0;
        }
         
        result.push_back(line);
    }
     
    return result;
}

 std::vector<svg::Text> MapRenderer::GetBusLabel(const std::deque<tc::Bus>& buses, const SphereProjector& sp) const {
    std::vector<svg::Text> result;
    size_t color_num = 0;
    for (const auto& bus : buses) {
        if (bus.stops_.empty()) {
            continue;
        }
        svg::Text text;
        svg::Text underlayer;
        text.SetPosition(sp(bus.stops_[0]->coordinates_));
        text.SetOffset(render_settings_.bus_label_offset);
        text.SetFontSize(render_settings_.bus_label_font_size);
        text.SetFontFamily("Verdana");
        text.SetFontWeight("bold");
        text.SetData(bus.name_);
        text.SetFillColor(render_settings_.color_palette[color_num]);
        if (color_num < (render_settings_.color_palette.size() - 1)) {
            ++color_num;
        }
        else {
            color_num = 0;
        }
        underlayer.SetPosition(sp(bus.stops_[0]->coordinates_));
        underlayer.SetOffset(render_settings_.bus_label_offset);
        underlayer.SetFontSize(render_settings_.bus_label_font_size);
        underlayer.SetFontFamily("Verdana");
        underlayer.SetFontWeight("bold");
        underlayer.SetData(bus.name_);
        underlayer.SetFillColor(render_settings_.underlayer_color);
        underlayer.SetStrokeColor(render_settings_.underlayer_color);
        underlayer.SetStrokeWidth(render_settings_.underlayer_width);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        result.push_back(underlayer);
        result.push_back(text);
        
        if (!bus.is_circle_ && bus.stops_[0] != bus.stops_[bus.stops_.size()/2]) {
            svg::Text text2 {text};
            svg::Text underlayer2 {underlayer};
            text2.SetPosition(sp(bus.stops_[bus.stops_.size()/2]->coordinates_));
            underlayer2.SetPosition(sp(bus.stops_[bus.stops_.size()/2]->coordinates_));
            
            result.push_back(underlayer2);
            result.push_back(text2);
        }
    }
    
    return result;
}

std::vector<svg::Circle> MapRenderer::GetStopsSymbols(std::map<std::string_view, const tc::Stop*> stops, const SphereProjector& sp) const {
    std::vector<svg::Circle> result;
    for (const auto& [stop_name, stop] : stops) {
        svg::Circle symbol;
        symbol.SetCenter(sp(stop->coordinates_));
        symbol.SetRadius(render_settings_.stop_radius);
        symbol.SetFillColor("white");
        
        result.push_back(symbol);
    }
    
    return result;
}

std::vector<svg::Text> MapRenderer::GetStopsLabels(std::map<std::string_view, const tc::Stop*> stops, const SphereProjector& sp) const {
    std::vector<svg::Text> result;
    svg::Text text;
    svg::Text underlayer;
    for (const auto& [stop_name, stop] : stops) {
        text.SetPosition(sp(stop->coordinates_));
        text.SetOffset(render_settings_.stop_label_offset);
        text.SetFontSize(render_settings_.stop_label_font_size);
        text.SetFontFamily("Verdana");
        text.SetData(stop->name_);
        text.SetFillColor("black");
        
        underlayer.SetPosition(sp(stop->coordinates_));
        underlayer.SetOffset(render_settings_.stop_label_offset);
        underlayer.SetFontSize(render_settings_.stop_label_font_size);
        underlayer.SetFontFamily("Verdana");
        underlayer.SetData(stop->name_);
        underlayer.SetFillColor(render_settings_.underlayer_color);
        underlayer.SetStrokeColor(render_settings_.underlayer_color);
        underlayer.SetStrokeWidth(render_settings_.underlayer_width);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        result.push_back(underlayer);
        result.push_back(text);
    }
    
    return result;
}
 
 svg::Document MapRenderer::GetSVG(const std::deque<tc::Bus>& buses) const {
    svg::Document result;
    std::vector<geo::Coordinates> route_stops_coord;
    std::map<std::string_view, const tc::Stop*> all_stops;
    
    for (const auto&  bus : buses) {
        if (bus.stops_.empty()) {
            continue;
        }
        for (const auto& stop : bus.stops_) {
            route_stops_coord.push_back(stop->coordinates_);
            all_stops[stop->name_] = stop;
        }
    }
    
    SphereProjector sp(route_stops_coord.begin(), route_stops_coord.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
    for (const auto& line : GetRouteLines(buses, sp)) {
        result.Add(line);
    }
    for (const auto& text : GetBusLabel(buses, sp)) {
        result.Add(text);
    }
    for (const auto& circle : GetStopsSymbols(all_stops, sp)) {
        result.Add(circle);
    }
    for (const auto& text : GetStopsLabels(all_stops, sp)) {
        result.Add(text);
    }
 
    return result;
}
 
} // namespace renderer
