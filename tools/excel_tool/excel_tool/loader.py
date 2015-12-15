class XmlLoader(object):
	def __init__(self, trait):
		self.trait = trait
	def load(self):
		from xml.etree.cElementTree import ElementTree
		t = ElementTree(file = self.trait.name + ".xml")
		root = t.getroot()
		return [{field.tag: self.convert_field(field.tag, field.text) for field in record} for record in root]
	def convert_field(self, field_name, s):
		# TODO: catch
		field_type = self.trait.fields.get(field_name)
		if not field_type:
			raise Exception('there is no field named "{0}" in data'.format(field_name))
		return field_type.convert_toplevel(s)
