import copy

class Type:
	def __init__(self):
		self.constraints = []
		self.has_set_constraints = False
	def __call__(self, *args):
		if self.has_set_constraints:
			raise
		ret = copy.deepcopy(self)
		ret.has_set_constraints = True
		ret.constraints = args
		return ret

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
	@property
	def name(self): return self.enumname

class String(Type):
	@property
	def name(self): return "flyweight<string>"
class Text(Type):
	@property
	def name(self): return "string"

def _tuple_name(templatename, tu):
	return "{0}<{1}>".format(templatename, ", ".join(t.name for t in tu.types))

class Tuple(Type):
	@property
	def name(self): return _tuple_name("tuple", self)
class Vector(Tuple):
	@property
	def name(self): return _tuple_name("vector", self)

class Map(Tuple):
	@property
	def name(self): return _tuple_name("map", self)
class UnorderedMap(Tuple):
	@property
	def name(self): return _tuple_name("unordered_map", self)
class MultiMap(Tuple):
	@property
	def name(self): return _tuple_name("multi_map", self)
class UnorderedMultiMap(Tuple):
	@property
	def name(self): return _tuple_name("unordered_multi_map", self)

def _make_enum(enumname):
	ret = Enum()
	ret.enumname = enumname
	return ret
def _make_tuple(*types):
	assert len(types) > 0
	ret = Tuple()
	ret.types = types
	return ret
def _make_vector(t):
	ret = Vector()
	ret.types = [t]
	return ret
class _MapMaker:
	def __init__(self, Class):
		self._class = Class
	def __call__(self, k, v):
		ret = self._class()
		ret.types = [k, v]
		return ret

bool = Bool()
int = Int()
float = Float()
enum = _make_enum

string = String()
text = Text()

tu = _make_tuple
vector = _make_vector

map = _MapMaker(Map)
unordered_map = _MapMaker(UnorderedMap)
multi_map = _MapMaker(MultiMap)
unordered_multi_map = _MapMaker(UnorderedMultiMap)
