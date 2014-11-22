#include "common.h"
#include "BunkerManager.h"

void BunkerManager::executeMicro(const UnitVector & targets)
{
	// Pfffft units are in wait in bunkers and attack automatically.
	UnitVector a = targets;
	//targets.noCheeseForThem();
}

void BunkerManager::orderMarines() const
{
	// getting all units of this bunkerManager squad.
	const UnitVector & bunkerFodder = getUnits();

	std::set<BWAPI::Unit*>::const_iterator begin = bunkersToFill.begin();
	std::set<BWAPI::Unit*>::const_iterator end = bunkersToFill.end();
	std::set<BWAPI::Unit*>::const_iterator it = begin;

	BOOST_FOREACH(BWAPI::Unit* unit, bunkerFodder){
		if (it == end)
			it = begin;
		// command unit to go into a bunker (right now at any distance).
		unit->rightClick(*it);
		it++;
	}
}

void BunkerManager::onUnitCreate(BWAPI::Unit* unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->self() && unit->getType() == BWAPI::UnitTypes::Terran_Bunker)
		bunkersAll.insert(unit);
}

void BunkerManager::onUnitDestroyed(BWAPI::Unit* unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->self() && unit->getType() == BWAPI::UnitTypes::Terran_Bunker)
	{
		if (bunkerToRepair == unit)
			bunkerToRepair = 0;
		bunkersAll.erase(unit);
	}
}

int BunkerManager::bunkerNeedsFilling() const
{
	int numUnitsNeeded = 0;

	BOOST_FOREACH(BWAPI::Unit * bunker, bunkersToFill)
	{
		int numInside = bunker->getLoadedUnits().size();
		// Check if there are empty spots. Bunker can hold maximum 4 marines.
		if (numInside < 4)
			numUnitsNeeded += 4 - numInside;
	}
	return numUnitsNeeded;
}

// this updates checks if each bunker needs some marines.
void BunkerManager::update()
{
	static int lastTime = 0;
	int timeNow = BWAPI::Broodwar->elapsedTime();

	//check every second only because this is expensive :(
	if (timeNow - lastTime >= 1)
	{
		// Check and update if bunkers need some marines.
		BOOST_FOREACH(BWAPI::Unit * bunker, bunkersAll)
		{
			// remove any full bunker or dead that is found within the yet-to-fill-container.
			if (bunker->getHitPoints() == 0 || bunker->getLoadedUnits().size() == 4)
			{
				std::set<BWAPI::Unit*>::iterator inList = bunkersToFill.find(bunker);
				if (inList != bunkersToFill.end())
					bunkersToFill.erase(inList);
			}
			// check if any bunkers need marines and update the bunker to have a repair slave.
			if (bunker->isCompleted() && bunker->getHitPoints() > 0 && bunker->exists())
			{
				int numInside = bunker->getLoadedUnits().size();
				// Check if there are empty spots. Bunker can hold maximum 4 marines.
				if (numInside < 4)
					bunkersToFill.insert(bunker);
				// if bunker to have a repair slave is dead, replace it with a new bunker.
				if (bunkerToRepair == 0 || bunkerToRepair->getHitPoints() == 0)
					bunkerToRepair = bunker;
			}
		}
		// Order marines into the bunkers if there are bunkers to fill.
		if (bunkersToFill.size() != 0)
			orderMarines();

		lastTime = BWAPI::Broodwar->elapsedTime();
	}
}

bool BunkerManager::allBunkersFull() const
{
	return bunkersToFill.size() == 0;
}

bool BunkerManager::bunkersExists() const
{
	return bunkersAll.size() > 0;
}

void BunkerManager::setBunkerSlave(BWAPI::Unit* slave)
{
	bunkerRepairSlave = slave;
}

void BunkerManager::updateBunkerSlave()
{
	// if no bunker slave exists or no bunkers exist
	if (bunkerRepairEmpty() || bunkerToRepair == 0 || bunkerToRepair->getHitPoints() == 0)
		return;
	
	// tell slave to stay near bunker!
	if (bunkerRepairSlave->getDistance(bunkerToRepair) > 80)
		bunkerRepairSlave->move(bunkerToRepair->getPosition());

	// tell slave to repair bunker if it is not 100%
	if (bunkerToRepair->getHitPoints() < 350 && !bunkerRepairSlave->isRepairing())
		bunkerRepairSlave->repair(bunkerToRepair);
}

bool BunkerManager::bunkerRepairEmpty() const
{
	return (bunkerRepairSlave == 0 || bunkerRepairSlave->getHitPoints() == 0);
}

BWAPI::Unit* BunkerManager::getBunkerSlave() const
{
	return bunkerRepairSlave;
}

BunkerManager & BunkerManager::Instance()
{
	static BunkerManager instance;
	return instance;
}
