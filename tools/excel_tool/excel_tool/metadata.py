from type import *
from index import *
from constraint import *

class MetaData:
	def __metadecl_to_tuple(self, v):
		if not v:
			return ()
		ret = eval(str(v))
		if type(ret) is tuple:
			return ret
		else:
			return (ret,)

	def init_from_metadecl(self, name, metadecl):
		self.name = name
		if not metadecl:
			if name == "id":
				metadecl = "int, unordered_unique"
			else:
				assert False
		metatuple = self.__metadecl_to_tuple(metadecl)

		self.type = metatuple[0]
		self.indice = metatuple[1:]
		return self
	def init_from_dict(self, v):
		self.name = v[0]
		self.type = v[1]["type"]
		self.indice = v[1]["indice"]
		return self

	def to_dict(self):
		return (self.name, {"type":self.type, "indice":self.indice})
	def __repr__(self):
		return self.to_dict().__repr__()
