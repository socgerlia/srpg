import copy
import inspect
from meta import Meta
from index import Index

class Type(Meta):
	@property
	def name(self): # abstract
		assert False
	def __neg__(self):
		return Optional(self)
	def __pos__(self):
		return Vector(self)
	# def __invert__(self):
	# 	return ?

class Bool(Type):
	@property
	def name(self): return "bool"
class Int(Type):
	@property
	def name(self): return "int"
class Float(Type):
	@property
	def name(self): return "float"

class Enum(Type):
	def __init__(self, enumname):
		Type.__init__(self)
		self.enumname = enumname
	@property
	def name(self): return self.enumname

class String(Type):
	@property
	def name(self): return "string"
class ShortString(Type):
	@property
	def name(self): return "flyweight<string>"

class Template(Type):
	def __init__(self, templatename, *types):
		Type.__init__(self)
		self.templatename = templatename
		self.types = types
	@property
	def name(self):
		return "{0}<{1}>".format(self.templatename, ", ".join(t.name for t in self.types))
class Tuple(Template):
	def __init__(self, *types):
		assert len(types) > 0
		Template.__init__(self, "tuple", *types)
class Vector(Template):
	def __init__(self, t):
		Template.__init__(self, "vector", t)
class Optional(Template):
	def __init__(self, t):
		Template.__init__(self, "optional", t)

class Map(Template):
	def __init__(self, k, v):
		Template.__init__(self, "map", k, v)
class UnorderedMap(Template):
	def __init__(self, k, v):
		Template.__init__(self, "unordered_map", k, v)
class MultiMap(Template):
	def __init__(self, k, v):
		Template.__init__(self, "multi_map", k, v)
class UnorderedMultiMap(Template):
	def __init__(self, k, v):
		Template.__init__(self, "unordered_multi_map", k, v)

bool = Bool().set_readonly()
int = Int().set_readonly()
float = Float().set_readonly()
enum = Enum

string = String().set_readonly()
ss = ShortString().set_readonly()
text = string

tu = Tuple
vector = Vector
optional = Optional

map = Map
unordered_map = UnorderedMap
multi_map = MultiMap
unordered_multi_map = UnorderedMultiMap

class Field:
	def __init__(self, name, type, *props):
		self.name = name
		self.type = type
		self.props = props
	def get_props(self, pred=None):
		if not pred:
			return self.type.props
		if inspect.isclass(pred):
			Class = pred
			pred = lambda prop: isinstance(prop, Class)
		return filter(pred, self.type.props)
	def get_self_prop_pairs(self, pred=None):
		return [(self, prop) for prop in self.get_props(pred)]

def dm(name, type, *args):
	print args
import sys
class Struct(Type):
	def __init__(self, name, *fields):
		self.name = name
		self.fields = [Field(*v) for v in fields]
	# def __repr__(self):
	# 	return "Struct({0}, {1})".format(self.name, ", ".join(field.__repr__() for field in fields))

	def indent(self, n, prefix="", suffix=""):
		return prefix + " ".repeat(n) + suffix
	def field_decls(self):
		return "\n  ".join("{0} {1};".format(field.type.name, field.name) for field in self.fields)
	def serialize_exprs(self):
		return "ar & " + " & ".join(field.name for field in self.fields) + ";"

class Dumper:
	def __init__(self, struct, file):
		self.struct = struct
		self.file = file
		self.has_dump = False

	def write(self, s):
		self.file.write(s)
	def newline_if(self):
		pass
		# if self.has_dump:
		# 	self.file.write("\n")
		# else:
		# 	self.has_dump = True

	def fields(self):
		self.newline_if()
		tpl = """
  {field_decls}
"""
		self.write(tpl.format(field_decls = self.struct.field_decls()))
	def serialize_func(self):
		self.newline_if()
		tpl = """
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version){{
    {serialize_exprs}
  }}
"""
		self.write(tpl.format(serialize_exprs = self.struct.serialize_exprs()))
	def tpl_related(self):
		struct_name = self.struct.name
		indice = reduce(lambda a, b: a+b, (field.get_self_prop_pairs(Index) for field in self.struct.fields), [])
		index_tags = "\n  ".join("struct i_{0}{{}};".format(field.name) for field, index in indice)
		index_decls = ",\n      ".join(index.decl(self.struct, field) for field, index in indice)
		def index_decl(self, struct, field):
			return "{0}<tag<i_{1}>, member<{2}, {3}, &{2}::{1}>>".format(self.name, field.name, struct.name, field.type.name)
		tpl = """
  {index_tags}
  typedef multi_index_container<
    {struct_name},
    indexed_by<
      {index_decls}
    >
  > map_type;

  template<class Tag> static auto get(){{ return data.get<Tag>(); }}
  static auto get(){{ return data.get<0>(); }}

  map_type data;
  static const char name[] = "{struct_name}";
"""
		self.write(tpl.format(**locals()))

	def begin(self):
		self.write("struct {0}{{".format(self.struct.name))
		self.has_dump = False
	def end(self):
		self.write("};")
		self.has_dump = False

	def dump_min(self):
		self.begin()
		self.fields()
		self.end()

	def dump_serialize(self):
		self.begin()
		self.fields()
		self.serialize_func()
		self.end()

	def dump_tpl(self):
		self.begin()
		self.fields()
		self.serialize_func()
		self.tpl_related()
		self.end()

struct = Struct
