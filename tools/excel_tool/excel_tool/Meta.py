import copy

class Meta:
	def __init__(self):
		self.props = []
		self.readonly = False
	def __call__(self, prop):
		return self.add_props(*props)
	def __div__(self, prop):
		return self.add_props(prop)
	def __set_readonly(self):
		self.readonly = True
		return self
	def add_props(self, *props):
		# readonly will return a new object
		ret = self
		if self.readonly:
			ret = copy.deepcopy(self)
			ret.readonly = False
		ret.props += props
		return ret
