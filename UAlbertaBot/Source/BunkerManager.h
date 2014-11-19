//should rename it to BunkerDefense?? since its not just building but also maintaining marines into it. 
#pragma once

#include "common.h"
#include "micromanagement\MicroManager.h"
//#include "squad.h"

class MicroManager;

class BunkerManager : public MicroManager
{
	void executeMicro(const UnitVector & targets);
	std::set<BWAPI::Unit*> bunkersToFill;

public:
	
	BunkerManager() {}
	~BunkerManager() {}

	// returns the number of units needed to fill a bunker
	int bunkerNeedsFilling();

	// returns true if all bunkers are full.
	bool allBunkersFull();

	static BunkerManager & Instance();
	void update();
};