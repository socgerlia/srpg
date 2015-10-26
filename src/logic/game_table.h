#pragma once

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include "utils/using.h"

#include "logic/entity.h"

namespace srpg{

namespace mi = boost::multi_index;

class game_map;
class entity;
class player;
class history;
class status;
class operation;
class time_bar;

class game_table{
public:
	struct i_id{};
	struct i_next_turn{};
	typedef boost::multi_index_container<
		shared_ptr<entity>,
		mi::indexed_by<
			mi::hashed_non_unique<mi::tag<i_id>, mi::member<entity, int, &entity::id>>, // id
			mi::ordered_non_unique< // next_turn
				mi::tag<i_next_turn>,
				mi::composite_key<
					entity,
					mi::member<entity, int, &entity::next_turn>,
					mi::member<entity, int, &entity::next_turn_eps>
				>
			>
		>
	> entity_set;

protected:
	game_map* map_;
	entity_set entities_;
	std::map<int, player*> players;
	time_bar* time_;
	history* history_;
	int next_turn_eps_cnt = 0; // every action will plus 1, effect entity::next_turn_eps

public:
	void add_entity(entity* en);
	void remove_entity(entity* en);

	status* make_status();

	// TODO: time out
	void game_loop();
	void on_game_create(); // init players, map, enemies ...
	void apply_operation(entity* en, operation* op);
	bool check_game_over();

	// template<class Filter, class Comp>
	// void filter_sort_entities(vector<entity*>& container, Filter&& filter, Comp&& comp){
	// 	container.clear();
	// 	for(auto& p : entities_)
	// 		if(filter(p))
	// 			container.push_back(p.get());
	// 	sort(container.begin(), container.end(), comp);
	// }

	// bool dirty;
	// vector<entity*> entities_by_next_turn;
	// const vector<entity*>& get_entities_by_next_turn(){
	// 	if(dirty){
	// 		dirty = false;
	// 		filter_sort_entities(
	// 			entities_by_next_turn,
	// 			[](auto& p) -> bool{ return p->next_turn > 0; },
	// 			[](auto a, auto b) -> bool{
	// 				int d;
	// 				return (d = a.next_turn - b.next_turn) != 0 ? d < 0 :
	// 					a.next_turn_eps - b.next_turn_eps < 0;
	// 			});
	// 	}
	// }

};

}
