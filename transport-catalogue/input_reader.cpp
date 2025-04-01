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

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */

geo::Coordinates ParseCoordinates(std::string_view str) {
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
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
            return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

command::Description ParseCommandDescription(std::string_view line) {
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

void ParseAndSetDistance(std::string_view line, std::string_view stop_name, TransportCatalogue& catalogue) {
    auto command = line.find(',');
    auto command2 = line.find(',', command + 1);
    if (command2 == line.npos) {
        return;
    }
    std::string_view str = line.substr(command2 + 1);
    auto pos = str.find_first_not_of(' ');
    str = str.substr(pos);
    while (command != str.npos) {
        command = str.find(',');
        std::string_view str2 = str.substr(0, command); 
        int distance = std::stoi(std::string(str2.substr(0, str.find('m'))));
        pos = str2.find("to");
        std::string_view stop_name2 = str2.substr(pos + 3, command - (pos + 3));
        str = str.substr(command + 1); 
        catalogue.SetDistance(catalogue.GetStop(stop_name), catalogue.GetStop(stop_name2), distance);	
    }  
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
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
    for (auto &command : commands_stop) {
        auto coordinates = ParseCoordinates(command.description);
        const Stop& stop = {command.id, coordinates};
        catalogue.AddStop(stop);
    }

    for (auto &command : commands_stop) {
        ParseAndSetDistance(command.description, command.id, catalogue);
    }

    for (auto &command : commands_bus) {
        auto route = ParseRoute(command.description);
        std::vector<const Stop*> helper;
        for (const auto& stop: route) {
            const Stop* info_stop = catalogue.GetStop(stop);
            helper.push_back(info_stop);
        }
        const Bus& bus = {command.id, helper};
        catalogue.AddBus(bus);
    }  
}
} //namespace detail 
} //namespace transport_catalogue
