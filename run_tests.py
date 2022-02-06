import re
import subprocess as subproc
def load_test_file(filename):
    with open(filename, "r") as f:
        return f.readlines()

def run_test(test_file):
    values = []
    for line in load_test_file(test_file):
        values.append(re.findall("Expect: -?[\d\.\d]*", line))

    values = values[1:]
    values = [x[0].split(':') for x in values if x != []]
    values = [float(x[1]) for x in values]

    capture = subproc.run(["./xpr", test_file], capture_output=True)
    got = str(capture.stdout)
    got = re.findall("-?[\d\.\d]*", got)
    got = [float(x) for x in got if x != '']

    result = [x == y for x, y in zip(values, got)]
    if False not in result:
        print("[RESULT] === PASSED ===")
    else:
        print("[RESULT] === FAILED ===")


print("Running Tests")
print(f"[TEST] Expression Parsing and Evaluation")
run_test("tests/expression.xpr")
