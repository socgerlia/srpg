
struct test{
	int a;
	string b;
	vector<float> c;

	struct field_index<0>{
		static const int index = 0;
		using type = int;
		template<class T> static type& get(T& self){
			return self.a;
		}
	};
	static const int max_field_index = 0;
};

template<class T, class Enable = void> struct has_field_index : std::false_type{};
template<class T, int id> struct has_field_index<T,
	std::void_t<typename T::field_index<id>>
> : std::true_type{};

template<class Archive, class T>
std::enable_if<has_field_index<T>::value, Archive&> serialize(Archive& ar, T& v){
}

