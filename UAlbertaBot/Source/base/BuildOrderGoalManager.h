#pragma once

#include "Common.h"
#include "MetaType.h"

typedef std::vector<std::pair<MetaType, bool> > BuildOrder;

struct BuildOrderGoalItem
{
	// the unit to build
	MetaType metaType;

	// the number to build
	int count;

	// the priority to build this (less than 0 means use default)
	int priority;

	// whether we should block other things from being constructed while this is building
	bool blocking;

	BuildOrderGoalItem(const MetaType & metaType, int count = 1, int priority = -1, bool blocking = true);
};

typedef std::vector<BuildOrderGoalItem> BOGIVector;

struct BuildOrderGoal
{
	BOGIVector items;

	int priority;

	BuildOrderGoal(int priority) : priority(priority) {}

	void addItem(const BuildOrderGoalItem & item)
	{
		items.push_back(item);
	}

	bool operator < (const BuildOrderGoal & bog)
	{
		return priority < bog.priority;
	}
};

class BuildOrderGoalManager {
	std::vector<BuildOrderGoal>	goals;

	// checks to see if a goal item is completed by using BWAPI data
	bool isCompleted(const BuildOrderGoalItem & bogi) const;

public:

	BuildOrderGoalManager(const BOGIVector & items);

	// gets a vector of the highest-priority build orders which haven't been completed
	void BuildOrderGoalManager::getBuildOrder(BuildOrder & buildOrder);
};
