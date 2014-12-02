#include "VultureManager.h"

VultureManager::VultureManager() {}

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

		bool weCanHit = !targets[i]->getType().isFlyer();

		// conditions for targeting
		if (targets[i]->isVisible() && weCanHit)
		{
			rangedUnitTargets.push_back(targets[i]);
		}
	}

	// figure out stealth targets
	UnitVector stealthTargets;
	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		// Add stealth units. If they have wraiths hope our wraiths fight them unstealthed
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Lurker ||
			unit->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar)
		{
			stealthTargets.push_back(unit);
		}
	}

	// for each zealot
	BOOST_FOREACH(BWAPI::Unit * rangedUnit, rangedUnits)
	{
		UnitVector effectiveTargets(rangedUnitTargets);
		// Deal with stealth units on a unit by unit basis
		BOOST_FOREACH(BWAPI::Unit * stealthEnemy, stealthTargets) {
			if (rangedUnit->getDistance(stealthEnemy) <= stealthEnemy->getType().sightRange()) {
				effectiveTargets.push_back(stealthEnemy);
			}
		}

		if (rangedUnit->getOrder() == BWAPI::Orders::VultureMine || rangedUnit->getOrder() == BWAPI::Orders::PlaceMine)
		{
			continue;
		}

		// if the order is to attack or defend
		if (order.type == order.Attack || order.type == order.Defend || order.type == order.Harass) {

			// if there are targets
			if (!effectiveTargets.empty())
			{
				// find the best target for this zealot
				BWAPI::Unit * target = getTarget(rangedUnit, effectiveTargets);

				// attack it
				kiteTarget(rangedUnit, effectiveTargets, target);
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
