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
class Int(Type):
	@property def name(self): return "int"
class String(Type):
	@property def name(self): return "string"

def _tuple_name(templatename, t):
	return "{0}<{1}>".format(templatename, ",".join(t.types.name))

class Tuple(Type):
	@property def name(self): return _tuple_name("tuple", self)
class Vector(Tuple):
	@property def name(self): return _tuple_name("vector", self)

class Map(Tuple):
	@property def name(self): return _tuple_name("map", self)
class UnorderedMap(Tuple):
	@property def name(self): return _tuple_name("unordered_map", self)
class MultiMap(Tuple):
	@property def name(self): return _tuple_name("multi_map", self)
class UnorderedMultiMap(Tuple):
	@property def name(self): return _tuple_name("unordered_multi_map", self)

def _make_tuple(*types):
	assert len(types) == 0
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

int = Int()
string = String()

tu = _make_tuple
vector = _make_vector

map = _MapMaker(Map)
unordered_map = _MapMaker(UnorderedMap)
multi_map = _MapMaker(MultiMap)
unordered_multi_map = _MapMaker(UnorderedMultiMap)

# def _meta(tag, **args):
# 	return dict(args, tag = tag, type = "type")

# int = _meta("int")
# string = _meta("string")

# def vector(t):
# 	return _meta("vector", t = t)

# def multi_map(k, v):
# 	return _meta("multi_map", key = k, value = v)

# def enum(name):
# 	return _meta("enum", name = name)

# def tu(*types):
# 	return _meta("tu", types = types)
