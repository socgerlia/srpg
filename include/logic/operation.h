
template<class T>
struct list_hook{
	T* next;
	T* prev;
};

namespace srpg{

struct operation{
	int type;
};

template<class T, class Archive, class U>
inline std:enable_if<Archive::load_tag>::type new_and_serialize(Archive& ar, U*& p, const unsigned int version){
	T* ret = new T();
	p = ret;
	ret->serialize(ar, version);
}
template<class T, class Archive, class U>
inline std:enable_if<Archive::save_tag>::type new_and_serialize(Archive& ar, U*& p, const unsigned int version){
	reinterpret_cast<T*>(p)->serialize(ar, version);
}

template<class Archive>
void serialize(Archive& ar, operation*& op, const unsigned int version){
	int type;
	ar & type;
	switch(type){
	case op_move: new_and_serialize<op_move>(ar, op, version); break;
	case op_change_facing: new_and_serialize<op_change_facing>(ar, op, version); break;
	}
}

#define _RR_DF_FIELD_DECL(type, name) type name;
#define _RR_DF_FIELD_DECL_IT(r, data, i, elm) _RR_DF_FIELD_DECL elm

#define _RR_DF_SERIALIZE_EXPR(type, name) ar & name;
#define _RR_DF_SERIALIZE_EXPR_IT(r, data, i, elm) _RR_DF_SERIALIZE_EXPR elm

#define RR_DEFINE_SERIALIZABLE_FIELDS(decls) \
	BOOST_PP_SEQ_FOR_EACH_I(_RR_DF_FIELD_DECL_IT, _, decls)\
	template<class Archive>\
	void serialize(Archive& ar, const unsigned int version){\
		BOOST_PP_SEQ_FOR_EACH_I(_RR_DF_FIELD_DECL_IT, _, decls)\
	}

struct op_move : operation{
	RR_DEFINE_SERIALIZABLE_FIELDS(
		(int, entity_id)
		(vector<int>, path)
	)
	// TODO: create
};
struct op_change_facing : operation{
	int entity_id;
	int value;
};

} // end namespace srpg
