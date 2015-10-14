class Index:
	def __init__(self, name):
		self.name = name
	def decl(self, struct, field):
		return "{0}<tag<i_{1}>, member<{2}, {3}, &{2}::{1}>>".format(self.name, field.name, struct.name, field.type.name)

unordered_unique = Index("unordered_unique")
ordered_unique = Index("ordered_unique")
unordered_non_unique = Index("unordered_non_unique")
ordered_non_unique = Index("ordered_non_unique")
