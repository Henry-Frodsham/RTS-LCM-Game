// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <string>

// struct for storing info related to city construction
struct CityConstructionInfo {
  std::string NodeName;
  std::string EntName;
  CityConstructionInfo(std::string N, std::string E)
      : NodeName(N), EntName(E) {}
};

// struct for storing info related to unit construction
struct UnitConstructionInfo {
  std::string NodeName;
  std::string EntName;
  UnitConstructionInfo(std::string N, std::string E)
      : NodeName(N), EntName(E) {}
};
