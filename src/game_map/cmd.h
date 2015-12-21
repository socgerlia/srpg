#pragma once

#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>

#define _RR_SEQ_WRAP(seq) BOOST_PP_CAT(_RR_SEQ_WRAP_REC_0 seq, _END)
#define _RR_SEQ_WRAP_REC_0(...) ((__VA_ARGS__))_RR_SEQ_WRAP_REC_1
#define _RR_SEQ_WRAP_REC_1(...) ((__VA_ARGS__))_RR_SEQ_WRAP_REC_0
#define _RR_SEQ_WRAP_REC_0_END
#define _RR_SEQ_WRAP_REC_1_END

#define _RR_DF_FIELD_DECL(type, name) type name;
#define _RR_DF_FIELD_DECL_IT(r, data, i, elm) _RR_DF_FIELD_DECL elm

#define _RR_DF_SERIALIZE_EXPR(type, name) ar & boost::serialization::make_nvp(#name, name);
#define _RR_DF_SERIALIZE_EXPR_IT(r, data, i, elm) _RR_DF_SERIALIZE_EXPR elm

#define _RR_DEFINE_SERIALIZABLE_FIELDS(decls) \
	friend boost::serialization::access; \
	BOOST_PP_SEQ_FOR_EACH_I(_RR_DF_FIELD_DECL_IT, _, decls)\
	template<class Archive>\
	void serialize(Archive& ar, const unsigned int version){\
		BOOST_PP_SEQ_FOR_EACH_I(_RR_DF_SERIALIZE_EXPR_IT, _, decls)\
	}

#define RR_DEFINE_SERIALIZABLE_FIELDS(decls) _RR_DEFINE_SERIALIZABLE_FIELDS(_RR_SEQ_WRAP(decls))

namespace srpg{

class cmd{
	RR_DEFINE_SERIALIZABLE_FIELDS()
public:
	virtual ~cmd(){}
	virtual void redo() = 0;
	virtual void undo() = 0;
};

class cmd_add_group : public cmd{
	RR_DEFINE_SERIALIZABLE_FIELDS(
		(int, id))
public:
	cmd_add_group(){}
	cmd_add_group(int i) : id(i){}
	~cmd_add_group() override {}
	void redo() override{};
	void undo() override{};
};

class cmd_add_vertex : public cmd{
	RR_DEFINE_SERIALIZABLE_FIELDS(
		(int, id)
		(int, group))

public:
	cmd_add_vertex(){};
	cmd_add_vertex(int i, int g) : id(i), group(g){}
	~cmd_add_vertex() override{}
	void redo() override{};
	void undo() override{};
};

}

// BOOST_CLASS_EXPORT(srpg::cmd)
// BOOST_CLASS_EXPORT(srpg::cmd_add_group)
// BOOST_CLASS_EXPORT(srpg::cmd_add_vertex)

