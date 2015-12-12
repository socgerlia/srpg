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
	pass

class Int(Type):
	def __init__(self, length=32):
		self.length = length
	def check(self, ck):
		# TODO: check whether type is Int, and Int length
		return Type.check(self, ck)
	def convert_toplevel(self, s):
		try:
			ret = int(float(s))
			return ret
		except:
			pass
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
			ret = int(float(s[b:i]))
			return ret, i
		except:
			pass

class String(Type):
	def check(self, ck):
		return Type.check(self, ck)
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
	def __init__(self, t):
		self.t = t
	def check(self, ck):
		# TODO: test value is an array
		for item in ck.value():
			if not ck.push_check_pop(self.t, item):
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
			value, i = self.t.convert(s, i)
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
