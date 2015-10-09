#pragma once

#include "vec.h"

namespace srpg{

struct map;

struct vertex{
public:
	typedef vec<2, int> index_type;

protected:
	map* map_;
	size_t id;
	int top;
	int bottom;
	index_type index;
	vertex* adjs[6];
	vector<unit*> units;

// public:1
// 	vertex();

// 	// getter
// 	const vertex_type& get_vertex(size_t uid) const{ return _data[uid]; }
// 	const vertex_type& get_vertex(size_t x, size_t y) const{ return _data[x * _size.width + y]; }

// 	const index_type& get_dimension() const{ return _dimension; }
// 	const data_type& get_data() const{ return _data; }

// 	// modify
// 	vertex* get_adjacent_vertex(size_t dir){ return adjacency[dir]; }
};

} // end namespace srpg
