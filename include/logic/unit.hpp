#pragma once

struct unit{
	int id;
	tpl_unit& tpl_unit_;

	int level;
	int exp;
	basic_attr attr;

	int max_equips;
	vector<equip> equips;
	int max_cards;
	vector<card> cards;
};
