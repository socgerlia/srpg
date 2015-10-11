def _meta(tag, **args):
	return dict(args, tag = tag, type = "constraint")

def extern(file, fieldname="id"):
	return _meta("extern", file = file, fieldname = fieldname)

