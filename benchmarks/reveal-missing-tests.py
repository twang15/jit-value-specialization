# Just shows which tests we are still not able to run.

import os

suitePath = []
suitePath.append("default/tests/sunspider-1.0")
suitePath.append("default/tests/v8-v6")
suitePath.append("default/tests/pbench-0.9")
suitePath.append("kraken/tests/kraken-1.1")

# Extract suites names.
suites = [suite.split("/")[-1] for suite in suitePath]

fList = []

# Extract tests.
for path in suitePath:
    files = os.listdir(path)
    files = [file for file in files if file[-3:] == ".js" and file[-7:-3] != "data"]

    for i in range(len(files)):
        files[i] = files[i][:-3]

    listFile = open(path + "/LIST")
    list = listFile.readlines()
    listFile.close()

    for i in range(len(list)):
        if list[i][-1] == "\n":
            list[i] = list[i][:-1]

    files = [file for file in files if not file in list]

    fList.append(files)

# Print results.
print "---------------------------------------------------"
print "WARNING: the following tests are not being executed"
print "---------------------------------------------------"

for i in range(len(suites)):
    print suites[i] + ":"

    for j in range(len(fList[i])):
        print "\t" + fList[i][j]
