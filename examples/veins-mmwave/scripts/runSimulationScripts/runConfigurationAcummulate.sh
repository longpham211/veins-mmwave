#!/bin/bash
scenario="$1"

rm "../../results/${scenario}-processed.csv" &> /dev/null
if [ $? -eq 0 ]; then
	echo "Delete the existing ${scenario}-processed.csv file"
fi

echo "Generate ProcessCSVFile javac file"
javac ../ProcessCSVFile.java > /dev/null
if [ $? -ne 0 ]; then
    	echo "Failed to compile ProcessCSVFile.java file"
else 
	echo "Done!"
	cd ../..

	repeat_string=$(grep repeat omnetpp.ini)
	repeat=${repeat_string:8:13}

	counter=0

	while [ $counter -lt $repeat ] 
	do
	    echo "Running ${scenario}-${counter}"
	    ./run -u Cmdenv -c "${scenario}" -r "${counter}" #> /dev/null
	    if [ $? -eq 0 ]; then
		echo "Success!"
		echo "Convert vec file to CSV-Spreadsheet files!"
		cd results/
		scavetool x "${scenario}-${counter}.vec" -o "${scenario}AggregateData-${counter}.csv" -f "name(aggregateData:vector)" -F CSV-S -x allowMixed=false -x columnNames=true -x precision=14 -x separator=semicolon > /dev/null
		
		scavetool x "${scenario}-${counter}.vec" -o "${scenario}Overhead-${counter}.csv" -f "name(overhead:vector)" -F CSV-S -x allowMixed=false -x columnNames=true -x precision=14 -x separator=semicolon > /dev/null
		
		scavetool x "${scenario}-${counter}.vec" -o "${scenario}PeriodCount-${counter}.csv" -f "name(period:vector)" -F CSV-S -x allowMixed=false -x columnNames=true -x precision=14 -x separator=semicolon > /dev/null
		scavetool x "${scenario}-${counter}.vec" -o "${scenario}SelectedMCS-${counter}.csv" -f "name(selectedMCS:vector)" -F CSV-S -x allowMixed=false -x columnNames=true -x precision=14 -x separator=semicolon > /dev/null
	    	if [ $? -eq 0 ]; then
			echo "Convert success! Delete vec, csv, vci files to save storage!"
			rm  "${scenario}-${counter}.vec"  "${scenario}-${counter}.sca" "${scenario}-${counter}.vci" 

			cd ../scripts
			java ProcessCSVFile "../results/${scenario}AggregateData-${counter}.csv" "3600" "G"
			echo "ProcessAggregateData file to Gigabit"
			java ProcessCSVFile "../results/${scenario}Overhead-${counter}.csv" "3600" "M"
			echo "Process OverheadData file to Megabit" 
			if [ $? -eq 0 ]; then
#				echo "Delete csv file to save storage!"
#				rm "../results/${scenario}AggregateData-${counter}.csv"
#				rm "../results/${scenario}Overhead-${counter}.csv"

				cd .. #Come back to the original location to process the next repetition
			else
				echo "Failed to post process the csv file"
			fi
		else
			echo "Failed to convert to csv file"
			exit
		fi

	    else	
		echo "Failed to run the simulation!"
		exit
	    fi

	       ((counter++))
	done

	echo "DONE!"
fi
