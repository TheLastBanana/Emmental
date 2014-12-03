#include "TankManager.h"

TankManager::TankManager() {}

void TankManager::kiteTarget(BWAPI::Unit * rangedUnit, const UnitVector & targets, BWAPI::Unit * target)
{
	// Stand ground and attack
	if (rangedUnit->isInWeaponRange(target))
	{
		smartAttackUnit(rangedUnit, target);
	}
	else if (order.type != order.Defend)
	{
		smartAttackMove(rangedUnit, target->getPosition());
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
		// if we're not near the order position
		if (rangedUnit->getDistance(order.position) > 100)
		{
			// if there are targets, siege
			if (!rangedUnitTargets.empty())
			{
				bool toSiege = false;
				BOOST_FOREACH(const BWAPI::Unit * target, rangedUnitTargets)
				{
					// siege if something is in siege range
					if (rangedUnit->getDistance(target->getPosition()) <= BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode.groundWeapon().maxRange() &&
						!rangedUnit->isSieged())
					{
						toSiege = true;
						break;
					}
				}

				if (toSiege && !rangedUnit->isSieged())
				{
					rangedUnit->siege();
				}
				else if (!toSiege && rangedUnit->isSieged())
				{
					rangedUnit->unsiege();
				}
			}
			else
			{
				// move to it
				smartMove(rangedUnit, order.position);
			}
		}
		else
		{
			if (!rangedUnit->isSieged())
			{
				rangedUnit->siege();
			}
			else if (!rangedUnitTargets.empty())
			{
				// find the best target for this zealot
				BWAPI::Unit * target = getTarget(rangedUnit, rangedUnitTargets);

				// attack it
				kiteTarget(rangedUnit, rangedUnitTargets, target);
			}
		}

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG)
		{
			BWAPI::Broodwar->drawLineMap(rangedUnit->getPosition().x(), rangedUnit->getPosition().y(),
				rangedUnit->getTargetPosition().x(), rangedUnit->getTargetPosition().y(), Options::Debug::COLOR_LINE_TARGET);
		}
	}
}