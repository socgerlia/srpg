template<int N, class Int>
Int next_index(Int v){
	constexpr Int mask = ((1 << N) - 1);
	return (v + 1) & mask;
}

template<class Int>
Int next_index<sizeof(Int) * 8, Int>(Int v){
	return v + 1;
}

template<int bytes, bool is_signed>
struct integer_type;

template<> struct integer_type<1, true>{ using type = int8_t; };
template<> struct integer_type<2, true>{ using type = int16_t; };
template<> struct integer_type<4, true>{ using type = int32_t; };
template<> struct integer_type<8, true>{ using type = int64_t; };

template<> struct integer_type<1, false>{ using type = uint8_t; };
template<> struct integer_type<2, false>{ using type = uint16_t; };
template<> struct integer_type<4, false>{ using type = uint32_t; };
template<> struct integer_type<8, false>{ using type = uint64_t; };



template<class T, size_t max_size>
struct spsc_queue{
	static_assert(max_size & (max_size - 1), "not pow of 2");

	enum{
		size = max_size,
		max_index = size - 1,
		integer_bytes = (N == 0 ? 1 : N / 8)
	};

	using index_type = integer_type<integer_bytes, false>::type;
	using atomic_integer_type = integer_type<integer_bytes * 2, false>::type;

	union state_type{
		struct{
			index_type wp;
			index_type rp;
		};
		atomic_integer_type value;
	};

	boost::atomic<atomic_integer_type> state_;
	T* data_[max_size];

	bool push(T* v){
		state_type x;
		x.value = state_.load();

		if(next_index<N>(x.wp) == x.rp)
			return false;
		data[x.wp] = v;
 
		return x.wp == max_index ? state.fetch_sub(max_index) : state.fetch_add(1);
	}

	uint32_t inc_rp(state_type x){
		return x.rp == max_index ? state.fetch_sub(max_index) : state.fetch_add(1);
	}

	template<class Func>
	void consume_thread(state_type x, Func&& f){
		do{
			// TODO: multi copy

			// handling ...
			T& value = data[x.rp];
			bool delay = f(std::move(value));
			value.~T();
			if(delay)
				return;

			x.value = inc_rp();
		}while(x.rp != x.wp);
	}

	template<class Func>
	void after_consume(state_type x, Func&& f){
		x.value = inc_rp();
		if(x.rp != x.wp)
			consume_thread(x, std::forward<Func>(f));
	}
};

struct mpsc_queue{
	enum{
		max_count = 1 << 10,
		max_index = max_count - 1,
	};

	union state_type{
		struct{
			int wp:16;
			int rp:16;
		};
		uint32_t value;
	};
	atomic<uint32_t> state;
	atomic<uint16_t> bp;
	T* data[max_count];

	bool push(T* v){
		state_type x;
		x.value = state.load();
		if(x.wp + 1 = x.rp)
			return false;

		uint16_t wp = x.wp;
		while(!bp.compare_exchange(wp, wp + 1)); // ++bp

		data[x.wp] = v;
		x.value = state.fetch_add(1 << 16); // ++wp
		if(x.wp == x.rp + 1)
			consume_thread(x);
	}

	void consume_thread(state_type x){
		do{
			// TODO: multi copy
			T* v = data[x.rp];

			// handling ...

			if(x.rp == max_index)
				x.value = state.fetch_sub(max_index); // add rp
			else
				x.value = state.fetch_add(1); // add rp
		}while(x.rp == x.wp);
	}
};
