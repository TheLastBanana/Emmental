#pragma once

#include "Common.h"
#include "BuildingData.h"
#include "MetaType.h"
#include "../InformationManager.h"

class BuildingPlacer
{

	BuildingPlacer();

	std::vector< std::vector<bool> > reserveMap;
	int buildDistance;

	int					boxTop, 
						boxBottom, 
						boxLeft, 
						boxRight;

 public:

	static BuildingPlacer & Instance();

	// queries for various BuildingPlacer data
	bool					buildable(int x, int y) const;
	bool					isReserved(int x, int y) const;
	bool					isInResourceBox(int x, int y) const;
	bool					tileOverlapsBaseLocation(BWAPI::TilePosition tile, BWAPI::UnitType type) const;

	// determines whether we can build at a given location
	bool					canBuildHere(BWAPI::TilePosition position, const Building & b) const;
	bool					canBuildHereWithSpace(BWAPI::TilePosition position, const Building & b, int buildDist, bool horizontalOnly = false) const;

	// returns a build location near a building's desired location
	BWAPI::TilePosition		getBuildLocationNear(const Building & b, int buildDist, bool inRegion = false, bool horizontalOnly = false, BWAPI::TilePosition pos = BWAPI::TilePosition (-1,-1)) const;
	
	void					reserveTiles(BWAPI::TilePosition position, int width, int height);
	void					freeTiles(BWAPI::TilePosition position, int width, int height);
	void					setBuildDistance(int distance);
	int						getBuildDistance() const;
	
	void					drawReservedTiles();
	void					computeResourceBox();
	
	BWAPI::TilePosition		getRefineryPosition();

	// Increments from the target towards the closeTo, bit by bit (tilePosition size) and check if can build there.
	// returns BWAPI::TilePositions::None if cannot find any space to build.
	BWAPI::TilePosition getPointClosestTo(BWAPI::TilePosition target, BWAPI::TilePosition closeTo, const Building & b, int bDist) const;
	
};