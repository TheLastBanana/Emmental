#include "Common.h"
#include "BuildOrderGoalManager.h"
#include "BuildingManager.h"

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

bool BuildOrderGoalManager::isCompleted(const BuildOrderGoalItem & bogi, const BuildOrder & buildOrder)
{
	if (bogi.count == 0) return true;

	int count = 0;
	BOOST_FOREACH(const ToBuild & toBuild, buildOrder)
	{
		if (toBuild.first == bogi.metaType)
		{
			++count;
		}
	}

	if (bogi.metaType.type == MetaType::Unit)
	{
		count += BWAPI::Broodwar->self()->allUnitCount(bogi.metaType.unitType);

		// tanks can be multiple types
		if (bogi.metaType.unitType == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
		{
			count += BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode);
		}

		// we definitely have enough units
		if (count >= bogi.count) return true;

		// check buildings under construction
		if (bogi.metaType.unitType.isBuilding())
		{
			count += BuildingManager::Instance().buildingCount(bogi.metaType.unitType);
		}

		if (count >= bogi.count) return true;
		
		// not enough
		return false;
	}
	// if we have not researched that tech, return false
	else if (bogi.metaType.type == MetaType::Tech)
	{
		if (!BWAPI::Broodwar->self()->hasResearched(bogi.metaType.techType) &&
			!BWAPI::Broodwar->self()->isResearching(bogi.metaType.techType) &&
			count == 0 &&
			bogi.count > 0)
		{
			return false;
		}
	}
	// if we have not upgraded to that level, return false
	else if (bogi.metaType.type == MetaType::Upgrade)
	{
		count += BWAPI::Broodwar->self()->getUpgradeLevel(bogi.metaType.upgradeType);
		if (BWAPI::Broodwar->self()->isUpgrading(bogi.metaType.upgradeType))
		{
			++count;
		}

		if (count < bogi.count)
		{
			return false;
		}
	}

	return true;
}

BuildOrderGoalManager::BuildOrderGoalManager(const BOGIVector & items)
{
	BOOST_FOREACH(const BuildOrderGoalItem & item, items)
	{
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

void BuildOrderGoalManager::getBuildOrder(BuildOrder & buildOrder)
{
	// Determine supply provider
	BWAPI::UnitType supplyType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();
	int supplyPerProvider = supplyType.supplyProvided();

	// Determine future supply
	int supplyRemaining = BWAPI::Broodwar->self()->supplyTotal();
	supplyRemaining += BuildingManager::Instance().buildingCount(supplyType) * supplyPerProvider;
	supplyRemaining += BuildingManager::Instance().buildingCount(BWAPI::UnitTypes::Terran_Command_Center) * BWAPI::UnitTypes::Terran_Command_Center.supplyProvided();

	int supplyUsed = BWAPI::Broodwar->self()->supplyUsed();
	supplyRemaining -= supplyUsed;

	BOOST_FOREACH(BuildOrderGoal & bog, goals)
	{
		bool complete = false;

		// keep repeating to queue items of same priority in a round-robin fashion
		while (!complete) {
			complete = true;

			BOOST_FOREACH(BuildOrderGoalItem & bogi, bog.items)
			{
				// don't queue beyond max supply
				if (bogi.metaType.isUnit() && supplyUsed + bogi.metaType.unitType.supplyRequired() > 400)
				{
					continue;
				}

				// this item hasn't been completed, so add one to the build order
				if (!isCompleted(bogi, buildOrder))
				{
					buildOrder.push_back(std::pair<MetaType, bool>(bogi.metaType, bogi.blocking));

					// account for projected supply amount
					supplyRemaining -= bogi.metaType.supplyRequired();
					if (bogi.metaType.isUnit()) supplyRemaining += bogi.metaType.unitType.supplyProvided();
					supplyUsed += bogi.metaType.supplyRequired();

					// build more supply providers
					if (supplyRemaining < supplyPerProvider)
					{
						buildOrder.push_back(std::pair<MetaType, bool>(supplyType, false));
						supplyRemaining += supplyPerProvider;
					}

					complete = false;
				}
				else
				{
					// mark as complete
					bogi.count = 0;
				}
			}
		}
	}
}