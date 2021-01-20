#!/bin/bash

scenario=$1

echo "Generate PostProcessCSVFileAggregateData javac file"
javac PostProcessWithOrWithoutTLSCSVFileAggregateData.java > /dev/null

if [ $? -ne 0 ]; then
	echo "Failed to compile PostProcessCSVFileAggregateData.java file"
else
	echo "Done!"
	java PostProcessWithOrWithoutTLSCSVFileAggregateData "../../results/${scenario}UrbanAggregateData-processed.csv" "../../results/withOrWithoutTLSAggregateData${scenario}.csv"
	java PostProcessWithOrWithoutTLSCSVFileAggregateData "../../results/${scenario}UrbanWithoutTLSAggregateData-processed.csv" "../../results/withOrWithoutTLSAggregateData${scenario}.csv"

	cd ../plotScripts
	Rscript aggregateData.R "../../results/withOrWithoutTLSAggregateData${scenario}.csv" "../../results/withOrWithoutTLSAggregateData${scenario}.pdf"
#	Rscript overhead.R "../../results/overhead${scenario}.csv" "../../results/overhead${scenario}.pdf"
	
fi
