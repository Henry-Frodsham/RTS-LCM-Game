// Copyright © 2025 Henry Frodsham
#pragma once

//resources owned by a faction
enum FactionResource : size_t{
	CASH = 0,
	POPULATION,
	POPULATION_MAX
};

//resources owned by a unit (that is owned by a faction)
enum UnitResource : size_t {
	HEALTH = 0,

};

enum BuildingResource : size_t {
	BUILD_PROGRESS = 0,

};