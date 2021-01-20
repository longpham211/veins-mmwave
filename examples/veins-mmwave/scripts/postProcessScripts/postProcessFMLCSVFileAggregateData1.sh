#!/bin/bash

scenario=$1

echo "Generate PostProcessCSVFileAggregateData javac file"
javac PostProcessFMLCSVFileAggregateData.java > /dev/null

if [ $? -ne 0 ]; then
	echo "Failed to compile PostProcessCSVFileAggregateData.java file"
else
	echo "Done!"
	java PostProcessFMLCSVFileAggregateData "../../results/FML${scenario}FactorControl0AggregateData-processed.csv" "../../results/fmlAggregateData${scenario}.csv"
	java PostProcessFMLCSVFileAggregateData "../../results/FML${scenario}FactorControl1AggregateData-processed.csv" "../../results/fmlAggregateData${scenario}.csv"
	java PostProcessFMLCSVFileAggregateData "../../results/FML${scenario}FactorControl2AggregateData-processed.csv" "../../results/fmlAggregateData${scenario}.csv"
	java PostProcessFMLCSVFileAggregateData "../../results/FML${scenario}FactorControl3AggregateData-processed.csv" "../../results/fmlAggregateData${scenario}.csv"
	java PostProcessFMLCSVFileAggregateData "../../results/FML${scenario}FactorControl4AggregateData-processed.csv" "../../results/fmlAggregateData${scenario}.csv"
	java PostProcessFMLCSVFileAggregateData "../../results/FML${scenario}FactorControl5AggregateData-processed.csv" "../../results/fmlAggregateData${scenario}.csv"
	java PostProcessFMLCSVFileAggregateData "../../results/FML${scenario}FactorControl6AggregateData-processed.csv" "../../results/fmlAggregateData${scenario}.csv"
	java PostProcessFMLCSVFileAggregateData "../../results/FML${scenario}FactorControl7AggregateData-processed.csv" "../../results/fmlAggregateData${scenario}.csv"
	java PostProcessFMLCSVFileAggregateData "../../results/FML${scenario}FactorControl8AggregateData-processed.csv" "../../results/fmlAggregateData${scenario}.csv"
	java PostProcessFMLCSVFileAggregateData "../../results/FML${scenario}FactorControl9AggregateData-processed.csv" "../../results/fmlAggregateData${scenario}.csv"

	cd ../plotScripts
	Rscript aggregateData.R "../../results/fmlAggregateData${scenario}.csv" "../../results/fmlAggregateData${scenario}.pdf"
#	Rscript overhead.R "../../results/overhead${scenario}.csv" "../../results/overhead${scenario}.pdf"
	
fi
