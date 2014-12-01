#pragma once
#include "Common.h"
#include "micromanagement\RangedManager.h"
#include "../SquadOrder.h"

class RangedManager;

class VultureManager :
	public RangedManager
{
public:
	VultureManager();
	virtual ~VultureManager() {}
	void executeMicro(const UnitVector & targets);
};

