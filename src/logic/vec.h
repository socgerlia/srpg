#pragma once

namespace srpg{

template<size_t N, class T>
struct vec;

template<class T>
struct vec<1, T>{
	union{
		T data[1];
		struct{
			T x;
		};
		struct{
			T width;
		};
	};
};

template<class T>
struct vec<2, T>{
	union{
		T data[2];
		struct{
			T x;
			T y;
		};
		struct{
			T width;
			T height;
		};
	};
};

} // end namespace srpg
