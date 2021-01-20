#!/bin/bash

scenario=$1

echo "Generate PostProcessCSVFileAggregateData javac file"
javac PostProcessSAMBACSVFileAggregateData.java > /dev/null
javac PostProcessCSVFileAggregateData.java > /dev/null

if [ $? -ne 0 ]; then
	echo "Failed to compile PostProcessCSVFileAggregateData.java file"
else
	echo "Done!"
#	java PostProcessSAMBACSVFileAggregateData "../../results/SAMBA${scenario}40mAggregateData-processed.csv" "../../results/sambaAggregateData${scenario}.csv"
	java PostProcessSAMBACSVFileAggregateData "../../results/SAMBA${scenario}60mAggregateData-processed.csv" "../../results/sambaAggregateData${scenario}.csv"
	java PostProcessSAMBACSVFileAggregateData "../../results/SAMBA${scenario}80mAggregateData-processed.csv" "../../results/sambaAggregateData${scenario}.csv"
	java PostProcessSAMBACSVFileAggregateData "../../results/SAMBA${scenario}100mAggregateData-processed.csv" "../../results/sambaAggregateData${scenario}.csv"
	java PostProcessSAMBACSVFileAggregateData "../../results/SAMBA${scenario}120mAggregateData-processed.csv" "../../results/sambaAggregateData${scenario}.csv"
	java PostProcessSAMBACSVFileAggregateData "../../results/SAMBA${scenario}140mAggregateData-processed.csv" "../../results/sambaAggregateData${scenario}.csv"


	#Cause 80211ad does not have overhead, we must add 0s to the overhead
#	javac AddOverheadForIeee80211ad.java
#	java AddOverheadForIeee80211ad "../../results/SAMBA${scenario}Overhead-processed.csv" "../../results/ieee80211ad${scenario}Overhead-processed.csv"

	java PostProcessCSVFileAggregateData "../../results/SAMBA${scenario}Overhead-processed.csv" "../../results/sambaOverhead${scenario}.csv"
#	java PostProcessCSVFileAggregateData "../../results/FML${scenario}Overhead-processed.csv" "../../results/overhead${scenario}.csv"

	cd ../plotScripts
	Rscript sambaAggregateData.R "../../results/sambaAggregateData${scenario}.csv" "../../results/sambaAggregateData${scenario}.pdf"
	Rscript overhead.R "../../results/sambaOverhead${scenario}.csv" "../../results/sambaOverhead${scenario}.pdf"
	
fi
