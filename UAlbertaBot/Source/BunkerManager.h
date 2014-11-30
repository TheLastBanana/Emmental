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

	// bunker repair slave
	BWAPI::Unit* bunkerRepairSlave;

	// maximum number of slave replacements
	int maxReplaceSlave;

	// number of times slave has been replaced
	int replacedSlave;

	// starting position of our bot.
	BWAPI::Position start;

	// not used.
	void executeMicro(const UnitVector & targets);

	// patrolling functuion
	void patrolling(BWAPI::Unit* unit, BWAPI::Position target, int radius) const;

	// orders marines to go into empty bunkers.
	void orderMarines() const;

	// tells SCV slave to repair bunker if needed and stay near bunker at all times!
	void updateBunkerSlave();

public:
	
	BunkerManager();
	~BunkerManager() {}

	// if a bunker is made add it to bunkerlist
	void onUnitCreate(BWAPI::Unit* unit);

	// if a bunker is destroyed update bunkerlist
	void onUnitDestroyed(BWAPI::Unit* bunkerIsKill);

	// returns the number of units needed to fill a bunker
	int bunkerNeedsFilling() const;

	// checks if bunkers need more marines or not, and then gets marines to enter them.
	void update();

	// returns true if all bunkers are full.
	bool allBunkersFull() const;

	// returns the number of bunkers that exist.
	const std::set<BWAPI::Unit*> &allBunkers() const;

	// adds a unit (should be SCV) to the repair function.
	void setBunkerSlave(BWAPI::Unit* slave);

	// returns true if it contains a dead SCV (or nothing). 
	bool bunkerRepairEmpty() const;

	// returns the bunker slave
	BWAPI::Unit* getBunkerSlave() const;

	// returns true if maximum replaced bunker slaves reached
	bool replacedMaxSlaves() const;

	// returns the instance of BunkerManager
	static BunkerManager & Instance();

};