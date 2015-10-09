#pragma once

struct basic_attr{
	int hp;
	int mp;

	int atk;
	int def;
	int mat;
	int mdef;
	int spd;
	int hit;
	int luck;

	int step;
	int stepu;
	int stepd;
	int wt;
};

struct battle_attr{
	int color;
	vertex* vert;
	int delay;
	int facing;
	int standby_type_id;
};
