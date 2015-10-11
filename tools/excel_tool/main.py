import argparse
def parse_argv():
	root = argparse.ArgumentParser(prog='excel_tool', description='an excel tool')  
	verbs = root.add_subparsers(dest='verb')
	verbs.required = True

	new = verbs.add_parser('new', help='new an empty .xlsx file')
	new.add_argument('path', help='file path')

	transform = verbs.add_parser('transform', help='transform .xlsx files')
	transform.add_argument('files', nargs='+', metavar='file', help='file paths')

	# intransform
	# intransform and open
	# constraint

	return root.parse_args()

args = parse_argv()

import openpyxl
verb = args.verb
if verb == 'new':
	def f(path):
		wb = openpyxl.Workbook()
		wb.save(path)
	f(args.path)
elif verb == 'transform':
	def transform(file):
		pass
