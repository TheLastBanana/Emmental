==============
= HOW TO USE =
==============
Import UAlbertaBot.sln into Visual Studio and compile as per the UAlbertaBot instructions:
https://code.google.com/p/ualbertabot/wiki/Instructions

===========
= CHANGES =
===========
StarcraftBuildOrderSearch/Source/starcraftsearch/StarcraftData.hpp
    - addActions(): Added Terran units to the search actions list

UAlbertaBot/Source/BunkerManager.cpp
UAlbertaBot/Source/BunkerManager.h
    - BunkerManager: A new class added by us to manage bunker placement

UAlbertaBot/Source/CombatCommander.cpp
UAlbertaBot/Source/CombatCommander.h
    - lowerID(): A new function to compare units IDs (for STL sorts)
    - assignHarass(): Added an (ultimately unused) "harass" order
    - assignScoutDefenseSquads(): Fixed a null pointer reference bug
    - assignDefenseSquads(): Added logic for bunker-defending tanks

UAlbertaBot/Source/GameCommander.cpp
UAlbertaBot/Source/GameCommander.h
    - onUnitCreate(), onUnitDestroy(), update(): Added hooks for BunkerManager

UAlbertaBot/Source/InformationManager.cpp
    - isCombatUnit(): Designated more types as combat units

UAlbertaBot/Source/MapGrid.h
UAlbertaBot/Source/MapGrid.cpp
    - GetUnitsWithInvisible(): A new function to get units including invisible ones

UAlbertaBot/Source/MapTools.cpp
UAlbertaBot/Source/MapTools.h
    - getClosestChokepoint(): A new function to get nearby chokepoints
    - drawChokepoints(): A new function to draw all checkpoints

UAlbertaBot/Source/Options.cpp
UAlbertaBot/Source/Options.h
    - DRAW_UALBERTABOT_SEARCHINFO: A new variable which controls whether search info is shown
    - Changed constant to disable all drawing for release version

UAlbertaBot/Source/Squad.cpp
UAlbertaBot/Source/Squad.h
    - setManagerUnits(): Set marines to defend bunker while it's being built, then enter it once completed
    - setManagerUnits(), update(), setNearEnemyUnits(): Seperated each unit type into different squads so they can have different orders
    - needsToRegroup(): Modified to add an ability for each squad to regroup under different conditions

UAlbertaBot/Source/SquadOrder.h
    - SquadOrder enum: Added an (ultimately unused) "harass" order

UAlbertaBot/Source/StrategyManager.cpp
UAlbertaBot/Source/StrategyManager.h
    - addStrategies(), getTerranBuildOrderGoal(): Changed to use our "TerranVultureRush" strategy
    - getBuildOrderGoal(), getTerranBuildOrderGoal(): Changed Terran strategy to use our custom build order sorter

UAlbertaBot/Source/UAlbertaBotModule.cpp
    - onFrame(), onStart(): Minor debug settings

UAlbertaBot/Source/base/BuildOrderGoalManager.cpp
UAlbertaBot/Source/base/BuildOrderGoalManager.h
    - BuildOrderGoalManager, BuildOrderGoal, BuildOrderGoalItem: Implementation of our custom build order sort

UAlbertaBot/Source/base/BuildOrderQueue.cpp
UAlbertaBot/Source/base/BuildOrderQueue.h
    - isCompleted(): Fixed a build order duplication bug

UAlbertaBot/Source/base/BuildingData.cpp
UAlbertaBot/Source/base/BuildingData.h
    - buildingCount(): A new function to count the number of buildings in progress/planned

UAlbertaBot/Source/base/BuildingManager.cpp
UAlbertaBot/Source/base/BuildingManager.h
    - buildingCount(): A new function to count the number of buildings in progress/planned
    - getBuildingLocation(): Added special code to place bunkers
    - getBuildingLocationNear(): Modified to accept TilePosition as argument
    - assignWorkersToUnassignedBuildings(): Fixed a bug which only partially accounted for Terran addon space

UAlbertaBot/Source/base/BuildingPlacer.cpp
UAlbertaBot/Source/base/BuildingPlacer.h
    - getBuildingLocationNear(): Added function to place buildings
    - getPointClosestTo(): A new function to find closest point along line to a given point on which we can build

UAlbertaBot/Source/base/MetaType.h
    - Properly labelled const functions as such
    - operator==(): Added MetaType comparison

UAlbertaBot/Source/base/ProductionManager.cpp
UAlbertaBot/Source/base/ProductionManager.h
    - setBuildOrder(), performBuildOrderSearch(): Overloaded versions which use our custom build order sort

UAlbertaBot/Source/base/StarcraftSearchData.h
    - drawSearchResults(): Added flag to disable search info drawing

UAlbertaBot/Source/base/WorkerData.cpp
UAlbertaBot/Source/base/WorkerData.h
    - getWorkerRepairUnitBuilding(): A new function to get the building being repaired by the "slave" worker
    - setWorkerJob(), clearPreviousJobs(): Made "slave" worker flee when attacked

UAlbertaBot/Source/base/WorkerManager.cpp
UAlbertaBot/Source/base/WorkerManager.h
    - updateWorkerStatus(): Added SCV bunker slave

UAlbertaBot/Source/micromanagement/MicroManager.cpp
UAlbertaBot/Source/micromanagement/MicroManager.h
    - smartMove(), smartAttackMove(): Added option to force movement even if it cancels a previous command

UAlbertaBot/Source/micromanagement/RangedManager.cpp
UAlbertaBot/Source/micromanagement/RangedManager.h
    - kiteTarget(): Modified to allow Vulture to imrpove kiting and allow use of mines against certain enemy types, as well as allowing air units to behave properly
    - getEffectiveRange(): New function which takes a unit's movement speed into account when calculating its range
    - checkFleePosition(): New function which checks if a position is safe to flee to

UAlbertaBot/Source/micromanagement/TankManager.cpp
UAlbertaBot/Source/micromanagement/TankManager.h
    - TankManager: New manager class to handle Siege Tank micro (detailed in report)

UAlbertaBot/Source/micromanagement/VultureManager.cpp
UAlbertaBot/Source/micromanagement/VultureManager.h
    - VultureManager: New manager class to handle Vulture micro (detailed in report)