#!/bin/bash

scenario=$1
diameter=$2
bi=$3
hour=$4
rm "../${diameter}mResult/mcsUsed${scenario}${diameter}BI100At${hour}.csv"
rm "../${diameter}mResult/channelUsageFML${scenario}${diameter}BI${bi}At${hour}.csv" 
rm "../${diameter}mResult/channelUsageIEEE80211ad${scenario}${diameter}BI${bi}At${hour}.csv" 
rm "../${diameter}mResult/channelUsageSAMBA${scenario}${diameter}BI${bi}At${hour}.csv" 

javac PostProcessCSVFileMCS.java 


if [ $? -ne 0 ]; then
	echo "Failed to compile PostProcessCSVFileAggregateData.java file"
else
	echo "Done!"

java PostProcessCSVFileMCS "../${diameter}mResult/ieee80211ad${scenario}BI${bi}SelectedMCS-processed.csv" "../${diameter}mResult/mcsUsed${scenario}${diameter}BI${bi}At${hour}.csv" "../${diameter}mResult/channelUsageIEEE80211ad${scenario}${diameter}BI${bi}At${hour}.csv" ${hour}
java PostProcessCSVFileMCS "../${diameter}mResult/SAMBA${scenario}BI${bi}SelectedMCS-processed.csv" "../${diameter}mResult/mcsUsed${scenario}${diameter}BI${bi}At${hour}.csv" "../${diameter}mResult/channelUsageSAMBA${scenario}${diameter}BI${bi}At${hour}.csv" ${hour}
java PostProcessCSVFileMCS "../${diameter}mResult/FML${scenario}SelectedMCS-processed.csv" "../${diameter}mResult/mcsUsed${scenario}${diameter}BI${bi}At${hour}.csv" "../${diameter}mResult/channelUsageFML${scenario}${diameter}BI${bi}At${hour}.csv" ${hour}


	cd ../plotScripts
	Rscript usedMCSAtHour.R "../${diameter}mResult/mcsUsed${scenario}${diameter}BI${bi}At${hour}.csv" "../${diameter}mResult/mcsUsed${scenario}${diameter}BI${bi}At${hour}.pdf"
	Rscript channelUsageAtHour.R "../${diameter}mResult/channelUsageFML${scenario}${diameter}BI${bi}At${hour}.csv" "../${diameter}mResult/channelUsageIEEE80211ad${scenario}${diameter}BI${bi}At${hour}.csv" "../${diameter}mResult/channelUsageSAMBA${scenario}${diameter}BI${bi}At${hour}.csv" "../${diameter}mResult/channelUsage${scenario}${diameter}BI${bi}At${hour}.pdf" 
	
fi
