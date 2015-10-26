#include "game_table.h"

namespace srpg{

void game_table::game_loop(){
// 	on_game_create();
// 	for(auto pair : players){
// 		players->on_game_prepare();
// 	}

// 	while(true){
// 		auto cur_entity = entities_.get<i_next_turn>().front();
// 		if(cur_entity->next_turn == time_bar->turn){
// 			while(true){
// 				operation* op = cur_entity->on_think();
// 				apply_operation(cur_entity, op);
// 				if(check_game_over()){
// 					goto __game_over__;
// 				}
// 				else if(cur_entity->is_turn_over()){
// 					break;
// 				}
// 			}
// 		}
// 		else{
// 			time_bar->next_time();
// 		}
// 	}
// __game_over__:
}

}
