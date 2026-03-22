#pragma once
#include <string>
struct CityConstructionInfo {
  std::string NodeName;
  std::string EntName;
  CityConstructionInfo(std::string N, std::string E)
      : NodeName(N), EntName(E) {}
};