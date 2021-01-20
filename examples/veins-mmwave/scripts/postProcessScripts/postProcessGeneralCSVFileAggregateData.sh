#!/bin/bash

scenario=$1

echo "Generate PostProcessCSVFileAggregateData javac file"
javac PostProcessCSVFileAggregateData.java > /dev/null

if [ $? -ne 0 ]; then
	echo "Failed to compile PostProcessCSVFileAggregateData.java file"
else
	echo "Done!"
	java PostProcessCSVFileAggregateData "../../results/ieee80211ad${scenario}AggregateData-processed.csv" "../../results/aggregateData${scenario}.csv"
	java PostProcessCSVFileAggregateData "../../results/SAMBA${scenario}AggregateData-processed.csv" "../../results/aggregateData${scenario}.csv"
	java PostProcessCSVFileAggregateData "../../results/FML${scenario}AggregateData-processed.csv" "../../results/aggregateData${scenario}.csv"


	#Cause 80211ad does not have overhead, we must add 0s to the overhead
	javac AddOverheadForIeee80211ad.java
	java AddOverheadForIeee80211ad "../../results/SAMBA${scenario}Overhead-processed.csv" "../../results/ieee80211ad${scenario}Overhead-processed.csv"

	java PostProcessCSVFileAggregateData "../../results/ieee80211ad${scenario}Overhead-processed.csv" "../../results/overhead${scenario}.csv"
	java PostProcessCSVFileAggregateData "../../results/SAMBA${scenario}Overhead-processed.csv" "../../results/overhead${scenario}.csv"
	java PostProcessCSVFileAggregateData "../../results/FML${scenario}Overhead-processed.csv" "../../results/overhead${scenario}.csv"

	cd ../plotScripts
	Rscript aggregateData.R "../../results/aggregateData${scenario}.csv" "../../results/aggregateData${scenario}.pdf"
	Rscript overhead.R "../../results/overhead${scenario}.csv" "../../results/overhead${scenario}.pdf"
	
fi
