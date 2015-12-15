
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



value = Tuple(Int(), List(Int()), String()).convert_toplevel("  {12, [12, 12, -23, -34.01 ], \"-34.01\nasd\" }  ")
