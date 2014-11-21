#pragma once

#include "common.h"
#include "micromanagement\MicroManager.h"

class MicroManager;

class BunkerManager : public MicroManager
{
	// contains bunkers that need some marines
	std::set<BWAPI::Unit*> bunkersToFill;

	// not used.
	void executeMicro(const UnitVector & targets);

	// orders marines to go into empty bunkers.
	void orderMarines() const;

public:
	
	BunkerManager() {}
	~BunkerManager() {}

	// returns the number of units needed to fill a bunker
	int bunkerNeedsFilling() const;

	// checks if bunkers need more marines or not, and then gets marines to enter them.
	void update();

	// returns true if all bunkers are full.
	bool allBunkersFull() const;

	// returns the instance of BunkerManager
	static BunkerManager & Instance();

};