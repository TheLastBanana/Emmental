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
		if (bunkerToRepairSet.find(unit) != bunkerToRepairSet.end())
			bunkerToRepairSet.erase(unit);
		if (bunkersToFill.find(unit) != bunkersToFill.end())
			bunkersToFill.erase(unit);
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
			// remove any full bunker that is found within the yet-to-fill-container.
			if (bunker->getLoadedUnits().size() == 4)
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
				if (bunkerToRepairSet.find(bunker) == bunkerToRepairSet.end())
					bunkerToRepairSet.insert(bunker);
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

const std::set<BWAPI::Unit*> &BunkerManager::allBunkers() const
{
	return bunkersAll;
}

void BunkerManager::setBunkerSlave(BWAPI::Unit* slave)
{
	bunkerRepairSlave = slave;
}

void BunkerManager::updateBunkerSlave()
{
	// if no bunker slave exists or no bunkers exist
	if (bunkerRepairEmpty() || bunkerToRepairSet.size() == 0 )
		return;
	
	// tell slave to stay near bunker if it is not repairing.
	BWAPI::Unit* bunker = *bunkerToRepairSet.begin();
	if (!bunkerRepairSlave->isRepairing() && bunkerRepairSlave->getDistance(bunker) > 60)
		bunkerRepairSlave->move(bunker->getPosition());

	// tell slave to repair bunker if it is not 100%
	BOOST_FOREACH(BWAPI::Unit* bunkerToRepair, bunkerToRepairSet)
	{
		if (bunkerToRepair->getHitPoints() < 350 && !bunkerRepairSlave->isRepairing())
			bunkerRepairSlave->repair(bunkerToRepair);
	}
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
