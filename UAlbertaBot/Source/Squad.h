#pragma once

#include "Common.h"
#include "micromanagement/RangedManager.h"
#include "micromanagement/VultureManager.h"
#include "micromanagement/TankManager.h"
#include "micromanagement/MeleeManager.h"
#include "SquadOrder.h"
#include "DistanceMap.hpp"
#include "StrategyManager.h"
#include "CombatSimulation.h"
#include "BunkerManager.h"

//class VultureManager;

class MeleeManager;
class RangedManager;
class VultureManager;
class TankManager;

class Squad
{
	UnitVector			units;
	std::string			regroupStatus;
	int					lastFrameRegroup;
	bool				squadObserverNear(BWAPI::Position p);
	
	SquadOrder			order;
	RangedManager		rangedManager;
	VultureManager		vultureManager;
	MeleeManager		meleeManager;
	TankManager			tankManager;

	std::map<BWAPI::Unit *, bool>	nearEnemy;

	void				updateUnits();
	void				setManagerUnits();
	void				setNearEnemyUnits();
	void				setAllUnits();
	
	void				setUnits(const UnitVector & u)	{ units = u; }
	
	bool				unitNearEnemy(BWAPI::Unit * unit);
	bool				needsToRegroup(const UnitVector & u, int unitsNeeded = 5);
	BWAPI::Unit *		getRegroupUnit();
	int					squadUnitsNear(BWAPI::Position p);

	BWAPI::Unit *		unitClosestToEnemy();

    static int          lastRetreatSwitch;
    static bool         lastRetreatSwitchVal;

public:


	Squad(const UnitVector & units, SquadOrder order);
	Squad() {}
	~Squad() {}

	BWAPI::Position		calcCenter();
	BWAPI::Position		calcRegroupPosition(const UnitVector & u);

	void				update();

	const UnitVector &	getUnits() const;
	const SquadOrder &	getSquadOrder()	const;

	void				setSquadOrder(const SquadOrder & so);
};
