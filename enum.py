import sys

assert(len(sys.argv) == 2)

enumer = []
strs = []

with open(sys.argv[1]) as f:
	lines = f.readlines()
	for line in lines:
		if len(line.split()) == 2:
			(s, e) = line.split()
			enumer.append(e)
			strs.append(s)

enum_str_lines = [ "enum {" ]
strs_str_lines = [ "const char *tokens[] = {" ] 

INDENT = "  "
MAX_LINE_LEN = 70

cur_enum = INDENT
cur_str = INDENT

for i in range(0, len(enumer)):
	if(len(cur_enum + enumer[i]) + 2 > MAX_LINE_LEN):
		enum_str_lines.append(cur_enum)
		cur_enum = INDENT
	cur_enum = cur_enum + enumer[i]	 + ", "

	if(len(cur_str + strs[i]) + 4 > MAX_LINE_LEN):
		strs_str_lines.append(cur_str)
		cur_str = INDENT
	cur_str = cur_str + '"' + strs[i] + '", '


strs_str_lines.append(cur_str[0:-2])
enum_str_lines.append(cur_enum[0:-2])
strs_str_lines.append("};")
enum_str_lines.append("};")

print()

for line in enum_str_lines:
	print(line)

print()

for line in strs_str_lines:
	print(line)

print()
	
