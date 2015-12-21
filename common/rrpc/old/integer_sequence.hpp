#pragma once

#include <cstddef>

// declare
template<class T, T... Elms>
struct seq;

namespace detail{
	template<class Seq>
	struct seq_pop_front;
	template<class T, T elm, T... Elms>
	struct seq_pop_front<seq<T, elm, Elms...>>{
		using type = seq<T, Elms...>;
	};

	template<class Seq, size_t dist>
	struct seq_shift_left{
		using type = typename seq_shift_left<
			typename seq_shift_left<Seq, 1>::type,
			(dist%Seq::size) - 1
		>::type;
	};
	template<class T, T elm, T... Elms>
	struct seq_shift_left<seq<T, elm, Elms...>, 1>{
		using type = seq<T, Elms..., elm>;
	};
	template<class Seq>
	struct seq_shift_left<Seq, 0>{
		using type = Seq;
	};

	template<class T, size_t n, T v, T... elms>
	struct select_value : select_value<T, n-1, elms...>{
	};
	template<class T, T v, T... elms>
	struct select_value<T, 0, v, elms...>{
		static const T value = v;
	};
} // end namespace detail

template<class T, T... Elms>
struct seq{
	using this_type = seq;
	using value_type = T;
	static const size_t size = sizeof...(Elms);

	template<size_t n>
	struct at : std::integral_constant<T,
		detail::select_value<T, n, Elms...>::value
	>{};

	using front = at<0>;
	using back = at<size-1>;

	using pop_front = detail::seq_pop_front<this_type>;

	template<T value>
	struct push_front{
		using type = seq<T, value, Elms...>;
	};

	template<T... Values>
	struct push_back{
		using type = seq<T, Elms..., Values...>;
	};

	template<size_t n, T begin, T step=1>
	struct push_ladder : push_back<begin>::type::template push_ladder<
		n-1,
		begin + step,
		step
	>{};
	template<T begin, T step>
	struct push_ladder<0, begin, step>{
		using type = this_type;
	};

	template<class Rhs>
	struct add;
	template<T... RhsElms>
	struct add<seq<T, RhsElms...>>{
		static_assert(sizeof...(RhsElms) == size, "size mismatch");
		using type = seq<T, (Elms + RhsElms)...>;
	};

	struct partial_sum{
		template<class Out, class In, size_t cnt>
		struct detail{
			using type = typename detail<
				typename Out::template push_back<Out::back::value + In::front::value>::type,
				typename In::pop_front::type,
				cnt-1
			>::type;
		};
		template<class Out, class In>
		struct detail<Out, In, 0>{
			using type = Out;
		};

		using type = typename detail<
			seq<T, front::value>,
			typename pop_front::type,
			size - 1
		>::type;
	};

	template<size_t dist>
	using shift_left = detail::seq_shift_left<this_type, dist>;

	template<size_t dist>
	using shift_right = shift_left<size - (dist%size)>;

	template<int dist>
	struct shift : std::conditional< (dist > 0),
		shift_right<size_t(dist)>,
		shift_left<size_t(-dist)>
	>::type{};
	// 	using type = typename std::conditional< (dist > 0),
	// 		shift_right<size_t(dist)>,
	// 		shift_left<size_t(-dist)>
	// 	>::type;
	// };
};

/*
namespace seq_operator{
	template<class Seq, size_t n, value_type_t<Seq> value>
	struct push_front_n{
		using type = typename push_front_n<
			typename push_front<Seq, value>::type,
			n-1,
			value
		>::type;
	};
	template<class Seq, value_type_t<Seq> begin>
	struct push_front_n<Seq, 0, begin>{
		using type = Seq;
	};

	template<class Seq, size_t n, value_type_t<Seq> value>
	struct push_back_n{
		using type = typename push_back_n<
			typename push_back<Seq, value>::type,
			n-1,
			value
		>::type;
	};
	template<class Seq, value_type_t<Seq> begin>
	struct push_back_n<Seq, 0, begin>{
		using type = Seq;
	};

	template<class Seq, value_type_t<Seq> value>
	struct add_n{
		using type = typename add<
			Seq,
			typename push_back_n<seq<value_type_t<Seq>>, Seq::size, value>::type
		>::type;
	};

	template<class Seq, class Tester, class... Args>
	struct seq_test_push_back;
	template<int... Elms, class Tester>
	struct seq_test_push_back<seq<Elms...>, Tester>{
		using type = seq<Elms...>;
	};
	template<int... Elms, class Tester, class T, class... Args>
	struct seq_test_push_back<seq<Elms...>, Tester, T, Args...>{
		static const int value = Tester::template test<T>::value;
		using type = typename seq_test_push_back<seq<Elms..., value>, Tester, Args...>::type;
	};
} // end namespace seq_operator
//*/

// C++14
namespace std{

template<class T, T... Elms>
using integer_sequence = seq<T, Elms...>;

template<size_t... Elms>
using index_sequence = seq<size_t, Elms...>;

template<class T, T N>
using make_integer_sequence = typename seq<T>::template push_ladder<N, 0>::type;
template<size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;

template<class... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

} // end namespace std
