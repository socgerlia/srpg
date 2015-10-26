#pragma once

#include "vec.h"
#include "vertex.h"

namespace srpg{

struct vertex_data_tpl{
	int top;
	int bottom;
	int terrain_id;
};

struct vertex_data : vertex_data_tpl{
	int id;
	index2 index;
};

struct vertex_group_data_tpl{
	boost::multi_array<vertex_data, 2> data;
};

struct vertex_group_data : vertex_group_data_tpl{
	int id;
};

struct game_map_area{
	boost::multi_array<vertex_data, 2> data;
};

class game_map_data{
	typedef vec<2, int> index2;

	index2 max_dimension;
	std::map<int, game_map_area> areas;

	int add_area();
	void remove_area(int id);
};

class game_map{
public:
	typedef vec<2, int> index_type;
	typedef std::vector<vertex_type> data_type;

	enum Dir{
		FORE, BACK, LEFT, RIGHT, UP, DOWN
	};

protected:
	data_type data_;
	index_type dimension_;

public:
	game_map();

	// getter
	vertex_type& get_vertex(size_t uid){ return data_[uid]; }
	vertex_type& get_vertex(size_t x, size_t y){ return data_[x * size_.width + y]; }

	const index_type& get_dimension() const{ return dimension_; }
	data_type& get_data(){ return data_; }

	// modify
	vertex_type& add_vertex(const index_type& index, const vertex_type& value);
	void erase_vertex(size_t uid){}
	void swap_vertex(size_t uid_a, size_t uid_b){}
};

} // end namespace srpg
