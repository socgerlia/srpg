import copy

class Constraint:
	def __and__(self, rhs):
		return AndConstraint(self, rhs)
	def __or__(self, rhs):
		return OrConstraint(self, rhs)
	def __not__(self):
		return NotConstraint(self)
	def check(self, ck):
		return true
	def clone(self):
		return copy.copy(self)

class AndConstraint(Constraint):
	def __init__(self, lhs, rhs):
		self.lhs = lhs
		self.rhs = rhs
	def check(self, ck):
		return self.lhs.check(ck) and self.rhs.check(ck)
	def clone(self):
		return self.__class__(self.lhs.clone(), self.rhs.clone())

class OrConstraint(Constraint):
	def __init__(self, lhs, rhs):
		self.lhs = lhs
		self.rhs = rhs
	def check(self, ck):
		return self.lhs.check(ck) or self.rhs.check(ck)
	def clone(self):
		return self.__class__(self.lhs.clone(), self.rhs.clone())

class NotConstraint(Constraint):
	def __init__(self, inner):
		self.inner = inner
	def check(self, ck):
		return not self.inner(ck)
	def clone(self):
		return self.__class__(self.inner.clone())

# 
class Extern(Constraint):
	def __init__(self, file, fieldname="id"):
		self.file = file
		self.fieldname = fieldname

class UniqueType(Constraint):
	def __init__(self):
		self.data = None

Unique = UniqueType()
