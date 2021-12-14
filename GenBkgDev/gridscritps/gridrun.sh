#!/bin/bash

nevents=$1
job_id=$2

SET_CORE=/cvmfs/seaquest.opensciencegrid.org/seaquest/software/e1039/this-e1039.sh #set core you want to use
RUN_MACRO="Fun4Sim_genbkg.C" #set running macro here
ANA_MODULE=/cvmfs/seaquest.opensciencegrid.org/seaquest/users/apun/anamodule-inst  #set the module libs/include here (migrate to cvmfs area first)

if [ -z ${CONDOR_DIR_INPUT+x} ];
  then
    CONDOR_DIR_INPUT=./input;
    echo "CONDOR_DIR_INPUT is initiallized as $CONDOR_DIR_INPUT"
  else
    echo "CONDOR_DIR_INPUT is set to '$CONDOR_DIR_INPUT'";
fi

if [ -z ${CONDOR_DIR_OUTPUT+x} ];
  then
    CONDOR_DIR_OUTPUT=./out;
    mkdir -p $CONDOR_DIR_OUTPUT
    echo "CONDOR_DIR_OUTPUT is initiallized as $CONDOR_DIR_OUTPUT"
  else
    echo "CONDOR_DIR_OUTPUT is set to '$CONDOR_DIR_OUTPUT'";
fi

echo "hello, grid." | tee out.txt $CONDOR_DIR_OUTPUT/out.txt
echo "HOST = $HOSTNAME" | tee -a out.txt $CONDOR_DIR_OUTPUT/out.txt
pwd | tee -a out.txt $CONDOR_DIR_OUTPUT/out.txt

tar -xzvf $CONDOR_DIR_INPUT/input.tar.gz
ls -lh | tee -a out.txt $CONDOR_DIR_OUTPUT/out.txt


echo "SET_CORE = $SET_CORE"
source $SET_CORE

export LD_LIBRARY_PATH=$ANA_MODULE/lib:$LD_LIBRARY_PATH
export  ROOT_INCLUDE_PATH=$ANA_MODULE/include:$ROOT_INCLUDE_PATH

echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
echo " "
echo " "
echo "ROOT_INCLUDE_PATH=$ROOT_INCLUDE_PATH"


time root -b -q $RUN_MACRO\($nevents\)
RET=$?
if [ $RET -ne 0 ] ; then
    echo "Error in Running Macro: $RET"
    exit $RET
fi

mv *.root $CONDOR_DIR_OUTPUT/

echo "gridrun.sh finished!"
