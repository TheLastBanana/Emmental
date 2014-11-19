#include "Common.h"
#include "RangedManager.h"

// Get effective range of unit, taking into account the fact that it can move
double getEffectiveRange(BWAPI::Unit * unit)
{
	double speedWeight = 1.5;
	return unit->getType().groundWeapon().maxRange() + unit->getType().topSpeed() * speedWeight;
}

RangedManager::RangedManager() { }

// Check that the given position is a good place to flee given enemy positions
bool RangedManager::checkFleePosition(const BWAPI::Unit * rangedUnit, const UnitVector & targets, const BWAPI::Position & fleePosition)
{
	// check that the position is free
	if (!checkPositionWalkable(fleePosition))
	{
		return false;
	}

	// Check that no enemies in view can target this position
	BOOST_FOREACH(BWAPI::Unit * enemy, BWAPI::Broodwar->enemy()->getUnits())
	{
		// ignore the enemy if it's not going to damage us
		if (enemy->getType().groundWeapon().damageAmount() == 0 || enemy->getType().isWorker()) continue;

		if (enemy->getDistance(fleePosition) < getEffectiveRange(enemy))
		{
			return false;
		}
	}

	return true;
}

// Try fleeing toward a position in a micromanaged way which allows squadmate avoidance
// Currently not used because it actually decreases effectiveness.
void RangedManager::flee(BWAPI::Unit * rangedUnit, const BWAPI::Position & fleePosition)
{
	// Plan only as far as we can make it in the next 10 frames
	double2 offset(fleePosition - rangedUnit->getPosition());
	offset.normalise();
	double fleeDist = std::min(rangedUnit->getType().topSpeed() * 10, rangedUnit->getPosition().getDistance(fleePosition));
	BWAPI::Position tempPosition(rangedUnit->getPosition() + offset * fleeDist);
	BWAPI::Position test(tempPosition);

	// Try to avoid running into squadmates
	double maxPush(20.0);	// How far we'll try to move if the unit is at minimum distance
	double dropOff(0.0001);	// How quickly push drops off given distance

	const UnitVector & rangedUnits = getUnits();
	BOOST_FOREACH(BWAPI::Unit * squadmate, rangedUnits)
	{
		if (squadmate == rangedUnit) break;

		double2 offset(rangedUnit->getPosition() - squadmate->getPosition());
		double dist = std::max(1.0, offset.lenSq());
		double push = std::min(maxPush, maxPush / (dist * dropOff));
		offset.normalise();
		offset = offset * push;

		tempPosition += offset;

		BWAPI::Broodwar->drawLineMap(test.x(), test.y(), test.x() + offset.x, test.y() + offset.y, BWAPI::Colors::Cyan);
	}

	// If the new position isn't free, just use let pathfinding do the work
	if (!checkPositionWalkable(tempPosition))
	{
		tempPosition = fleePosition;
	}

	// Force movement, even if we're mid-attack
	smartMove(rangedUnit, tempPosition, true);
}

void RangedManager::executeMicro(const UnitVector & targets) 
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

		// conditions for targeting
		if (targets[i]->isVisible()) 
		{
			rangedUnitTargets.push_back(targets[i]);
		}
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

void RangedManager::kiteTarget(BWAPI::Unit * rangedUnit, const UnitVector & targets, BWAPI::Unit * target)
{
	double range(rangedUnit->getType().groundWeapon().maxRange());
	if (rangedUnit->getType() == BWAPI::UnitTypes::Protoss_Dragoon && BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Singularity_Charge))
	{
		range = 6*32;
	}

	// determine whether the target can be kited
	if (range <= getEffectiveRange(target))
	{
		// if we can't kite it, there's no point
		smartAttackUnit(rangedUnit, target);
		return;
	}

	double		minDist(64);
	bool		kite(true);
	double		dist(rangedUnit->getDistance(target));
	double		speed(rangedUnit->getType().topSpeed());

	// don't kite damageless enemies
	if (target->getType().groundWeapon() == BWAPI::WeaponTypes::None) {
		kite = false;
	}

	// stay still if it'll take us longer to get back in range than to cooldown the weapon
	double	timeToEnter = std::max(0.0, (dist - range) / speed + rangedUnit->getType().acceleration() / 256.0);
	if ((timeToEnter >= rangedUnit->getGroundWeaponCooldown()) && (dist >= minDist))
	{
		kite = false;
	}

	// don't kite buildings
	if (target->getType().isBuilding() && target->getType() != BWAPI::UnitTypes::Terran_Bunker)
	{
		kite = false;
	}

	if (rangedUnit->isSelected())
	{
		BWAPI::Broodwar->drawCircleMap(rangedUnit->getPosition().x(), rangedUnit->getPosition().y(), 
			(int)range, BWAPI::Colors::Cyan);
	}

	// if we can't shoot, run away
	if (kite)
	{
		// direction from target to current unit (so we move away from the target)
		double2 direction(rangedUnit->getPosition() - target->getPosition());
		direction.normalise();

		BWAPI::Position fleePosition;
		bool found = false;

		// check in expanding circles for a valid flee position
		for (double moveDist = dist; moveDist < 300.0; moveDist += 10.0)
		{
			// check every 45 degrees, mirroring along the way.
			// direction is calculated based on the relative positions of the unit and its target,
			// so we start by checking if backpedaling slightly is possible, then consider other angles.
			// this goes up to 180, so it's possible to end up running past the target if it's the only option.
			for (double angle = 0; angle < 180.0; angle += 45.0)
			{
				// try both angles (clockwise and counter-clockwise)

				// get rotated offset
				double2 offset = direction * moveDist;
				offset.rotate(angle);

				// add offset to get flee position and check if it's a good place to run
				fleePosition = rangedUnit->getPosition() + offset;
				if (checkFleePosition(rangedUnit, targets, fleePosition))
				{
					found = true;
					break;
				}

				// rotate the other way
				offset = direction * moveDist;
				offset.rotate(-angle);

				// add offset to get flee position and check if it's a good place to run
				fleePosition = rangedUnit->getPosition() + offset;
				if (checkFleePosition(rangedUnit, targets, fleePosition))
				{
					found = true;
					break;
				}
			}
		}

		BWAPI::Broodwar->drawLineMap(rangedUnit->getPosition().x(), rangedUnit->getPosition().y(),
			fleePosition.x(), fleePosition.y(), BWAPI::Colors::Cyan);

		// no flee position found; just attack
		if (!found)
		{
			if (rangedUnit->isSelected()) BWAPI::Broodwar->printf("No escape!");

			smartAttackUnit(rangedUnit, target);
			return;
		}

		// Force movement, even if we're mid-attack
		smartMove(rangedUnit, fleePosition, true);
	}
	// otherwise shoot if not already fleeing
	else
	{
		smartAttackUnit(rangedUnit, target);
	}
}

// get a target for the zealot to attack
BWAPI::Unit * RangedManager::getTarget(BWAPI::Unit * rangedUnit, UnitVector & targets)
{
	int range(rangedUnit->getType().groundWeapon().maxRange());

	int highestInRangePriority(0);
	int highestNotInRangePriority(0);
	int lowestInRangeHitPoints(10000);
	int lowestNotInRangeDistance(10000);

	BWAPI::Unit * inRangeTarget = NULL;
	BWAPI::Unit * notInRangeTarget = NULL;

	BOOST_FOREACH(BWAPI::Unit * unit, targets)
	{
		int priority = getAttackPriority(rangedUnit, unit);
		int distance = rangedUnit->getDistance(unit);

		// if the unit is in range, update the target with the lowest hp
		if (rangedUnit->getDistance(unit) <= range)
		{
			if (priority > highestInRangePriority ||
				(priority == highestInRangePriority && unit->getHitPoints() < lowestInRangeHitPoints))
			{
				lowestInRangeHitPoints = unit->getHitPoints();
				highestInRangePriority = priority;
				inRangeTarget = unit;
			}
		}
		// otherwise it isn't in range so see if it's closest
		else
		{
			if (priority > highestNotInRangePriority ||
				(priority == highestNotInRangePriority && distance < lowestNotInRangeDistance))
			{
				lowestNotInRangeDistance = distance;
				highestNotInRangePriority = priority;
				notInRangeTarget = unit;
			}
		}
	}

	// if there is a highest priority unit in range, attack it first
	return (highestInRangePriority >= highestNotInRangePriority) ? inRangeTarget : notInRangeTarget;
}

	// get the attack priority of a type in relation to a zergling
int RangedManager::getAttackPriority(BWAPI::Unit * rangedUnit, BWAPI::Unit * target) 
{
	BWAPI::UnitType rangedUnitType = rangedUnit->getType();
	BWAPI::UnitType targetType = target->getType();

	bool canAttackUs = rangedUnitType.isFlyer() ? targetType.airWeapon() != BWAPI::WeaponTypes::None : targetType.groundWeapon() != BWAPI::WeaponTypes::None;

	// harass prioritizes workers
	if (order.type == order.Harass && targetType.isWorker())
	{
		return 4;
	}

	// highest priority is something that can attack us or aid in combat
	if (targetType == BWAPI::UnitTypes::Terran_Medic || canAttackUs ||
		targetType ==  BWAPI::UnitTypes::Terran_Bunker) 
	{
		return 3;
	} 
	// next priority is worker
	else if (targetType.isWorker()) 
	{
		return 2;
	} 
	// then everything else
	else 
	{
		return 1;
	}
}

BWAPI::Unit * RangedManager::closestrangedUnit(BWAPI::Unit * target, std::set<BWAPI::Unit *> & rangedUnitsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit * closest = NULL;

	BOOST_FOREACH (BWAPI::Unit * rangedUnit, rangedUnitsToAssign)
	{
		double distance = rangedUnit->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = rangedUnit;
		}
	}
	
	return closest;
}