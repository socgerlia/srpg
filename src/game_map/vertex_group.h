#pragma once

#include "logic/vec.h"

namespace srpg{
typedef vec<2, int> index2;

struct vertex_data{
	int id;
	index2 index;

	int top;
	int bottom;
	int terrain_id;
};

struct map_node{
	int id;
	map_node* parent;
	std::vector<map_node*> children;

	game_map_data* get_root() const;
	virtual ~map_node(){}
};

struct vertex_group : map_node{
};

struct game_map_data : map_node{
	std::vector<vertex_group> groups;

	std::unorder_map<int, vertex_data> vertices;
	boost::multi_array<vertex_data*, 2> vertices_by_index;

	vertex_group* get_group(const std::string& name) const;
	vertex_group* add_group();

	vertex_data* get_vertex(index2 index) const;
	vertex_data* add_vertex(index2 index);
	bool remove_vertex(index2 index);

	index2 get_base() const;
	index2 get_dimension() const;
	void rebase(index2 index);
};


}