#pragma once

#include "utils/using.h"

namespace srpg{

class game_table;
class unit_data;
class vertex;

class entity{
	friend game_table;

public:
	int id;
	game_table* gt;
	shared_ptr<unit_data> data;

	int type;
	int create_turn;
	int next_turn;
	int next_turn_eps;

	int color;
	vertex* pos;
	int facing;
	int standby_type_id;
};

}
