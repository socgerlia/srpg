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
		return true

class AndConstraint(Constraint):
	def __init__(self, lhs, rhs):
		self.lhs = lhs
		self.rhs = rhs
	def clone(self):
		return self.__class__(self.lhs.clone(), self.rhs.clone())
	def check(self, ck):
		return self.lhs.check(ck) and self.rhs.check(ck)

class OrConstraint(Constraint):
	def __init__(self, lhs, rhs):
		self.lhs = lhs
		self.rhs = rhs
	def clone(self):
		return self.__class__(self.lhs.clone(), self.rhs.clone())
	def check(self, ck):
		return self.lhs.check(ck) or self.rhs.check(ck)

class NotConstraint(Constraint):
	def __init__(self, inner):
		self.inner = inner
	def clone(self):
		return self.__class__(self.inner.clone())
	def check(self, ck):
		return not self.inner(ck)

# 
class Foreign(Constraint):
	tpl_map = {} # { tpl_name: { field_name: { field_value, ... }, ... }, ... }
	@staticmethod
	def get_field_value_set(ck, tpl_name, field_name):
		data_map = tpl_map.get(tpl_name)
		if not data_map:
			data_map = {}
			tpl_map[tpl_name] = data_map

		field_value_set = data_map.get(field_name)
		if not field_value_set:
			field_value_set = set((record[field_name] for record in ck.get_tpl(tpl_name))) # TODO: unique
		return field_value_set

	def __init__(self, tpl_name, field_name="id"):
		self.tpl_name = tpl_name
		self.field_name = field_name
	def check(self, ck):
		return ck.get().value in Foreign.get_field_value_set(ck, self.tpl_name, self.field_name)

class UniqueType(Constraint):
	def __init__(self):
		self.data = set([])
	def check(self, ck):
		value = ck.get().value
		if value in self.data:
			return False # TODO: raise
		self.data.add(value)
		return True

class In(Constraint):
	def __init__(self, value_set):
		self.value_set = value_set
	def check(self, ck):
		return ck.get().value in self.value_set



Unique = UniqueType()
