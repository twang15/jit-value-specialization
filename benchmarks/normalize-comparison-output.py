import sys

inFile = open(sys.argv[1])

output = inFile.readlines()

output = output[10:] # Eliminate header.
output = [line for line in output if line != "\n"]
output = output[::2] # Eliminate redundant categories.

# Normalize numbers.
for i in range(len(output)):
    output[i] = output[i].split()

    if output[i][1][0] == "*":
        output[i][1] = "-" + output[i][1][1:]

    if output[i][1][-1] == "x":
        output[i][1] = output[i][1][:-1]

    output[i] = output[i][:2]

for line in output:
    print line[0] + " " + line[1]
