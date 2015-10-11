def _meta(tag, **args):
	return dict(args, tag = tag, type = "type")

int = _meta("int")
string = _meta("string")

def vector(t):
	return _meta("vector", t = t)

def multi_map(k, v):
	return _meta("multi_map", key = k, value = v)

def enum(name):
	return _meta("enum", name = name)
