#!/bin/bash
#
#kick off n clients for testing 
#
#usage: ./multiclient.sh n-clients output-file n-objects-each-client
#
outname="$2"
max=$3
if [ "$1" = "" ]
then 
  nclients=1
else
  nclients=$1
fi

i=0
while [ $i -lt $nclients ]
do  
  i=$[ $i + 1 ]
  if [ $i -le 9 ]
  then
    number="00"$i
  elif [ $i -le 99 ]
  then
    number="0"$i
  elif [ $i -le 999 ]
  then
    number=$i
  else
    echo are you sure you want to start more than 999 clients???? 
    exit
  fi
echo "./squidcache.py -o ${outname}_${number}.dat cdfdbfrontier.fnal.gov 8000 $ cdf_cid_table_list_1.pkl {max} > ${outname}_${number}.log&"
./squidcache.py -o ${outname}_${number}.dat cdfdbfrontier.fnal.gov 8000  cdf_cid_table_list_1.pkl ${max} > ${outname}_${number}.log&
done
