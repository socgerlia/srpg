#pragma once

struct unit_data{
	int id;
	int unit_id;

	int level;
	int exp;
	basic_attr attr;

	int max_equips;
	vector<equip> equips;
	int max_cards;
	vector<card> cards;
};
