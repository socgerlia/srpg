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

	time_bar time_;
	history history_;

	int next_turn_eps_cnt = 0; // every action will plus 1, effect entity::next_turn_eps

	void add_entity(entity* en);
	void remove_entity(entity* en);
};
