#if 0

#include "vertex_group.h"

namespace srpg{

game_map_data* map_node::get_root() const{
	auto p = this;
	while(p.parent) p = p.parent;
	return dynamic_cast<game_map_data*>(p);
}

// game_map_data
vertex_group* get_group(const std::string& name) const{
	auto it = std::find(groups.begin(), groups.end(), [](const vertex_group& item){ return item.name == name; });
	return it == groups.end() ? nullptr : &*it;
}
vertex_group* add_group(){
	groups.push_back();
	ret = &groups.back();
	ret->name = "New Group";
	return ret;
}

vertex_data* game_map_data::get_vertex(index2 index) const{
}
vertex_data* game_map_data::add_vertex(index2 index){
}
bool game_map_data::remove_vertex(index2 index){
}

index2 game_map_data::get_base() const{
}
index2 get_dimension() const{
}
void rebase(index2 index){
}


}

#endif
