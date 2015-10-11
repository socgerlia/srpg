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

class MetaData:
	def __metadecl_to_tuple(self, v):
		if not v:
			return ()
		ret = eval(str(v))
		if type(ret) is tuple:
			return ret
		else:
			return (ret,)
	def __check_if_all_constraints(self, seq):
		ret = []
		for item in seq:
			if item["type"] == "constraint":
				ret.append(item)
			else:
				raise
		return ret

	def init_from_metadecl(self, name, metadecl):
		self.name = name
		if not metadecl:
			if name == "id":
				metadecl = "int, unordered_unique"
			else:
				raise
		metatuple = self.__metadecl_to_tuple(metadecl)

		self.type = metatuple[0]
		if self.type["tag"] in ("multi_map", "map"): # TODO: unordered
			self.indice = []
			self.constraints = [
				self.__check_if_all_constraints(at(metatuple, 1, [])),
				self.__check_if_all_constraints(at(metatuple, 2, []))]
		else:
			self.indice = []
			self.constraints = []
			for item in metatuple[1:]:
				if item["type"] == "index":
					self.indice.append(item)
				elif item["type"] == "constraint":
					self.constraints.append(item)
				else:
					raise
		return self
	def init_from_dict(self, v):
		self.name = v[0]
		self.type = v[1]["type"]
		self.indice = v[1]["indice"]
		self.constraints = v[1]["constraints"]
		return self

	def to_dict(self):
		return (self.name, {"type":self.type, "indice":self.indice, "constraints":self.constraints})
	def __repr__(self):
		return self.to_dict().__repr__()

	def type_str(self):
		def get_typename(t):
			tag = t["tag"]
			if tag == "vector":
				return "vector<{0}>".format(get_typename(t["t"]))
			elif tag == "multi_map":
				return "multi_map<{0}, {1}>".format(get_typename(t["key"]), get_typename(t["value"]))
			elif tag == "enum":
				return get_typename(int)
			else:
				return tag
		return get_typename(self.type)
	def index_str(self, structname):
		index = at(self.indice, 0)
		if index:
			return "{0}<member<{1}, {2}, &{1}::{3}>>".format(index["tag"], structname, self.type_str(), self.name)

def get_meta_data(ws):
	try:
		rowIt = ws.rows
		meta_row = rowIt.next()
		name_row = rowIt.next()
		return [ MetaData().init_from_metadecl(name.value, meta_row[i].value) for i, name in enumerate(name_row) ]
	except StopIteration:
		pass

def dump_src_file(name, meta):
	print "struct {0}{{".format(name)

	# fields
	for field in meta:
		print "  {0} {1};".format(field.type_str(), field.name)
	print

	# serialize
	print "  template<class Archive>"
	print "  void serialize(Archive& ar, const unsigned int version){"
	for field in meta:
		print "    ar & {0};".format(field.name)
	print "  }"
	print

	# typedef map_type
	print "  typedef multi_index_container<"
	print "    {0},".format(name)
	print "    indexed_by<"
	for field in meta:
		s = field.index_str(name)
		if s:
			print "      {0},".format(s)
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
		meta = get_meta_data(ws)
		pp.pprint([v.to_dict() for v in meta])
	f(args.path)

