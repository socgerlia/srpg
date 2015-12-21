def _convert_field(trait, field_name, s):
	# TODO: catch
	field_type = trait.fields.get(field_name)
	if field_type:
		try:
			return field_type.convert_toplevel(s)
		except Exception as e:
			raise Exception("tpl_name: {0}, field_name: {1}, s: {2}".format(trait.name, field_name, s))
	else:
		return s
		# raise Exception('there is no field named "{0}" in data'.format(field_name))

# xml
def _load_xml(path, trait):
	from xml.etree.cElementTree import ElementTree
	t = ElementTree(file = path + trait.name + ".xml")
	root = t.getroot()
	return [{field.tag: _convert_field(trait, field.tag, field.text) for field in record} for record in root]

def XmlLoader(path):
	return lambda trait: _load_xml(path, trait)
