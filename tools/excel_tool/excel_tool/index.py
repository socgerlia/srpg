def _meta(tag, **args):
	return dict(args, tag = tag, type = "index")

unordered_unique = _meta("unordered_unique")
ordered_unique = _meta("ordered_unique")
