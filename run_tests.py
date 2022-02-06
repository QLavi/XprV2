import re
import subprocess as subproc
def load_test_file(filename):
    with open(filename, "r") as f:
        return f.readlines()

values = []
for line in load_test_file("tests/expression.xpr"):
    values.append(re.findall("Expect: -?[\d\.\d]*", line))

values = values[1:]
values = [x[0].split(':') for x in values if x != []]
values = [float(x[1]) for x in values]

print("Running Tests")
print(f"[TEST] Expression Parsing and Evaluation")
capture = subproc.run(["./xpr", "tests/expression.xpr"], capture_output=True)
got = str(capture.stdout)
got = re.findall("-?[\d\.\d]*", got)
got = [float(x) for x in got if x != '']
result = [x == y for x, y in zip(values, got)]
if False in result:
    print("[RESULT] === Failed ===")
else:
    print("[RESULT] === Passed ===")

