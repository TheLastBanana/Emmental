#pragma once

#include "Common.h"

class SquadOrder
{
public:

	enum { None, Attack, Defend, Regroup, Harass, SquadOrderTypes };

	int					type;
	BWAPI::Position		position;
	int					radius;
	std::string			status;

	SquadOrder() 
		: type(None) 
	{
	}

	SquadOrder(int type, BWAPI::Position position, int radius, std::string stat = "Default") 
		: type(type)
		, position(position)
		, radius(radius) 
		, status(stat)
	{
	}

	std::string getStatus() const 
	{
		return status;
	}
};
