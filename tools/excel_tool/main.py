def at(seq, i, default=None):
	return seq[i] if i < len(seq) else default

import argparse
def parse_argv():
	root = argparse.ArgumentParser(prog='excel_tool', description='an excel tool')  
	verbs = root.add_subparsers(dest='verb')
	verbs.required = True

	new = verbs.add_parser('new', help='new an empty .xlsx file')
	new.add_argument('path', help='file path')

	transform = verbs.add_parser('transform', help='transform .xlsx files')
	transform.add_argument('files', nargs='+', metavar='file', help='file paths')

	meta = verbs.add_parser('meta', help='print the meta data')
	meta.add_argument('path', help='file path')

	# intransform
	# intransform and open
	# constraint

	return root.parse_args()

args = parse_argv()

from excel_tool.type import *
from excel_tool.index import *
from excel_tool.constraint import *
def get_meta_data(ws):
	def meta_to_tuple(v):
		if not v:
			return ()
		ret = eval(str(v))
		if type(ret) is tuple:
			return ret
		else:
			return (ret,)
	def check_if_all_constraints(v, index):
		ret = []
		if index < len(v):
			seq = v[index]
			for item in seq:
				if item["type"] == "constraint":
					ret.append(item)
				else:
					raise
		return ret
	def transfrom_meta(v):
		ret = { "type": v[0] if len(v) > 0 else int }
		if ret["type"]["tag"] in ("multi_map", "map"): # TODO: unordered
			ret["indice"] = []
			ret["constraints"] = [check_if_all_constraints(v, 1), check_if_all_constraints(v, 2)]
		else:
			ret["indice"] = []
			ret["constraints"] = []
			for item in v[1:]:
				if item["type"] == "index":
					ret["indice"].append(item)
				elif item["type"] == "constraint":
					ret["constraints"].append(item)
				else:
					raise
		return ret
	def to_meta(name, v):
		meta = None
		if not v:
			if name == "id":
				meta = { "type": int, "indice": [unordered_unique], "constraints": [] }
			else:
				raise
		else:
			meta = transfrom_meta(meta_to_tuple(v))
		return (name, meta)

	try:
		rowIt = ws.rows
		meta_row = rowIt.next()
		name_row = rowIt.next()
		return [ to_meta(name.value, meta_row[i].value) for i, name in enumerate(name_row) ]
	except StopIteration:
		pass

def dump_src_file(name, meta):
	def get_type(t):
		tag = t["tag"]
		if tag == "vector":
			return "vector<{0}>".format(get_type(t["t"]))
		elif tag == "multi_map":
			return "multi_map<{0}, {1}>".format(get_type(t["key"]), get_type(t["value"]))
		elif tag == "enum":
			return get_type(int)
		else:
			return tag
	def get_index(typename, fieldname, t):
		return "{0}<member<{1}, {2}, &{1}::{3}>>".format(t["tag"], name, typename, fieldname)

	print "struct {0}{{".format(name)

	# fields
	for field in meta:
		print "  {0} {1};".format(get_type(field[1]["type"]), field[0])
	print

	# serialize
	print "  template<class Archive>"
	print "  void serialize(Archive& ar, const unsigned int version){"
	for field in meta:
		print "    ar & {0};".format(field[0])
	print "  }"
	print

	# typedef map_type
	print "  typedef multi_index_container<"
	print "    {0},".format(name)
	print "    indexed_by<"
	for field in meta:
		index = at(field[1]["indice"], 0)
		if index:
			print "      {0},".format(get_index(get_type(field[1]["type"]), field[0], index))
	print "    >"
	print "  > map_type;"

	print "};"

import openpyxl
verb = args.verb
if verb == 'new':
	def f(path):
		wb = openpyxl.Workbook()
		wb.save(path)
	f(args.path)
elif verb == 'transform':
	def f(path):
		from os.path import splitext, basename
		(rootname, ext) = splitext(basename(path))

		wb = openpyxl.load_workbook(path, read_only=True)
		ws = wb.active
		meta = get_meta_data(ws)
		dump_src_file(rootname, meta)
	f(args.files[0])
elif verb == 'meta':
	def f(path):
		import pprint
		pp = pprint.PrettyPrinter(indent=2)

		wb = openpyxl.load_workbook(path, read_only=True)
		ws = wb.active
		pp.pprint(get_meta_data(ws))
	f(args.path)

