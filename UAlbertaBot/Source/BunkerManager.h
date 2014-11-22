#pragma once

#include "common.h"
#include "micromanagement\MicroManager.h"

class MicroManager;

class BunkerManager : public MicroManager
{
	// contains all bunkers
	std::set<BWAPI::Unit*> bunkersAll;

	// contains bunkers that need some marines
	std::set<BWAPI::Unit*> bunkersToFill;

	BWAPI::Unit* bunkerToRepair;
	BWAPI::Unit* bunkerRepairSlave;

	// not used.
	void executeMicro(const UnitVector & targets);

	// orders marines to go into empty bunkers.
	void orderMarines() const;

public:
	
	BunkerManager() { bunkerRepairSlave = bunkerToRepair = 0; }
	~BunkerManager() {}

	// if a bunker is made add it to bunkerlist
	void onUnitCreate(BWAPI::Unit* unit);

	// if a bunker is destroyed update bunkerlist
	void onUnitDestroyed(BWAPI::Unit* unit);

	// returns the number of units needed to fill a bunker
	int bunkerNeedsFilling() const;

	// checks if bunkers need more marines or not, and then gets marines to enter them.
	void update();

	// returns true if all bunkers are full.
	bool allBunkersFull() const;

	// returns true if there exist bunkers.
	bool BunkerManager::bunkersExists() const;

	// adds a unit (should be SCV) to the repair function.
	void setBunkerSlave(BWAPI::Unit* slave);

	// tells SCV slave to repair bunker if needed and stay near bunker at all times!
	void updateBunkerSlave();

	// returns true if it contains a dead SCV (or nothing). 
	bool bunkerRepairEmpty() const;

	// returns the bunker slave
	BWAPI::Unit* getBunkerSlave() const;

	// returns the instance of BunkerManager
	static BunkerManager & Instance();

};