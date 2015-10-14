#pragma once

#include "basic_attr.hpp"

struct tpl_card{
	int id;
	int delay;
	int min_hp;
	int max_hp;
	int min_mp;
	int max_mp;
	int gene_id;

	vector<int> skills;
};
