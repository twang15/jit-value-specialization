#!/bin/bash

i=1
while [[ i -le $# ]]; do
    if [[ ${@:$i:1} == "-h" ]]; then
        echo "./run.sh [-h] [-shell <JS shell path>] [-csv] [-gwiki] [-tex]"
        exit
    elif [[ ${@:$i:1} == "-shell" ]]; then
        (( i++ ))
        shell=${@:$i:1}
    elif [[ ${@:$i:1} == "-csv" ]]; then
        outFormat=${@:$i:1}
    elif [[ ${@:$i:1} == "-gwiki" ]]; then
        outFormat=${@:$i:1}
    elif [[ ${@:$i:1} == "-tex" ]]; then
        outFormat=${@:$i:1}
    fi 

    (( i++ )) 
done   

if [[ -z $shell ]]; then
    echo "ERROR: missing JS shell path. Use -shell <shell_path>"
    exit
fi

if [[ -z $outFormat ]]; then
    outFormat=
fi

declare -a flags # Shell versions to be tested.
declare -a suites
declare -a run # How many times each suite will be executed.

# Shell versions to be tested.
baseLine=""

# Shell versions to be tested.
flags[0]="--ion-ps"
flags[1]="--ion-cp"
flags[2]="--ion-ps --ion-cp"
flags[3]="--ion-ps --ion-cp --ion-bce"
flags[4]="--ion-ps --ion-cp --ion-dcec"
flags[5]="--ion-ps --ion-cp --ion-linv"
flags[6]="--ion-ps --ion-cp --ion-dcec --ion-bce"
flags[7]="--ion-ps --ion-cp --ion-linv --ion-dcec --ion-bce"

# For each suite, we must specify the respective number of executions.
suites[0]="sunspider-1.0"
run[0]=5000

# Results file.
# It must be an absoute path.
outPath=/tmp/results.out

errorOutPath=/tmp/benchError.out

rm $outPath

# Since the actual benchmarks are in other folder, we need to normalize the
# shell path.
shell="../${shell}"

for ((i = 0; i < ${#suites[@]}; i++)); do
    # We need to move to the correct suite folder.
    if [[ ${suites[$i]} == "kraken-1.1" ]]; then
        cd kraken
    else
        cd default
    fi

    # As all versions will be tested against the baseline, we run it only once.
    echo "Running baseline version for " ${suites[$i]} " " ${run[$i]} " times: " $baseLine
    echo "suite: " ${suites[$i]} >> $outPath

    basePath=$(./sunspider --args="${baseLine}" --shell=$shell --run=${run[$i]} --suite=${suites[$i]} | tail -1 | awk '{print $5}')

    # Run all modified versions of the JS shell.

    echo "Running modified versions."

    for ((j = 0; j < ${#flags[@]}; j++)); do
        if [[ ! ${flags[$j]} ]]; then
            echo "ERROR: empty flag."
            exit
        fi

        echo "Running: " ${suites[$i]} ", version: " ${flags[$j]}
        echo "flags: " ${flags[$j]} >> $outPath

        modPath=$(./sunspider --args="${flags[$j]}" --shell=$shell --run=${run[$i]} --suite=${suites[$i]} 2> $errorOutPath | tail -1 | awk '{print $5}')

        if [[ ` grep "Segmentation fault" $errorOutPath | wc -l` -gt 0 ]]; then
            echo "FATAL ERROR: segmentation fault during benchmark execution."
            exit
        fi

        if [[ $modPath == $basePath ]]; then
            echo "ERROR: try a greater number of executions."
            exit
        fi

        # Compare execution results.
        echo "Comparing results."

        cmpoutPath=comparison.out

        ./sunspider-compare-results --shell=$shell --suite=${suites[$i]} $basePath $modPath > $cmpoutPath

        python ../normalize-comparison-output.py $cmpoutPath >> $outPath

        rm $cmpoutPath
    done;

    # Get back to the root, in order to run a new suite.
    cd ..
done;

# Generate results.
python generate-output.py $outPath $outFormat

rm $errorOutPath
