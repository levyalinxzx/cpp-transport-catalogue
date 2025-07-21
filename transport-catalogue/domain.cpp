#include "domain.h"

#include <utility>

namespace tc{

Stop::Stop(std::string name, geo::Coordinates coordinates) : 
   name_(std::move(name)), coordinates_(coordinates) {}


Bus::Bus(std::string name, Route stops, bool is_circle) :
   name_(std::move(name)), stops_(std::move(stops)), is_circle_(is_circle)
    {}

bool Bus::operator<(Bus& other) {
    return std::lexicographical_compare(name_.begin(), name_.end(),
    other.name_.begin(), other.name_.end());
}

bool BusPtrComparator::operator()(Bus* lhs, Bus* rhs) const {
    return *lhs < *rhs;
}

size_t Hasher::operator()(const Stop *stop) const  {
    return std::hash<std::string>{}(stop->name_)
      + std::hash<double>{}(stop->coordinates_.lat) * salt
      + std::hash<double>{}(stop->coordinates_.lng) * salt * salt;
}

std::size_t Hasher::operator()(const std::pair<const Stop*, const Stop*> pair_stops) const noexcept {
    auto hash_1 = static_cast<const void*>(pair_stops.first);
    auto hash_2 = static_cast<const void*>(pair_stops.second);
    return hasher(hash_1) *  salt + hasher(hash_2);
}
}
