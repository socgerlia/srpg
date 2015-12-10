class Constraint:
	def __and__(self, rhs):
		return AndConstraint(self, rhs)
	def __or__(self, rhs):
		return OrConstraint(self, rhs)
	def __not__(self):
		return NotConstraint(self)
	def check(*args):
		return true

class AndConstraint(Constraint):
	def __init__(self, lhs, rhs):
		self.lhs = lhs
		seld.rhs = rhs
	def check(*args):
		return self.lhs.check(*args) and self.rhs.check(*args)

class OrConstraint(Constraint):
	def __init__(self, lhs, rhs):
		self.lhs = lhs
		seld.rhs = rhs
	def check(*args):
		return self.lhs.check(*args) or self.rhs.check(*args)

class NotConstraint(Constraint):
	def __init__(self, inner):
		self.inner = inner
	def check(*args):
		return not self.inner(*args)

class extern(Constraint):
	def __init__(self, file, fieldname="id"):
		self.file = file
		self.fieldname = fieldname
