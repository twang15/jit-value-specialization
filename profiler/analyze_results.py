def getPassIndex(passes, pass_):
    if not pass_ in passes:
        print "FATAL ERROR"
        exit()

    for i in range(len(passes)):
        if passes[i] == pass_:
            return i

profilePath = "/tmp/ionprofile.out"
profileFile = open(profilePath)

profile = profileFile.read()
profile = profile.split("Script ")

scripts = [] # All known scripts.

sProfiles = [] # Profile data arranged by script and execution: sProfiles[script][execution].

# Extract scripts names and profile data.
for i in range(len(profile)):
    split = profile[i].split("\n", 1)

    if len(split) > 1:
        if split[1] != "":
            script = split[0]

            if not script in scripts:
                scripts.append(script)
                sProfiles.append([])
                sProfiles[-1].append(split[1])
            else:
                index = None
                for i in range(len(scripts)):
                    if scripts[i] == script:
                        index = i

                if index == None:
                    print "FATAL ERROR"
                    exit()

                sProfiles[index].append(split[1])

for i in range(len(sProfiles)):
    for j in range(len(sProfiles[i])):
        sProfiles[i][j] = sProfiles[i][j].split("\n")[:-1]

        for k in range(len(sProfiles[i][j])):
            sProfiles[i][j][k] = sProfiles[i][j][k].split(" ")

# Extract passes names.
passes = []

for i in range(len(sProfiles)):
    for j in range(len(sProfiles[i])):
        for k in range(len(sProfiles[i][j])):
            if not sProfiles[i][j][k][0] in passes:
                passes.append(sProfiles[i][j][k][0])


# Extract the time results in the correct order.
results = []

for i in range(len(sProfiles)):
    results.append([])

    for j in range(len(sProfiles[i])):
        results[i].append([])

        # Initially all results are empty.
        for k in range(len(passes)):
            results[i][j].append("-")

        # Store the known results.
        for k in range(len(sProfiles[i][j])):
            index = getPassIndex(passes, sProfiles[i][j][k][0])
            results[i][j][index] = sProfiles[i][j][k][1]

# Print the results in CSV format.
outPath = "results.csv"
outFile = open(outPath, "w")

# Print the header.
outFile.write("Script")
for pass_ in passes:
    outFile.write("," + pass_)
outFile.write("\n")

# Print the time results.
for i in range(len(scripts)):
    for j in range(len(sProfiles[i])):
        if j == 0:
            outFile.write(scripts[i])

        for k in range(len(passes)):
            outFile.write("," + results[i][j][k])

        outFile.write("\n")

outFile.close()
