import copy

class Constraint(object):
	def __and__(self, rhs):
		return AndConstraint(self, rhs)
	def __or__(self, rhs):
		return OrConstraint(self, rhs)
	def __not__(self):
		return NotConstraint(self)
	def clone(self):
		return copy.deepcopy(self)
	def check(self, ck):
		pass

class AndConstraint(Constraint):
	def __init__(self, lhs, rhs):
		self.lhs = lhs
		self.rhs = rhs
	def clone(self):
		return self.__class__(self.lhs.clone(), self.rhs.clone())
	def check(self, ck):
		self.lhs.check(ck)
		self.rhs.check(ck)

class OrConstraint(Constraint):
	def __init__(self, lhs, rhs):
		self.lhs = lhs
		self.rhs = rhs
	def clone(self):
		return self.__class__(self.lhs.clone(), self.rhs.clone())
	def check(self, ck):
		try:
			self.lhs.check(ck)
		except Exception as e:
			self.rhs.check(ck)

class NotConstraint(Constraint):
	def __init__(self, inner):
		self.inner = inner
	def clone(self):
		return self.__class__(self.inner.clone())
	def check(self, ck):
		try:
			self.inner.check(ck)
		except Exception as e:
			pass
		else:
			raise Exception(self)

# 
class Foreign(Constraint):
	tpl_map = {} # { tpl_name: { field_name: { field_value, ... }, ... }, ... }
	@staticmethod
	def get_field_value_set(ck, tpl_name, field_name):
		data_map = Foreign.tpl_map.get(tpl_name)
		if not data_map:
			data_map = {}
			Foreign.tpl_map[tpl_name] = data_map

		field_value_set = data_map.get(field_name)
		if not field_value_set:
			field_value_set = set((record[field_name] for record in ck.get_tpl(tpl_name))) # TODO: unique
		return field_value_set

	def __init__(self, tpl_name, field_name="id"):
		self.tpl_name = tpl_name
		self.field_name = field_name
	def check(self, ck):
		if ck.get().value not in Foreign.get_field_value_set(ck, self.tpl_name, self.field_name):
			raise Exception(self)

class UniqueType(Constraint):
	def __init__(self):
		self.data = set([])
	def check(self, ck):
		value = ck.get().value
		if value in self.data:
			raise Exception(self)
		self.data.add(value)

class In(Constraint):
	def __init__(self, *value_set):
		self.value_set = set(value_set)
	def check(self, ck):
		if ck.get().value not in self.value_set:
			raise Exception(self)

Unique = UniqueType()
