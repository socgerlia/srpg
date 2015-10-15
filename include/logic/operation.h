
template<class T>
struct list_hook{
	T* next;
	T* prev;
};

namespace srpg{

class Operation{
public:

protected:
	list_hook<Operation> _hook;
	size_t _type;

	Operation() : _type(0);
};

struct operation{
	int type;
};

template<class T, class Archive>
inline T* new_and_serialize(Archive& ar, const unsigned int version){
	auto ret = new T();
	ret->serialize(ar, version);
	return ret;
}

template<class Archive>
void serialize(Archive& ar, operation*& op, const unsigned int version){
	int type;
	ar & type;
	switch(type){
	case op_move: op = new_and_serialize<op_move>(ar, version); break;
	}
	op->type = type;
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
