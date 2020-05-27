#!/bin/bash

FOLDER="03"
#sed -i 's/threadCount="12"/threadCount="4"/g' /mnt/d/Alex/submission/config.xml

#SECONDS=0
#echo "4 ====== Start Time: FOLDER=${FOLDER}"
#date +"%T %N"

#./tgnews languages ../${FOLDER}/ > result.txt


#echo "4 ======Finish Time:"
#date +"%T %N"
#echo "Diff in SECONDS = ${SECONDS}"

#echo "========================================="

#sed -i 's/threadCount="4"/threadCount="6"/g' /mnt/d/Alex/submission/config.xml
#SECONDS=0
#echo "6 ====== Start Time:"
#date +"%T %N"

#./tgnews languages ../${FOLDER}/ > result.txt

#echo "6 ======Finish Time:"
#date +"%T %N"
#echo "Diff in SECONDS = ${SECONDS}"

echo "========================================="

sed -i 's/threadCount="6"/threadCount="12"/g' /mnt/d/Alex/submission/config.xml
SECONDS=0
echo "12 ====== Start Time:"
date +"%T %N"

./tgnews languages ../${FOLDER}/ > result.txt

echo "12 ======Finish Time:"
date +"%T %N"
echo "Diff in SECONDS = ${SECONDS}"