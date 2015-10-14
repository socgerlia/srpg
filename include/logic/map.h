#pragma once

#include "vec.h"
#include "vertex.h"

namespace srpg{

class map{
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
	map();

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
