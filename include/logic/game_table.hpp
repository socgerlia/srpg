#pragma once

struct game_table{
	map map_;
	list<shared_ptr<entity>> entities_;

	struct i_id{};
	struct i_next_turn{};
	typedef multi_index_container<
		shared_ptr<entity>,
		indexed_by<
			unordered_non_unique<tag<i_id>, member<entity, int, &entity::id>> // id
			ordered_non_unique< // next_turn
				tag<i_next_turn>,
				composite_key<
					entity,
					member<entity, int, &entity::next_turn>,
					member<entity, int, &entity::next_turn_eps>
				>
			>
		>
	> entity_set;

	template<class Filter, class Comp>
	void filter_sort_entities(vector<entity*>& container, Filter&& filter, Comp&& comp){
		container.clear();
		for(auto& p : entities_)
			if(filter(p))
				container.push_back(p.get());
		sort(container.begin(), container.end(), comp);
	}

	time_bar time_;
	history history_;

	int next_turn_eps_cnt = 0; // every action will plus 1, effect entity::next_turn_eps

	void add_entity(entity* en);
	void remove_entity(entity* en);

	bool dirty;
	vector<entity*> entities_by_next_turn;
	const vector<entity*>& get_entities_by_next_turn(){
		if(dirty){
			dirty = false;
			filter_sort_entities(
				entities_by_next_turn,
				[](auto& p) -> bool{ return p->next_turn > 0; },
				[](auto a, auto b) -> bool{
					int d;
					return (d = a.next_turn - b.next_turn) != 0 ? d < 0 :
						a.next_turn_eps - b.next_turn_eps < 0;
				});
		}
	}

	void move_to(entity* en, const vector<int>& path){
		
	}
};
