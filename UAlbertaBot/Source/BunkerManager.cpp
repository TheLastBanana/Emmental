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

int BunkerManager::bunkerNeedsFilling() const
{
	std::set<BWAPI::Unit*> allOwnedUnits = BWAPI::Broodwar->self()->getUnits();
	int numUnitsNeeded = 0;

	BOOST_FOREACH(BWAPI::Unit * bunker, bunkersToFill)
	{
		if (bunker->isCompleted() && bunker->getHitPoints() > 0 && bunker->exists())
		{
			int numInside = bunker->getLoadedUnits().size();
			// Check if there are empty spots. Bunker can hold maximum 4 marines.
			if (numInside < 4)
				numUnitsNeeded += 4 - numInside;
		}
	}
	return numUnitsNeeded;
}

// this updates checks if each bunker needs some marines.
void BunkerManager::update()
{
	static int lastTime = 0;
	int timeNow = BWAPI::Broodwar->elapsedTime();

	//check every second only because this is expensive :(
	if (timeNow - lastTime > 1)
	{
		// Check and update if bunkers need some marines.
		std::set<BWAPI::Unit*> allOwnedUnits = BWAPI::Broodwar->self()->getUnits();
		BOOST_FOREACH(BWAPI::Unit * bunker, allOwnedUnits)
		{
			BWAPI::UnitType type = bunker->getType();
			// remove any full bunker or dead that is found within the yet-to-fill-container.
			if (type == BWAPI::UnitTypes::Terran_Bunker && (bunker->getHitPoints() == 0 || bunker->getLoadedUnits().size() == 4))
			{
				std::set<BWAPI::Unit*>::iterator inList = bunkersToFill.find(bunker);
				if (inList != bunkersToFill.end())
					bunkersToFill.erase(inList);
			}
			// check if any bunkers need marines.
			if (type == BWAPI::UnitTypes::Terran_Bunker && bunker->isCompleted() && bunker->getHitPoints() > 0 && bunker->exists())
			{
				int numInside = bunker->getLoadedUnits().size();
				// Check if there are empty spots. Bunker can hold maximum 4 marines.
				if (numInside < 4)
					// pushing it multiple times so it can be assigned to a unit to get in easily/lazily
					bunkersToFill.insert(bunker);
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

BunkerManager & BunkerManager::Instance()
{
	static BunkerManager instance;
	return instance;
}
