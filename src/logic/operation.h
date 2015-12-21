
template<class T>
struct list_hook{
	T* next;
	T* prev;
};

namespace srpg{

template<class T, class Archive, class U>
inline std:enable_if<Archive::load_tag>::type new_and_serialize(Archive& ar, U*& p, const unsigned int version){
	T* ret = new T();
	p = ret;
	ret->serialize(ar, version);
}
template<class T, class Archive, class U>
inline std:enable_if<Archive::save_tag>::type new_and_serialize(Archive& ar, U*& p, const unsigned int version){
	static_cast<T*>(p)->serialize(ar, version);
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

class operation{
protected:
	unsigned int ref_cnt_;
	int type;
public:
	enum Type{
		op_turn_end,
		op_move,
		op_change_facing,
	};

public:
	virtual ~operation(){};
	int get_type() const { return type; }
};
struct op_turn_end : operation{
	op_turn_end() : type(Type::op_turn_end){}
};
struct op_move : operation{
	op_move() : type(Type::op_move){}
	RR_DEFINE_SERIALIZABLE_FIELDS(
		(vector<int>, path))
};
struct op_change_facing : operation{
	op_change_facing() : type(Type::op_change_facing){}
	RR_DEFINE_SERIALIZABLE_FIELDS(
		(int, value))
};

} // end namespace srpg
