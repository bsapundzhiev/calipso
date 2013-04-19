#!/bin/bash
#
# Calipso simple log parser
#

LOG=$1
OUTFILE="output.csv"

if [ -f "$LOG" ]; then
    echo "Processing '$LOG'..."
else
	echo "'$LOG' does not exist, use $0 <filename>"
exit 1
fi

#rm $OUTFILE
touch $OUTFILE
echo "Client,Count" > $OUTFILE;

for i in `sort $LOG | awk '{print $1}' | uniq | tr -d "[]"`; do
	COUNT=`grep -c $i $LOG`
	echo -e "$i,$COUNT" >> $OUTFILE;
done

#sort -k5n,5 $OUTFILE
