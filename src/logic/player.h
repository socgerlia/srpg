#pragma once

class player{
	int id;
	int color;
	game_table* gt;

	virtual void on_game_prepare() = 0;
	virtual void set_battle_list(battle_list* list);

	virtual void on_think(status& s) = 0;
	virtual void set_operation(operation* op);
};
