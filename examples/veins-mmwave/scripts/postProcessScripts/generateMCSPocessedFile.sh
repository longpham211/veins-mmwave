#!/bin/bash

scenario=$2
diameter=$1

javac ProcessMcsCsvFile.java
java ProcessMcsCsvFile "../${diameter}mResult/${scenario}.csv"
