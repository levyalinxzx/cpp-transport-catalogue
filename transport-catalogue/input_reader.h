#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "transport_catalogue.h"
namespace transport_catalogue{
namespace detail{
namespace command {
struct Description {
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;      // Название команды
    std::string id;           // id маршрута или остановки
    std::string description;  // Параметры команды
};
}//namespace command

class InputReader {
public:
    /**
     * Парсит строку в структуру CommandDescription и сохраняет результат в commands_stop и commands_bus
     */
    void Line(std::string_view line);

    /**
     * Наполняет данными транспортный справочник, используя команды из commands_
     */
    void ApplyCommands(TransportCatalogue& catalogue);

private:
    std::vector<command::Description> commands_stop;
    std::vector<command::Description> commands_bus;
};
}//namespace detail
}//namespace transport_catalogue
