from type2 import *
from constraint import *

class TplTrait(object):
	def __init__(self, _tpl_name, **fields):
		self.name = _tpl_name
		self.fields = fields # { field_name: type, ... }

class TplInfo(object):
	def __init__(self, trait):
		self.trait = trait
		self.data = None

class CheckerStackElement(object):
	def __init__(self, type, value):
		self.type = type
		self.value = value

class Checker(object):
	def __init__(self, loader):
		self.loader = loader
		self.tpls = {} # { tpl_name: TplInfo, ... }
		self.tpl_name = ""
		self.field_name = ""
		self.stack = []

	def define_tpl(self, tpl_name, **fields):
		if "id" not in fields:
			fields["id"] = Int[Unique]
		# check if all values are instances of Type
		for field_name, field_type in fields.iteritems():
			if not isinstance(field_type, Type):
				raise Exception('the value of field "{0}" is not a Type, in tpl "{1}"'.format(field_name, tpl_name))
		# clone types
		fields = { field_name: field_type.clone() for field_name, field_type in fields.iteritems() }
		self.tpls[tpl_name] = TplInfo(TplTrait(tpl_name, **fields))

	def get_tpl(self, tpl_name):
		info = self.tpls.get(tpl_name)
		if not info:
			raise Exception('tpl {0} is not defined'.format(tpl_name))
		elif not info.data:
			info.data = self.loader(info.trait)
		return info

	def push(self, type, value):
		self.stack.append(CheckerStackElement(type, value))

	def pop(self):
		self.stack.pop()

	def push_check_pop(self, type, value):
		self.push(type, value)
		try:
			ret = type.check(self)
		except Exception as e:
			raise Exception("""
constraint "{constraint}" fail:
  tpl_name: {tpl_name}
  field_name: {field_name}
  id: {id}
  stack:
    {stack}
""".format(
	constraint = e.args[0].__class__,
	tpl_name = self.tpl_name,
	field_name = self.field_name,
	id = self.get(1).value["id"],
	stack = "\n    ".join([str((i, self.get(i).value)) for i in xrange(1, len(self.stack))])
))
			from sys import exit
			exit()
		self.pop()
		return ret

	def get(self, index=-1):
		return self.stack[index]

	def check_tpl(self, tpl_name):
		self.tpl_name = tpl_name
		info = self.get_tpl(tpl_name)
		self.push(None, info.data)

		for record in info.data:
			self.push(None, record)
			for field_name, field_value in record.iteritems():
				field_type = info.trait.fields.get(field_name)
				if field_type:
					self.field_name = field_name
					result = self.push_check_pop(field_type, field_value)
					# TODO: handle result
			self.pop()
		self.pop()

	def check_all(self):
		for tpl_name in self.tpls:
			self.check_tpl(tpl_name)
		print "check ok"
