class TplTrait(object):
	def __init__(self, name, **fields):
		self.name = name
		self.fields = fields # { field_name: type, ... }

class TplInfo(object):
	def __init__(self, traits):
		self.traits = traits
		self.data = None

class CheckerStackElement(object):
	def __init__(self, type, value):
		self.type = type
		self.value = value

class Checker(object):
	def __init__(self):
		self.tpls = {} # { tpl_name: TplInfo, ... }
		self.tpl_name = ""
		self.field_name = ""
		self.stack = []

	def get_tpl(self, tpl_name):
		if tpl_name not in self.tpls:
			raise
		info = self.tpls[tpl_name]
		if info.data == None:
			info.data = load_data(tpl_name) # TODO: load it
		return info

	def push(self, type, value):
		self.stack.append(CheckerStackElement(type, value))

	def pop(self):
		self.stack.pop()

	def push_check_pop(self, type, value):
		self.push(type, value)
		ret = type.check(self)
		self.pop()
		return ret

	def type(self, index=-1):
		return self.stack[index].type

	def value(self, index=-1):
		return self.stack[index].value

	def check_tpl(self, tpl_name):
		self.tpl_name = tpl_name
		info = self.get_tpl(tpl_name)
		self.push(None, info.data)

		for record in info.data:
			self.push(None, record)
			for field_name, field_value in record.iteritems():
				field_type = info.traits.fields.get(field_name)
				if field_type != None:
					self.field_name = field_name
					result = self.push_check_pop(field_type, field_value)
					# TODO: handle result
			self.pop()
		self.pop()

	def check_all(self):
		for tpl_name in self.tpls:
			self.check_tpl(tpl_name)
		print "check ok"
