#!/usr/bin/python

import sys

# Transformas a float in a percentage.
def normalizeSingleValue(result):
    try:
        rValue = float(result)
    except ValueError:
        return result

    if rValue < 0:
        rValue += 1
    else:
        rValue -= 1

    rValue *= 100
    rValue = int(round(rValue))
    return str(rValue)

# Prints the results in the screen.
def printResults(suites, flags, nBenchmarks, outPath):
    # Print the header.
    header = "BENCHMARK\SHELL_VERSION"
    for flag in flags:
        header += " "+ flag +""

    print header

    for i in range(len(suites)):
        print(suites[i])

        for j in range(nBenchmarks[i]):
            bench = input[i][0][j].split(":", 1)
            bench = bench[0]
            print(bench),

            for k in range(len(flags)):
                result = input[i][k][j].split()
                result = result[1]

                print(" " + result),

            print ""

# Generates the output in Comma Separated Values format.
def generateCSV(suites, flags, nBenchmarks, outPath):
    print "generateCSV(): oh! I'm not finished yet! Please do that!"

# Generates the output in the Google Code wiki format.
def generateGWiki(suites, flags, nBenchmarks, outPath):
    outPath += ".wiki"

    outFile = open(outPath, "w")

    # Print the header.
    header = "|| *BENCHMARK \ SHELL VERSION* ||"
    for flag in flags:
        header += " *"+ flag +"* ||"

    outFile.write(header + "\n")

    for i in range(len(suites)):
        outFile.write("|| _*" + suites[i]+ "*_ ||\n")

        for j in range(nBenchmarks[i]):
            bench = input[i][0][j].split(":", 1)
            bench = bench[0]
            outFile.write("|| *" + bench + "* ||")

            for k in range(len(flags)):
                result = input[i][k][j].split()
                result = result[1]

                result = normalizeSingleValue(result)

                if result[0] == "-" and len(result) > 1:
                    outFile.write(" <font color=\"red\">" + result + "</font> ||")
                elif result[0] == "-":
                    outFile.write(" <font color=\"blue\">" + result + "</font> ||")
                elif result == "??":
                    outFile.write(" " + result + " ||")
                else:
                    outFile.write(" <font color=\"green\">" + result + "</font> ||")

            outFile.write("\n")

    outFile.close()

    print "Results are located at " + outPath

# Generates the output in the Latex table format.
def generateTex(suites, flags, nBenchmarks, outPath):
    print "generateTex(): oh! I'm not finished yet! Please do that!"

outputFun = printResults

for arg in sys.argv:
    if arg == "-csv":
        outputFun = generateCSV
    if arg == "-gwiki":
        outputFun = generateGWiki
    if arg == "-tex":
        outputFun = generateTex

inFile = open(sys.argv[1])

input = inFile.read()

# Split the results by the benchmark suite.
input = input.split("suite: ")

input = [x for x in input if x != ""]

suites = []

for i in range(len(input)):
    split = input[i].split("\n", 1)
    suites.append(split[0])
    input[i] = split[1]

# Split the result of each suite by the shell flags used.
flags = []

for i in range(len(input)):
    input[i] = input[i].split("flags: ")

    input[i] = [x for x in input[i] if x != ""]

    currentFlags = []

    for j in range(len(input[i])):
        split = input[i][j].split("\n", 1)
        currentFlags.append(split[0])
        input[i][j] = split[1]

    if i == 0:
        flags = currentFlags
    else:
        if currentFlags != flags:
            print "FATAL ERROR!"
            exit()

nBenchmarks = [] # Number of benchmarks in each suite.

# Split the results by the benchmarks of each suite.
for i in range(len(suites)):
    for j in range(len(flags)):
        input[i][j] = input[i][j].split("\n")
        input[i][j] = [x for x in input[i][j] if x != ""]

    nBenchmarks.append(len(input[i][0]))

# Generate the results file.

outPath = "results"

outputFun(suites, flags, nBenchmarks, outPath)
