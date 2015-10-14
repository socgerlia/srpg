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
	int jump;
	int wt;
};

struct tpl_unit{
	int id;
	string name;
	int race;
	int gender;
	int career;
	int moments;
	int gene_id;

	basic_attr attr;
	int grow_id;

	vector<int> equip_grow; // [level, ...]
	vector<int> card_grow; // [level, ...]
	multi_map<int, int> skill_grow; // [{level, skill_id}, ...]

	vector<int> weapon_compatibility; // [rank, ...]
};
