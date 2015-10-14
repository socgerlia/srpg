class Constraint:
	pass
	# def __init__(self, name):
	# 	self.name = name

class extern(Constraint):
	def __init__(self, file, fieldname="id"):
		self.file = file
		self.fieldname = fieldname
