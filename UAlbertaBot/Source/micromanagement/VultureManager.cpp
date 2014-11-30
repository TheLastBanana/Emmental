#include "VultureManager.h"

VultureManager::VultureManager() {}

void VultureManager::execute(const SquadOrder & inputOrder)
{
	// Nothing to do if we have no units
	if (getUnits().empty() || !(inputOrder.type == SquadOrder::Attack || inputOrder.type == SquadOrder::Defend || inputOrder.type == SquadOrder::Harass))
	{
		//BWAPI::Broodwar->printf("Gots no units, fix shit up (%d)", order.type);
		return;
	}
	order = inputOrder;
	drawOrderText();

	// Discover enemies within region of interest
	UnitVector nearbyEnemies;

	// if the order is to defend, we only care about units in the radius of the defense
	if (order.type == order.Defend)
	{
		MapGrid::Instance().GetUnitsWithInvisible(nearbyEnemies, order.position, 800, false, true);

	} // otherwise we want to see everything on the way
	else if (order.type == order.Attack || order.type == order.Harass)
	{
		MapGrid::Instance().GetUnits(nearbyEnemies, order.position, 800, false, true);
		BOOST_FOREACH(BWAPI::Unit * unit, getUnits())
		{
			BWAPI::Unit * u = unit;
			BWAPI::UnitType t = u->getType();
			MapGrid::Instance().GetUnitsWithInvisible(nearbyEnemies, unit->getPosition(), 800, false, true);
		}
	}

	// the following block of code attacks all units on the way to the order position
	// we want to do this if the order is attack, defend, or harass
	if (order.type == order.Attack || order.type == order.Defend || order.type == order.Harass)
	{
		// if this is a worker defense force
		if (getUnits().size() == 1 && getUnits()[0]->getType().isWorker())
		{
			executeMicro(nearbyEnemies);
		}
		// otherwise it is a normal attack force
		else
		{
			// remove enemy worker units unless they are in one of their occupied regions
			UnitVector workersRemoved;

			BOOST_FOREACH(BWAPI::Unit * enemyUnit, nearbyEnemies)
			{
				// if its not a worker add it to the targets
				if (!enemyUnit->getType().isWorker())
				{
					workersRemoved.push_back(enemyUnit);
				}
				// if it is a worker
				else
				{
					BOOST_FOREACH(BWTA::Region * enemyRegion, InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->enemy()))
					{
						// only add it if it's in their region
						if (BWTA::getRegion(BWAPI::TilePosition(enemyUnit->getPosition())) == enemyRegion)
						{
							workersRemoved.push_back(enemyUnit);
						}
					}
				}
			}

			// Allow micromanager to handle enemies
			executeMicro(workersRemoved);
		}
	}
}

void VultureManager::executeMicro(const UnitVector & targets)
{
	const UnitVector & rangedUnits = getUnits();

	// figure out targets
	UnitVector rangedUnitTargets;
	for (size_t i(0); i<targets.size(); i++)
	{
		if (order.type == order.Harass && !targets[i]->getType().isWorker())
		{
			continue;
		}

		rangedUnitTargets.push_back(targets[i]);
		// Used to removed not visible units here
	}

	// for each zealot
	BOOST_FOREACH(BWAPI::Unit * rangedUnit, rangedUnits)
	{
		// low health; drop a mine before we die
		/*
		if (rangedUnit->getHitPoints() < 20 &&
		rangedUnit->getSpiderMineCount() == 3) // TODO: make this only apply to the last mine
		{
		// don't act if we're trying to mine
		if (!rangedUnit->isIdle() &&
		rangedUnit->getLastCommand().getTechType() == BWAPI::TechTypes::Spider_Mines)
		{
		continue;
		}

		// drop a mine
		else
		{
		bool success = rangedUnit->useTech(BWAPI::TechTypes::Spider_Mines, rangedUnit->getPosition());
		if (success) BWAPI::Broodwar->printf("Placing mine");
		}
		}
		*/

		if (rangedUnit->getOrder() == BWAPI::Orders::VultureMine || rangedUnit->getOrder() == BWAPI::Orders::PlaceMine)
		{
			continue;
		}

		// if the order is to attack or defend
		if (order.type == order.Attack || order.type == order.Defend || order.type == order.Harass) {

			// if there are targets
			if (!rangedUnitTargets.empty())
			{
				// find the best target for this zealot
				BWAPI::Unit * target = getTarget(rangedUnit, rangedUnitTargets);

				// attack it
				kiteTarget(rangedUnit, rangedUnitTargets, target);
			}
			// if there are no targets
			else
			{
				// if we're not near the order position
				if (rangedUnit->getDistance(order.position) > 100)
				{
					// move to it
					if (order.type == order.Harass)
					{
						smartMove(rangedUnit, order.position);
					}
					else {
						smartAttackMove(rangedUnit, order.position);
					}
				}
			}
		}

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG)
		{
			BWAPI::Broodwar->drawLineMap(rangedUnit->getPosition().x(), rangedUnit->getPosition().y(),
				rangedUnit->getTargetPosition().x(), rangedUnit->getTargetPosition().y(), Options::Debug::COLOR_LINE_TARGET);
		}
	}
}