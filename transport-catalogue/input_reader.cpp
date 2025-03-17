#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <iterator>

namespace transport_catalogue {
namespace detail {

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

namespace parse {
/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates Coordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto command = str.find(',');

    if (command == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', command + 1);

    double lat = std::stod(std::string(str.substr(not_space, command - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}
/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> Route(std::string_view route) {
    if (route.find('>') != route.npos) {
            return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

command::Description CommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}
}//namespace parse
void InputReader::Line(std::string_view line) {
    auto command_description = parse::CommandDescription(line);
    if (command_description) {
        if(command_description.command == "Stop") {
            commands_stop.push_back(std::move(command_description));
        }
        if(command_description.command == "Bus") {
            commands_bus.push_back(std::move(command_description));
        }
    }      
}


void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue)  {
    for (auto &command_description : commands_stop) {
        auto coordinates = parse::Coordinates(command_description.description);
        stop_detail::Stop stop = {command_description.id, coordinates};
        catalogue.AddStop(std::move(stop));
    }
    for (auto &command_description : commands_bus) {
        auto route = parse::Route(command_description.description);
        std::deque<const stop_detail::Stop*> helper;
        for (const auto& stop: route) {
            const stop_detail::Stop* info_stop = catalogue.GetStop(stop);
            helper.push_back(info_stop);
        }
        bus_detail::Bus bus = {command_description.id, helper};
        catalogue.AddBus(std::move(bus));
    }  
}
} //namespace detail 
} //namespace transport_catalogue// место для вашего кода
