#pragma once

#include "Common.h"

struct MetaType {

	enum type_enum {Unit, Tech, Upgrade, Command, Default};
	type_enum type;

	BWAPI::UnitCommandType commandType;
	BWAPI::UnitType unitType;
	BWAPI::TechType techType;
	BWAPI::UpgradeType upgradeType;

	MetaType () : type(MetaType::Default) {}
	MetaType (BWAPI::UnitType t) :        unitType(t),    type(MetaType::Unit) {}
	MetaType (BWAPI::TechType t) :        techType(t),    type(MetaType::Tech) {}
	MetaType (BWAPI::UpgradeType t) :     upgradeType(t), type(MetaType::Upgrade) {}
	MetaType (BWAPI::UnitCommandType t) : commandType(t), type(MetaType::Command) {}

	bool isUnit() const		{ return type == Unit; }
	bool isTech() const		{ return type == Tech; }
	bool isUpgrade() const	{ return type == Upgrade; }
	bool isCommand() const	{ return type == Command; }
	bool isBuilding() const	{ return type == Unit && unitType.isBuilding(); }
	bool isRefinery() const	{ return isBuilding() && unitType.isRefinery(); }

	bool operator==(const MetaType & other) const
	{
		if (type != other.type) return false;

		switch (type)
		{
		case Unit:
			return unitType == other.unitType;
			break;

		case Tech:
			return techType == other.techType;
			break;

		case Upgrade:
			return upgradeType == other.upgradeType;
			break;

		case Command:
			return commandType == other.commandType;
			break;

		case Default:
			return true;
			break;

		default:
			return false;
		}
	}

	int supplyRequired()
	{
		if (isUnit())
		{
			return unitType.supplyRequired();
		}
		else
		{
			return 0;
		}
	}

	int mineralPrice()
	{
		return isUnit() ? unitType.mineralPrice() : (isTech() ? techType.mineralPrice() : upgradeType.mineralPrice());
	}

	int gasPrice()
	{
		return isUnit() ? unitType.gasPrice() : (isTech() ? techType.gasPrice() : upgradeType.gasPrice());
	}

	BWAPI::UnitType whatBuilds()
	{
		return isUnit() ? unitType.whatBuilds().first : (isTech() ? techType.whatResearches() : upgradeType.whatUpgrades());
	}

	std::string getName()
	{
		if (isUnit())
		{
			return unitType.getName();
		}
		else if (isTech())
		{
			return techType.getName();
		}
		else if (isUpgrade())
		{
			return upgradeType.getName();
		}
		else if (isCommand())
		{
			return commandType.getName();
		}
		else
		{
			assert(false);
			return "LOL";	
		}
	}
};