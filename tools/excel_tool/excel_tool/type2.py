import copy

def eat_blank(s, i):
	while i < len(s) and s[i].isspace():
		i += 1
	return i

class Type(object):
	def __init__(self):
		self.constraint = None
	# def __pos__(self):
	# 	return List(self)
	def __getitem__(self, c):
		ret = copy.copy(self)
		return ret._and_constraint(c)
	def _and_constraint(self, c):
		if self.constraint:
			self.constraint = AndConstraint(self.constraint, c)
		else:
			self.constraint = c
		return self
	def clone(self):
		ret = copy.copy(self)
		if self.constraint:
			ret.constraint = self.constraint.clone()
		return ret
	def check(self, ck):
		if self.constraint:
			return self.constraint.check(ck)
		else:
			return True

class BoolType(Type):
	def convert_toplevel(self, s):
		try:
			ret = int(s)
			if ret == 0:
				return False
			elif ret == 1:
				return True
			else:
				raise Exception("can't convert to bool type")
		except:
			raise
	def convert(self, s, i):
		i = eat_blank(s, i)
		if i >= len(s):
			raise Exception("Empty")
		if s[i] == '0':
			return False, i+1
		elif s[i] == '1':
			return True, i+1
		else:
			raise Exception("can't convert to bool type")

class IntType(Type):
	def __init__(self, length=32):
		Type.__init__(self)
		self.length = length
	def convert_toplevel(self, s):
		try:
			return int(float(s))
		except:
			raise Exception("can't convert to int type")
	def convert(self, s, i):
		i = eat_blank(s, i)
		if i >= len(s):
			raise Exception("Empty")
		b = i
		if s[i] == '-':
			i += 1
		while i < len(s) and s[i] in '0123456789.':
			i += 1
		try:
			return int(float(s[b:i])), i
		except:
			raise Exception("can't convert to int type")

class FloatType(Type):
	def convert_toplevel(self, s):
		try:
			return float(s)
		except:
			raise
	def convert(self, s, i):
		i = eat_blank(s, i)
		if i >= len(s):
			raise
		b = i
		if s[i] == '-':
			i += 1
		while i < len(s) and s[i] in '0123456789.':
			i += 1
		try:
			return float(s[b:i]), i
		except:
			raise

class StringType(Type):
	def convert_toplevel(self, s):
		return s
	def convert(self, s, i):
		i = eat_blank(s, i)
		if i >= len(s):
			raise		
		end = s[i]
		if end not in '\'\"':
			raise		
		i += 1
		b = i
		while True:
			if i >= len(s):
				raise
			elif s[i] == end:
				i += 1
				break
			elif s[i] == '\\':
				i += 1
				if i >= len(s):
					raise
				i += 1
			else:
				i += 1
		return s[b:i-1], i

class List(Type):
	def __init__(self, type):
		Type.__init__(self)
		self.type = type
	def clone(self):
		ret = Type.clone(self)
		ret.type = self.type.clone()
		return ret
	def check(self, ck):
		# TODO: test value is an array
		for item in ck.get().value:
			if not ck.push_check_pop(self.type, item):
				return False
		return Type.check(self, ck) # TODO: check self constraints
	def convert_toplevel(self, s):
		ret, i = self.convert(s, 0)
		i = eat_blank(s, i)
		if i < len(s):
			raise
		return ret
	def convert(self, s, i):
		i = eat_blank(s, i)
		if i >= len(s):
			raise
		if s[i] != '[':
			raise
		i += 1
		ret = []
		while True:
			value, i = self.type.convert(s, i)
			ret.append(value)
			i = eat_blank(s, i)
			if i >= len(s):
				raise
			elif s[i] == ']':
				i += 1
				break
			elif s[i] == ',':
				i += 1
				continue
			else:
				raise
		return ret, i

class Tuple(Type):
	def __init__(self, *types):
		if len(types) == 0:
			raise Exception("tuple size cannot be 0")
		Type.__init__(self)
		self.types = types
	def clone(self):
		ret = Type.clone(self)
		ret.types = [t.clone() for t in self.types]
		return ret
	def check(self, ck):
		value = ck.get().value
		for i in xrange(len(self.types)):
			if not ck.push_check_pop(self.types[i], value[i]):
				return False
		return Type.check(self, ck) # TODO: check self constraints
	def convert_toplevel(self, s):
		ret, i = self.convert(s, 0)
		i = eat_blank(s, i)
		if i < len(s):
			raise
		return ret
	def convert(self, s, i):
		i = eat_blank(s, i)
		if i >= len(s):
			raise
		if s[i] != '{':
			raise
		i += 1
		ret = []
		for j in xrange(len(self.types)):
			if j > 0:
				i = eat_blank(s, i)
				if s[i] != ',':
					raise
				i += 1
			value, i = self.types[j].convert(s, i)
			ret.append(value)
		i = eat_blank(s, i)
		if i >= len(s):
			raise
		if s[i] != '}':
			raise
		i += 1
		return tuple(ret), i

Bool = BoolType()
Int = IntType()
Float = FloatType()
String = StringType()
