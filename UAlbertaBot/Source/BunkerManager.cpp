#include "common.h"
#include "BunkerManager.h"

void BunkerManager::executeMicro(const UnitVector & targets)
{
	// Pfffft units are in wait in bunkers and attack automatically.
	UnitVector a = targets;
	//targets.noCheeseForThem();
}

int BunkerManager::bunkerNeedsFilling()
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
	//allBunkerFilled = numUnitsNeeded == 0;
	return numUnitsNeeded;
}

// this updates checks if each bunker needs some marines.
void BunkerManager::update()
{
	static int lastTime = 0;
	int timeNow = BWAPI::Broodwar->elapsedTime();
	
	// set check to 3 seconds if all bunkers are filled, if bunkers need some marines 1 sec.
	int check = bunkersToFill.size() == 0 ? 3 : 1;
	
	//check every couple seconds because this is expensive :(
	if (timeNow - lastTime > check)
	{
		// Check and update if bunkers need some marines.
		std::set<BWAPI::Unit*> allOwnedUnits = BWAPI::Broodwar->self()->getUnits();
		BOOST_FOREACH(BWAPI::Unit * bunker, allOwnedUnits)
		{
			BWAPI::UnitType type = bunker->getType();
			// remove any full bunker that is found within the yet-to-fill-container.
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
					for (int i = 0; i < 4-numInside; ++i)
						bunkersToFill.insert(bunker);
			}
		}
		if (bunkersToFill.size() != 0)
		{
			// getting all units of this bunkerManager squad.
			const UnitVector & bunkerFodder = getUnits(); 
			BOOST_FOREACH(BWAPI::Unit* unit, bunkerFodder){
				BOOST_FOREACH(BWAPI::Unit* bunker, bunkersToFill){
					// command unit to go into the bunker (right now at any distance...).
						unit->rightClick(bunker);
				}
			}
		}
		lastTime = BWAPI::Broodwar->elapsedTime();
	}
}

bool BunkerManager::allBunkersFull()
{
	return bunkersToFill.size() == 0;
}

BunkerManager & BunkerManager::Instance()
{
	static BunkerManager instance;
	return instance;
}

/*void CombatCommander::assignIdleSquads(std::set<BWAPI::Unit *> & unitsToAssign)
{
	if (unitsToAssign.empty()) { return; }

	UnitVector combatUnits(unitsToAssign.begin(), unitsToAssign.end());
	unitsToAssign.clear();

	squadData.addSquad(Squad(combatUnits, SquadOrder(SquadOrder::Defend, BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()), 1000, "Defend Idle")));
}*/