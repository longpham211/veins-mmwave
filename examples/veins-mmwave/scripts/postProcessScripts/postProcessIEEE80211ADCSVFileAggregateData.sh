#!/bin/bash

scenario=$1

echo "Generate PostProcessCSVFileAggregateData javac file"
javac PostProcessIEEE80211ADCSVFileAggregateData.java > /dev/null

if [ $? -ne 0 ]; then
	echo "Failed to compile PostProcessCSVFileAggregateData.java file"
else
	echo "Done!"
	java PostProcessIEEE80211ADCSVFileAggregateData "../../results/ieee80211ad${scenario}BI30AggregateData-processed.csv" "../../results/ieee80211adAggregateData${scenario}.csv"
	java PostProcessIEEE80211ADCSVFileAggregateData "../../results/ieee80211ad${scenario}BI40AggregateData-processed.csv" "../../results/ieee80211adAggregateData${scenario}.csv"
	java PostProcessIEEE80211ADCSVFileAggregateData "../../results/ieee80211ad${scenario}BI50AggregateData-processed.csv" "../../results/ieee80211adAggregateData${scenario}.csv"
	java PostProcessIEEE80211ADCSVFileAggregateData "../../results/ieee80211ad${scenario}BI60AggregateData-processed.csv" "../../results/ieee80211adAggregateData${scenario}.csv"
	java PostProcessIEEE80211ADCSVFileAggregateData "../../results/ieee80211ad${scenario}BI70AggregateData-processed.csv" "../../results/ieee80211adAggregateData${scenario}.csv"
	java PostProcessIEEE80211ADCSVFileAggregateData "../../results/ieee80211ad${scenario}BI80AggregateData-processed.csv" "../../results/ieee80211adAggregateData${scenario}.csv"
	java PostProcessIEEE80211ADCSVFileAggregateData "../../results/ieee80211ad${scenario}BI90AggregateData-processed.csv" "../../results/ieee80211adAggregateData${scenario}.csv"
	java PostProcessIEEE80211ADCSVFileAggregateData "../../results/ieee80211ad${scenario}BI100AggregateData-processed.csv" "../../results/ieee80211adAggregateData${scenario}.csv"

	cd ../plotScripts
	Rscript ieee80211adAggregateData.R "../../results/ieee80211adAggregateData${scenario}.csv" "../../results/ieee80211adAggregateData${scenario}.pdf"
	
fi
