#!/bin/bash

scenario=$1
diameter=$2

rm "../${diameter}mResult/aggregateData${scenario}${diameter}BI100.csv"
rm "../${diameter}mResult/aggregateData${scenario}${diameter}BI60.csv"
rm "../${diameter}mResult/aggregateData${scenario}${diameter}BI30.csv"
rm "../${diameter}mResult/ieee80211ad${scenario}BI100Overhead-processed.csv"
rm "../${diameter}mResult/overhead${scenario}${diameter}.csv"

echo "Generate PostProcessCSVFileAggregateData javac file"
javac PostProcessCSVFileAggregateData.java > /dev/null


if [ $? -ne 0 ]; then
	echo "Failed to compile PostProcessCSVFileAggregateData.java file"
else
	echo "Done!"
	java PostProcessCSVFileAggregateData "../${diameter}mResult/ieee80211ad${scenario}BI100AggregateData-processed.csv" "../${diameter}mResult/aggregateData${scenario}${diameter}BI100.csv" 
	java PostProcessCSVFileAggregateData "../${diameter}mResult/SAMBA${scenario}BI100AggregateData-processed.csv" "../${diameter}mResult/aggregateData${scenario}${diameter}BI100.csv" 
	java PostProcessCSVFileAggregateData "../${diameter}mResult/FML${scenario}AggregateData-processed.csv" "../${diameter}mResult/aggregateData${scenario}${diameter}BI100.csv"

	java PostProcessCSVFileAggregateData "../${diameter}mResult/ieee80211ad${scenario}BI60AggregateData-processed.csv" "../${diameter}mResult/aggregateData${scenario}${diameter}BI60.csv"
	java PostProcessCSVFileAggregateData "../${diameter}mResult/SAMBA${scenario}BI60AggregateData-processed.csv" "../${diameter}mResult/aggregateData${scenario}${diameter}BI60.csv"
	java PostProcessCSVFileAggregateData "../${diameter}mResult/FML${scenario}AggregateData-processed.csv" "../${diameter}mResult/aggregateData${scenario}${diameter}BI60.csv"

	java PostProcessCSVFileAggregateData "../${diameter}mResult/ieee80211ad${scenario}BI30AggregateData-processed.csv" "../${diameter}mResult/aggregateData${scenario}${diameter}BI30.csv" 
	java PostProcessCSVFileAggregateData "../${diameter}mResult/SAMBA${scenario}BI30AggregateData-processed.csv" "../${diameter}mResult/aggregateData${scenario}${diameter}BI30.csv" 
	java PostProcessCSVFileAggregateData "../${diameter}mResult/FML${scenario}AggregateData-processed.csv" "../${diameter}mResult/aggregateData${scenario}${diameter}BI30.csv"


	#Cause 80211ad does not have overhead, we must add 0s to the overhead
	javac AddOverheadForIeee80211ad.java
	java AddOverheadForIeee80211ad "../${diameter}mResult/SAMBA${scenario}BI100Overhead-processed.csv" "../${diameter}mResult/ieee80211ad${scenario}BI100Overhead-processed.csv"

	java PostProcessCSVFileAggregateData "../${diameter}mResult/ieee80211ad${scenario}BI100Overhead-processed.csv" "../${diameter}mResult/overhead${scenario}${diameter}BI100.csv"
	java PostProcessCSVFileAggregateData "../${diameter}mResult/SAMBA${scenario}BI100Overhead-processed.csv" "../${diameter}mResult/overhead${scenario}${diameter}BI100.csv"
	java PostProcessCSVFileAggregateData "../${diameter}mResult/FML${scenario}Overhead-processed.csv" "../${diameter}mResult/overhead${scenario}${diameter}BI100.csv"

	java PostProcessCSVFileAggregateData "../${diameter}mResult/ieee80211ad${scenario}BI100Overhead-processed.csv" "../${diameter}mResult/overhead${scenario}${diameter}BI60.csv"
	java PostProcessCSVFileAggregateData "../${diameter}mResult/SAMBA${scenario}BI60Overhead-processed.csv" "../${diameter}mResult/overhead${scenario}${diameter}BI60.csv"
	java PostProcessCSVFileAggregateData "../${diameter}mResult/FML${scenario}Overhead-processed.csv" "../${diameter}mResult/overhead${scenario}${diameter}BI60.csv"

	java PostProcessCSVFileAggregateData "../${diameter}mResult/ieee80211ad${scenario}BI100Overhead-processed.csv" "../${diameter}mResult/overhead${scenario}${diameter}BI30.csv"
	java PostProcessCSVFileAggregateData "../${diameter}mResult/SAMBA${scenario}BI30Overhead-processed.csv" "../${diameter}mResult/overhead${scenario}${diameter}BI30.csv"
	java PostProcessCSVFileAggregateData "../${diameter}mResult/FML${scenario}Overhead-processed.csv" "../${diameter}mResult/overhead${scenario}${diameter}BI30.csv"

	cd ../plotScripts
	Rscript aggregateDataV2.R "../${diameter}mResult/aggregateData${scenario}${diameter}BI100.csv" "../${diameter}mResult/aggregateData${scenario}${diameter}BI60.csv" "../${diameter}mResult/aggregateData${scenario}${diameter}BI30.csv" "../${diameter}mResult/aggregateData${scenario}${diameter}.pdf"
	Rscript overheadV2.R "../${diameter}mResult/overhead${scenario}${diameter}BI100.csv" "../${diameter}mResult/overhead${scenario}${diameter}BI60.csv" "../${diameter}mResult/overhead${scenario}${diameter}BI30.csv" "../${diameter}mResult/overhead${scenario}${diameter}.pdf"
	
fi
