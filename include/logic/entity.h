#pragma once

struct entity{
	int id;
	map* map;
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
