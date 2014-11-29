#include "Common.h"
#include "BuildOrderGoalManager.h"

BuildOrderGoalItem::BuildOrderGoalItem(const MetaType & metaType, int count, int priority, bool blocking)
	: metaType(metaType), count(count), priority(priority), blocking(blocking)
{
	if (priority != -1) return;

	// set priorities
	switch (metaType.type) {
	case MetaType::Tech:
		priority = 10000;
		break;

	case MetaType::Upgrade:
		priority = 0;
		break;

	case MetaType::Unit:
		priority = 0;
		break;

	default:
		priority = 0;
		break;
	}
}

bool BuildOrderGoalManager::isCompleted(const BuildOrderGoalItem & bogi) const
{
	if (bogi.metaType.type == MetaType::Unit)
	{
		int unitCount = BWAPI::Broodwar->self()->allUnitCount(bogi.metaType.unitType);

		// we definitely have enough units
		if (unitCount >= bogi.count) return true;

		// if this is a building, a worker might already be on its way
		if (bogi.metaType.isBuilding()) {
			BOOST_FOREACH(const BWAPI::Unit *unit, BWAPI::Broodwar->self()->getUnits())
			{
				if (unit->getLastCommand().getUnitType() == bogi.metaType.unitType)
				{
					++unitCount;
					if (unitCount >= bogi.count)
					{
						return true;
					}
				}
			}
		}
		
		// not enough
		return false;
	}
	// if we have not researched that tech, return false
	else if (bogi.metaType.type == MetaType::Tech)
	{
		if (!BWAPI::Broodwar->self()->hasResearched(bogi.metaType.techType) &&
			!BWAPI::Broodwar->self()->isResearching(bogi.metaType.techType) &&
			bogi.count > 0)
		{
			return false;
		}
	}
	// if we have not upgraded to that level, return false
	else if (bogi.metaType.type == MetaType::Upgrade)
	{
		if (BWAPI::Broodwar->self()->getUpgradeLevel(bogi.metaType.upgradeType < bogi.count))
		{
			return false;
		}
	}

	return true;
}

BuildOrderGoalManager::BuildOrderGoalManager(const BOGIVector & items)
{
	BOOST_FOREACH(const BuildOrderGoalItem & item, items) {
		// don't bother queueing if it's done
		if (isCompleted(item))
		{
			continue;
		}

		// see if an item with this priority already exists
		int existingIndex = -1;
		for (int i(0); i < (int)goals.size(); ++i)
		{
			if (goals[i].priority == item.priority)
			{
				existingIndex = i;
				break;
			}
		}

		// if it already exists, add it to that goal
		if (existingIndex != -1)
		{
			goals[existingIndex].addItem(item);
		}
		// otherwise create a new goal
		else
		{
			BuildOrderGoal temp(item.priority);
			temp.addItem(item);
			goals.push_back(temp);
		}
	}

	// sort by priority
	std::sort(goals.rbegin(), goals.rend());
}

void BuildOrderGoalManager::getBuildOrder(std::vector<std::pair<MetaType, bool> > & buildOrder)
{
	BOOST_FOREACH(BuildOrderGoal & bog, goals)
	{
		bool complete = false;

		// keep repeating to queue items of same priority in a round-robin fashion
		while (!complete) {
			complete = true;

			BOOST_FOREACH(BuildOrderGoalItem & bogi, bog.items)
			{
				// this item hasn't been completed, so add one to the build order
				if (!isCompleted(bogi))
				{
					buildOrder.push_back(std::pair<MetaType, bool>(bogi.metaType, bogi.blocking));

					// subtract one so that it's considered as a built unit
					--bogi.count;

					complete = false;
				}
			}
		}
	}
}