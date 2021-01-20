#!/bin/bash

echo "Convert an OpenStreetMap file to sumo file"
outputString=$2

netconvert --type-files $SUMO_HOME/data/typemap/osmNetconvert.typ.xml,$SUMO_HOME/data/typemap/osmNetconvertUrbanDe.typ.xml,$SUMO_HOME/data/typemap/osmPolyconvert.typ.xml --osm-files "$1" --output-file "${outputString}.net.xml" --geometry.remove --roundabouts.guess --ramps.guess --junctions.join --tls.guess-signals --tls.discard-simple --tls.join

polyconvert --net-file "${outputString}.net.xml" --osm-files "$1" --type-file $SUMO_HOME/data/typemap/osmPolyconvert.typ.xml -o "${outputString}.poly.xml" 
echo "Done"
