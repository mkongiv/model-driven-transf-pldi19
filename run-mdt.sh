#!/bin/bash

if [ $# -ne 3 ]; then
  echo "Usage: ./run-ponos.sh <arg1> <arg2> <arg3>"
  echo "where: "
  echo "arg1 is \"benchmark filename (without .c extension)\""
  echo "arg2 is the schedule dimension in 2d+1 format"
  echo "arg3 is the architecture (SKX, KNL, PWR9)"
  #echo "arg3 is logging (1: ON), (2: OFF)"
  exit 1
fi

target=$1
dim=$2
tarch=$3
capr=0
caps=0

archlist="SKX KNL PWR9"
archvalid=0
for aa in $archlist; do 
  if [ $aa == $tarch ]; then
    archvalid=1
  fi
done
if [ $archvalid == 0 ]; then
  echo "Selected architecture is invalid. Aborting compilation ..."
  exit
fi

if [ $dim -lt 1 ]; then
  echo "Schedule dimension given was 0 (zero). Aborting ..."
  exit
fi
 
args=""
args+=" --ponos "
args+=" --ponos-solver cplex "
#args+=" --ponos-pip-gmp "

args+=" --ponos-solver-pre "
args+=" --ponos-farkas-max "
args+=" --ponos-farkas-nored "

args+=" --ponos-coef-N "
args+=" --ponos-build-2dp1 "
args+=" --ponos-obj codelet "
args+=" --ponos-sched-sz $dim  "
args+=" --pragmatizer "
args+=" --default-ctxt "
args+=" --ponos-chunked-arch $tarch "

base_args=$args

args+=" --ponos-chunked "
args+=" --ponos-chunked-auto "
args+=" --ponos-chunked-unroll"
args+=" --ponos-chunked-loop-max-refs $capr "
args+=" --ponos-chunked-loop-max-stmt $caps "
args+=" --ponos-coef 30 "
args+=" --ponos-K 40 "

echo "Generating chunked variant ..." 
cmd="time ./bin/pocc $args $target.c -o $target.chunked.c "
echo $cmd
x=`date`
echo "Start time: $x"

timestart=`date`
{ time ./bin/pocc $args $target.c -o $target.chunked.c &> $target.log ; } 2> $target.time
timeend=`date`
tempfile=$target.chunked.c

echo "" >> $tempfile
echo "/*" >> $tempfile
cat $target.log >> $tempfile
echo "" >> $tempfile
cat $target.time >> $tempfile
echo "Time start  : $timestart" >> $tempfile
echo "Time finish : $timeend" >> $tempfile
echo "*/" >> $tempfile
mv $tempfile $target.chunked.$tarch.c

