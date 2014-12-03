#include "Common.h"
#include "Squad.h"

int  Squad::lastRetreatSwitch = 0;
bool Squad::lastRetreatSwitchVal = false;

Squad::Squad(const UnitVector & units, SquadOrder order) 
	: units(units)
	, order(order)
{
}

void Squad::update()
{
	// update all necessary unit information within this squad
	updateUnits();

	
	// draw some debug info
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG && order.type == SquadOrder::Attack) 
	{
		BWAPI::Broodwar->drawTextScreen(200, 330, "%s", regroupStatus.c_str());

		BWAPI::Unit * closest = unitClosestToEnemy();
		if (closest && (BWAPI::Broodwar->getFrameCount() % 24 == 0))
		{
			//BWAPI::Broodwar->setScreenPosition(closest->getPosition().x() - 320, closest->getPosition().y() - 200);
		}
	}

	// determine whether or not we should regroup vultures (and tanks)
	const bool vulturesNeedToRegroup(needsToRegroup(vultureManager.getUnits()));
	if (vulturesNeedToRegroup) {
		InformationManager::Instance().lastFrameRegroup = 1;

		const BWAPI::Position regroupPosition(calcRegroupPosition(vultureManager.getUnits()));
		BWAPI::Broodwar->drawTextScreen(200, 150, "REGROUP");

		BWAPI::Broodwar->drawCircleMap(regroupPosition.x(), regroupPosition.y(), 30, BWAPI::Colors::Purple, true);

		vultureManager.regroup(regroupPosition);
		tankManager.regroup(regroupPosition);
	}
	else // otherwise, execute micro
	{
		InformationManager::Instance().lastFrameRegroup = 1;

		vultureManager.execute(order);
		tankManager.execute(order);
	}

	// determine whether or not we should regroup wraiths
	const bool wraithsNeedToRegroup(needsToRegroup(rangedManager.getUnits()));
	if (wraithsNeedToRegroup) {
		InformationManager::Instance().lastFrameRegroup = 1;

		const BWAPI::Position regroupPosition(calcRegroupPosition(rangedManager.getUnits()));
		BWAPI::Broodwar->drawTextScreen(200, 150, "REGROUP");

		BWAPI::Broodwar->drawCircleMap(regroupPosition.x(), regroupPosition.y(), 30, BWAPI::Colors::Purple, true);

		rangedManager.regroup(regroupPosition);
	}
	else // otherwise, execute micro
	{
		InformationManager::Instance().lastFrameRegroup = 1;

		rangedManager.execute(order);
	}

	// determine whether or not we should regroup melee (workers)
	const bool meleeNeedToRegroup(needsToRegroup(meleeManager.getUnits()));
	if (meleeNeedToRegroup) {
		InformationManager::Instance().lastFrameRegroup = 1;

		const BWAPI::Position regroupPosition(calcRegroupPosition(meleeManager.getUnits()));
		BWAPI::Broodwar->drawTextScreen(200, 150, "REGROUP");

		BWAPI::Broodwar->drawCircleMap(regroupPosition.x(), regroupPosition.y(), 30, BWAPI::Colors::Purple, true);

		meleeManager.regroup(regroupPosition);
	}
	else // otherwise, execute micro
	{
		InformationManager::Instance().lastFrameRegroup = 1;

		meleeManager.execute(order);
	}
}

void Squad::updateUnits()
{
	setAllUnits();
	setNearEnemyUnits();
	setManagerUnits();
}

void Squad::setAllUnits()
{
	// clean up the units vector just in case one of them died
	UnitVector goodUnits;
	BOOST_FOREACH(BWAPI::Unit * unit, units)
	{
		if( unit->isCompleted() && 
			unit->getHitPoints() > 0 && 
			unit->exists() &&
			unit->getPosition().isValid() &&
			unit->getType() != BWAPI::UnitTypes::Unknown)
		{
			goodUnits.push_back(unit);
		}
	}
	units = goodUnits;
}

void Squad::setNearEnemyUnits()
{
	nearEnemy.clear();
	BOOST_FOREACH(BWAPI::Unit * unit, units)
	{
		int x = unit->getPosition().x();
		int y = unit->getPosition().y();

		int left = unit->getType().dimensionLeft();
		int right = unit->getType().dimensionRight();
		int top = unit->getType().dimensionUp();
		int bottom = unit->getType().dimensionDown();

		nearEnemy[unit] = unitNearEnemy(unit);
		if (nearEnemy[unit])
		{
			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x-left, y - top, x + right, y + bottom, Options::Debug::COLOR_UNIT_NEAR_ENEMY);
		}
		else
		{
			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x-left, y - top, x + right, y + bottom, Options::Debug::COLOR_UNIT_NOTNEAR_ENEMY);
		}
	}
}

void Squad::setManagerUnits()
{
	UnitVector bunkerUnits;
	UnitVector vultures;
	UnitVector wraiths;
	UnitVector tanks;
	UnitVector melee;

	// add units to micro managers
	BOOST_FOREACH(BWAPI::Unit * unit, units)
	{
		if(unit->isCompleted() && unit->getHitPoints() > 0 && unit->exists())
		{
			// select marines to be defense and bunkers.
			if (unit->getType() == BWAPI::UnitTypes::Terran_Marine)
			{
				bunkerUnits.push_back(unit);
			}
			// select vultures
			else if (unit->getType() == BWAPI::UnitTypes::Terran_Vulture)
			{
				vultures.push_back(unit);
			}
			// select wraiths
			else if (unit->getType() == BWAPI::UnitTypes::Terran_Wraith)
			{
				wraiths.push_back(unit);
			}
			// select tanks
			else if (unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode || unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode)
			{
				tanks.push_back(unit);
			}
			else if (unit->getType() == BWAPI::UnitTypes::Terran_SCV) {
				melee.push_back(unit);
			} 
			else if (unit->getType() == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine) {} // Don't micro a mine
			else BWAPI::Broodwar->printf("Warn: unclassified unit: %s",
				unit->getType().c_str());
		}
	}

	vultureManager.setUnits(vultures);
	tankManager.setUnits(tanks);
	rangedManager.setUnits(wraiths);
	meleeManager.setUnits(melee);
	BunkerManager::Instance().setUnits(bunkerUnits);
}

// calculates whether or not to regroup
bool Squad::needsToRegroup(const UnitVector & u)
{
	// if we are not attacking, never regroup
	if (u.empty() || !(order.type == SquadOrder::Attack || order.type == SquadOrder::Harass))
	{
		regroupStatus = std::string("\x04 No combat units available");
		return false;
	}

	// for the Vulture rush strategy, we only care about whether we have enough vultures
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran &&
		StrategyManager::Instance().getCurrentStrategy() == StrategyManager::TerranVultureRush ||
		StrategyManager::Instance().getCurrentStrategy() == StrategyManager::TerranBugHunt)
	{
		// start attacking when we have 5 things in the vector
		return u.size() < 5;
	}

	// if we are DT rushing and we haven't lost a DT yet, no retreat!
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss &&
		StrategyManager::Instance().getCurrentStrategy() == StrategyManager::ProtossDarkTemplar &&
		(BWAPI::Broodwar->self()->deadUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) == 0))
	{
		regroupStatus = std::string("\x04 BLUE CHEESE HOOOOO!");
		return false;
	}

	BWAPI::Unit * unitClosest = unitClosestToEnemy();

	if (!unitClosest)
	{
		regroupStatus = std::string("\x04 No closest unit");
		return false;
	}

	CombatSimulation sim;
	sim.setCombatUnits(unitClosest->getPosition(), Options::Micro::COMBAT_REGROUP_RADIUS + InformationManager::Instance().lastFrameRegroup*300);
	ScoreType score = sim.simulateCombat();

    bool retreat = score < 0;
    int switchTime = 100;
    bool waiting = false;

    // we should not attack unless 5 seconds have passed since a retreat
    if (retreat != lastRetreatSwitchVal)
    {
        if (retreat == false && (BWAPI::Broodwar->getFrameCount() - lastRetreatSwitch < switchTime))
        {
            waiting = true;
            retreat = lastRetreatSwitchVal;
        }
        else
        {
            waiting = false;
            lastRetreatSwitch = BWAPI::Broodwar->getFrameCount();
            lastRetreatSwitchVal = retreat;
        }
    }
	
	if (retreat)
	{
		regroupStatus = std::string("\x04 Retreat - simulation predicts defeat");
	}
	else
	{
		regroupStatus = std::string("\x04 Attack - simulation predicts success");
	}

	return retreat;
}

void Squad::setSquadOrder(const SquadOrder & so)
{
	order = so;
}

bool Squad::unitNearEnemy(BWAPI::Unit * unit)
{
	assert(unit);

	UnitVector enemyNear;

	MapGrid::Instance().GetUnits(enemyNear, unit->getPosition(), 400, false, true);

	return enemyNear.size() > 0;
}

BWAPI::Position Squad::calcCenter()
{
	BWAPI::Position accum(0,0);
	BOOST_FOREACH(BWAPI::Unit * unit, units)
	{
		accum += unit->getPosition();
	}
	return BWAPI::Position(accum.x() / units.size(), accum.y() / units.size());
}

BWAPI::Position Squad::calcRegroupPosition(const UnitVector & u)
{
	BWAPI::Position regroup(0,0);

	int minDist(100000);

	BOOST_FOREACH(BWAPI::Unit * unit, u)
	{
		if (!nearEnemy[unit])
		{
			int dist = unit->getDistance(order.position);
			if (dist < minDist)
			{
				minDist = dist;
				regroup = unit->getPosition();
			}
		}
	}

	if (regroup == BWAPI::Position(0,0))
	{
		return BWTA::getRegion(BWTA::getStartLocation(BWAPI::Broodwar->self())->getTilePosition())->getCenter();
	}
	else
	{
		return regroup;
	}
}

BWAPI::Unit * Squad::unitClosestToEnemy()
{
	BWAPI::Unit * closest = NULL;
	int closestDist = 100000;

	BOOST_FOREACH (BWAPI::Unit * unit, units)
	{
		// the distance to the order position
		int dist = MapTools::Instance().getGroundDistance(unit->getPosition(), order.position);

		if (dist != -1 && (!closest || dist < closestDist))
		{
			closest = unit;
			closestDist = dist;
		}
	}

	if (!closest)
	{
		BOOST_FOREACH (BWAPI::Unit * unit, units)
		{
			// the distance to the order position
			int dist = unit->getDistance(BWAPI::Position(BWAPI::Broodwar->enemy()->getStartLocation()));

			if (dist != -1 && (!closest || dist < closestDist))
			{
				closest = unit;
				closestDist = dist;
			}
		}
	}

	return closest;
}

int Squad::squadUnitsNear(BWAPI::Position p)
{
	int numUnits = 0;

	BOOST_FOREACH (BWAPI::Unit * unit, units)
	{
		if (unit->getDistance(p) < 600)
		{
			numUnits++;
		}
	}

	return numUnits;
}

bool Squad::squadObserverNear(BWAPI::Position p)
{
	BOOST_FOREACH (BWAPI::Unit * unit, units)
	{
		if (unit->getType().isDetector() && unit->getDistance(p) < 300)
		{
			return true;
		}
	}

	return false;
}

const UnitVector &Squad::getUnits() const	
{ 
	return units; 
} 

const SquadOrder & Squad::getSquadOrder()	const			
{ 
	return order; 
}