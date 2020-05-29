import sys;

# A simple and badly written python script to
# automate writing enums and their respective
# character pointer arrays for the lexing phase
# so that it's easier to reorder them.

assert(len(sys.argv) == 4);

fp = open(sys.argv[1], "r");

line = fp.readline();
wenum = None;
wstring = None;

MAX_LINE_LEN = 70;

enum_def = [ "enum " + sys.argv[2] + " {"            ];
iden_def = [ "const char *" + sys.argv[3] + "[] = {" ];

curiden = "";
curenum = "";

while line:
	pair = line.strip().split();
	assert(len(pair) == 2 or len(pair) == 0);

	if len(pair) == 2:

		wenum, wstring = pair[1], pair[0];
		if(len(curiden) + len(wstring) > MAX_LINE_LEN - 4):
			print(curiden);
			iden_def.append("    " + curiden);
			curiden = "";
		curiden = '"' + wstring + '", ' + curiden;

		if(len(curenum) + len(wenum) > MAX_LINE_LEN + 4):
			enum_def.append("    " + curenum);
			curenum = "";
		curenum = wenum + ", " + curenum;

	line = fp.readline();

if curenum.isspace():
	curenum = enum_def.pop();

if curiden.isspace():
	curiden = iden_def.pop();

enum_def.append("    " + curenum[0:-2]);
iden_def.append("    " + curiden[0:-2]);

enum_def.append("};");
iden_def.append("};");

print();

for str in enum_def:
	print(str);

print();

for str in iden_def:
	print(str);