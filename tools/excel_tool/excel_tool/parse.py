def make_record(record):
	return { field.tag: field.text for field in record }
def all_record(root):
	for record in root:
		yield make_record(record)
def load_data(tpl_name):
	from xml.etree.cElementTree import ElementTree
	t = ElementTree(file = tpl_name + ".xml")
	root = t.getroot()
	return [record for record in all_record(root)]

def eat_blank(s, i):
	while i < len(s) and s[i].isspace():
		i += 1
	return i

def parse_list(s, end, i):
	ret = []
	i = eat_blank(s, i)
	if s[i] == '[':
		item, i = parse_list(s, ']', i + 1)
		ret.append(item)
	elif s[i] == '{':
		item, i = parse_list(s, '}', i + 1)
		ret.append(tuple(item))
	else:
		raise
	return ret, i

def parse_erlang_object(s):
	i = eat_blank(s, 0)
	if s[i] == '[':
		item, i = parse_list(s, ']', i + 1)
		i = eat_blank(s, i)
		assert i == len(s)
		return item
	elif s[i] == '{':
		item, i = parse_list(s, '}', i + 1)
		i = eat_blank(s, i)
		assert i == len(s)
		return tuple(item)
	elif s[i] in '\'\"':
		pass # TODO:

lst = []
def erlang_to_python(s):
	del lst[:]
	b = 0
	i = 0
	while i < len(s):
		if s[i] == '{':
			lst.append(s[b:i])
			lst.append('tuple([')
			i += 1
			b = i
		elif s[i] == '}':
			lst.append(s[b:i])
			lst.append('])')
			i += 1
			b = i
		elif s[i] in '\'\"':
			end = s[i]
			i += 1
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
		else:
			i += 1
	lst.append(s[b:i])
	return ''.join(lst)

class Type(object):
	def __init__(self):
		self.constraint = None
	# def __pos__(self):
	# 	return List(self)
	def __getitem__(self, i):
		if not self.constraint:
			self.constraint = i
		else:
			self.constraint = AndConstraint(self.constraint, i)
		return self
	def check(self, ck):
		return constraint.check(ck)

class Bool(Type):
	def convert_toplevel(self, s):
		try:
			ret = int(s)
			if ret == 0:
				return False
			elif ret == 1:
				return True
			else:
				raise
		except:
			raise
	def convert(self, s, i):
		i = eat_blank(s, i)
		if i >= len(s):
			raise
		if s[i] == '0':
			return False, i+1
		elif s[i] == '1':
			return True, i+1
		else:
			raise

class Int(Type):
	def __init__(self, length=32):
		self.length = length
	def convert_toplevel(self, s):
		try:
			return int(float(s))
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
			return int(float(s[b:i])), i
		except:
			raise

class Float(Type):
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

class String(Type):
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
		self.type = type
	def check(self, ck):
		# TODO: test value is an array
		for item in ck.value():
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
		self.types = types
	def check(self, ck):
		value = ck.value() # TODO: test value is a tuple, and size is matched
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


value = Tuple(Int(), List(Int()), String()).convert_toplevel("  {12, [12, 12, -23, -34.01 ], \"-34.01\nasd\" }  ")
