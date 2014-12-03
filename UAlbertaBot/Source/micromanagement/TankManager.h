#pragma once
#include "Common.h"
#include "micromanagement\RangedManager.h"
#include "../SquadOrder.h"

class RangedManager;

class TankManager :
	public RangedManager
{
public:
	TankManager();
	virtual ~TankManager() {}
	void kiteTarget(BWAPI::Unit * rangedUnit, const UnitVector & targets, BWAPI::Unit * target);
	void executeMicro(const UnitVector & targets);
};

