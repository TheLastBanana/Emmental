#include "TankManager.h"

TankManager::TankManager() {}

void TankManager::kiteTarget(BWAPI::Unit * rangedUnit, const UnitVector & targets, BWAPI::Unit * target)
{
	// Stand ground and attack
	if (rangedUnit->isInWeaponRange(target))
	{
		smartAttackUnit(rangedUnit, target);
		return;
	}
}

void TankManager::executeMicro(const UnitVector & targets)
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
		// siege once we're at the bunker
		if (BWAPI::Broodwar->getFrameCount() % 24)
		{
			BWAPI::Unit * bunker = NULL;

			// find the bunker
			BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
			{
				if (unit->getType() == BWAPI::UnitTypes::Terran_Bunker)
				{
					bunker = unit;
					break;
				}
			}

			if (bunker && rangedUnit->getPosition().getApproxDistance(bunker->getPosition()) < 250 &&
				BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Tank_Siege_Mode) &&
				!rangedUnit->isSieged() && !rangedUnit->isMoving())
			{
				rangedUnit->siege();

				return;
			}
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