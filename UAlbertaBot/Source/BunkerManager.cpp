#include "common.h"
#include "BunkerManager.h"

BunkerManager::BunkerManager()
{
	bunkerRepairSlave = 0; 
	maxReplaceSlave = 6; 
	replacedSlave = 0; 
	start = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	// ASCII art from http://www.retrojunkie.com/asciiart/food/cheese.htm
	// 11 lines max is the chat high.
	/*BWAPI::Broodwar->sendText(".      ___ ___");
	BWAPI::Broodwar->sendText(".     /\\ (_)    \\");
	BWAPI::Broodwar->sendText(".    /  \\      (_,");
	BWAPI::Broodwar->sendText(".   _)  _\\  _     \\");
	BWAPI::Broodwar->sendText(".  /   (_)\\_( )____\\");
	BWAPI::Broodwar->sendText(".  \\_     /    _  _/");
	BWAPI::Broodwar->sendText(".    )  /\\/  _ (_)(");
	BWAPI::Broodwar->sendText(".    \\ \\_) (_)   /");
	BWAPI::Broodwar->sendText(".     \\/______/");
	BWAPI::Broodwar->sendText("Good luck and have cheese.");
	*/

	BWAPI::Broodwar->sendText(".       _--\"-.");
	BWAPI::Broodwar->sendText(".    .-\"      \"-.");
	BWAPI::Broodwar->sendText(".   |\"\"--..      '-.");
	BWAPI::Broodwar->sendText(".   |     \"\"--..   '-.");
	BWAPI::Broodwar->sendText(".   |.-.  .-\".   \"\"--..\".");
	BWAPI::Broodwar->sendText(".   |'.'  -_'  .-.      |");
	BWAPI::Broodwar->sendText(".   |     .-.  '-'    .-'");
	BWAPI::Broodwar->sendText(".   '--..  '-'     .-. '.");
	BWAPI::Broodwar->sendText(".      \"\"--..    '_'  |");
	BWAPI::Broodwar->sendText("Good luck \"\"--..   |");
	BWAPI::Broodwar->sendText("Have cheese   \"-' ");
}

void BunkerManager::executeMicro(const UnitVector & targets)
{
	// Pfffft units are in wait in bunkers and attack automatically.
	UnitVector a = targets;
	//targets.noCheeseForThem();
}

void BunkerManager::patrolling(BWAPI::Unit* unit, BWAPI::Position target, int radius) const
{
	UnitVector enemyNear;
	MapGrid::Instance().GetUnits(enemyNear, target, 400, false, true);

	if (enemyNear.size() > 0)
		unit->attack(enemyNear.front());
	else
		//unit->move(target);
		unit->patrol(target + BWAPI::Position((rand() % radius) - (radius/2), (rand() % radius) - (radius/2)));
}

void BunkerManager::orderMarines() const
{
	// getting all units of this bunkerManager squad.
	const UnitVector & bunkerFodder = getUnits();
	
	// patrol/assign at most 4 marines on each bunker.
	int numAssigned = 0;

	std::set<BWAPI::Unit*>::const_iterator begin = bunkersToFill.begin();
	std::set<BWAPI::Unit*>::const_iterator end = bunkersToFill.end();
	std::set<BWAPI::Unit*>::const_iterator it = begin;
	BOOST_FOREACH(BWAPI::Unit* unit, bunkerFodder)
	{
		if (bunkersAll.size() == 0 || bunkersToFill.size() == 0)
		{
			// if no bunker, protect starting position.
			patrolling(unit, start, 700);
			continue;
		}
		if (it == end)
			it = begin;
		if ((*it)->isCompleted())
			// command unit to go into a bunker (right now at any distance).
			unit->rightClick(*it);
		else
			// patrol around the bunker
			patrolling(unit, BWAPI::Position((*it)->getPosition()), 200);
		if (numAssigned >= 4)
		{
			++it;
			numAssigned = 0;
		}
		++numAssigned;
	}
}

void BunkerManager::updateBunkerSlave()
{
	// if no bunker slave exists or no bunkers exist
	if (bunkerRepairEmpty() || bunkersAll.size() == 0)
		return;

	// tell slave to repair bunker if it is not 100% and not dead.
	BOOST_FOREACH(BWAPI::Unit* bunkerToRepair, bunkersAll)
	{
		if (bunkerToRepair->isCompleted() && bunkerToRepair->getHitPoints() < 350 && !bunkerRepairSlave->isRepairing())
			bunkerRepairSlave->repair(bunkerToRepair);
	}
	// tell slave to stay near bunker if it is not repairing.
	BWAPI::Unit* bunker = *bunkersAll.begin();
	if (!bunkerRepairSlave->isRepairing() && bunkerRepairSlave->getDistance(bunker) > 60)
		bunkerRepairSlave->move(bunker->getPosition());
}

void BunkerManager::onUnitCreate(BWAPI::Unit* unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->self() && unit->getType() == BWAPI::UnitTypes::Terran_Bunker)
		bunkersAll.insert(unit);
}

void BunkerManager::onUnitDestroyed(BWAPI::Unit* bunkerIsKill)
{
	if (bunkerIsKill->getPlayer() == BWAPI::Broodwar->self() && bunkerIsKill->getType() == BWAPI::UnitTypes::Terran_Bunker)
	{
		if (bunkersToFill.find(bunkerIsKill) != bunkersToFill.end())
			bunkersToFill.erase(bunkerIsKill);

		bunkersAll.erase(bunkerIsKill);
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
	//SparCraft::Timer t;
	//t.start();
	static int lastTime = 0;
	int timeNow = BWAPI::Broodwar->elapsedTime();
	// Check every second only because this is expensive/intensive
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
			else
			{
				// check if any bunkers need marines and update the bunker to have a repair slave.
				int numInside = bunker->getLoadedUnits().size();
				// Check if there are empty spots. Bunker can hold maximum 4 marines.
				if (numInside < 4)
					bunkersToFill.insert(bunker);
			}
		}
		// Order marines into the bunkers or patrol
		if (getUnits().size() > 0)
			orderMarines();

		lastTime = BWAPI::Broodwar->elapsedTime();
	}
	// Call slave to repair if bunker needs some fixin'
	updateBunkerSlave();
	//BWAPI::Broodwar->printf("time in MS %4.10f",t.getElapsedTimeInMilliSec());
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
	if (replacedSlave < maxReplaceSlave)
	{
		bunkerRepairSlave = slave;
		++replacedSlave;
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

bool BunkerManager::replacedMaxSlaves() const
{
	return replacedSlave >= maxReplaceSlave;
}

BunkerManager & BunkerManager::Instance()
{
	static BunkerManager instance;
	return instance;
}
