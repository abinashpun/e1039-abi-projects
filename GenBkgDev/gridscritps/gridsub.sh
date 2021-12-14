#!/bin/bash

dir_macros=$(dirname $(readlink -f $BASH_SOURCE))
LIFE_TIME=medium # short (3h), medium (8h) or long (23h)
PROJECT_NAME="bkg_study"

jobname=$1
do_sub=$2
njobs=$3
nevents=$4

echo "njobs=$njobs"
echo "nevents=$nevents"
if [ $do_sub == 1 ]; then
    echo "Grid mode."
    if ! which jobsub_submit &>/dev/null ; then
	echo "Command 'jobsub_submit' not found."
	echo "Forget 'source /e906/app/software/script/setup-jobsub-spinquest.sh'?"
	exit
    fi
    work=/pnfs/e1039/persistent/users/$USER/$PROJECT_NAME/$jobname
    ln -sf /pnfs/e1039/persistent/users/$USER/$PROJECT_NAME data
else
    echo "Local mode."
    work=$dir_macros/scratch/$jobname
fi

mkdir -p $work
chmod -R 01755 $work

cd $dir_macros
tar -czvf $work/input.tar.gz ../*.C ../support/*.cfg
cd -

for (( id=1; id<=$njobs; id++ ))
do  
  mkdir -p $work/$id/log
  mkdir -p $work/$id/out
  chmod -R 01755 $work/$id

  rsync -av $dir_macros/gridrun.sh $work/$id/gridrun.sh

  if [ $do_sub == 1 ]; then
    cmd="jobsub_submit --grid"
    cmd="$cmd --memory=2GB"
    cmd="$cmd --disk=2GB"
    cmd="$cmd -l '+SingularityImage=\"/cvmfs/singularity.opensciencegrid.org/e1039/e1039-sl7:latest\"'"
    cmd="$cmd --append_condor_requirements='(TARGET.HAS_SINGULARITY=?=true)'"
    cmd="$cmd --use_gftp --resource-provides=usage_model=DEDICATED,OPPORTUNISTIC,OFFSITE -e IFDHC_VERSION --expected-lifetime='$LIFE_TIME'"
    cmd="$cmd --mail_never"
    cmd="$cmd --lines '+FERMIHTC_AutoRelease=True'"
    cmd="$cmd --lines '+FERMIHTC_GraceLifetime=7200'" #2 hours of grace lifetime
    cmd="$cmd -L $work/$id/log/log.txt"
    cmd="$cmd -f $work/input.tar.gz"
    cmd="$cmd -d OUTPUT $work/$id/out"
    cmd="$cmd file://`which $work/$id/gridrun.sh` $nevents $id"
    echo "$cmd"
    $cmd
  else
    mkdir -p $work/$id/input
    rsync -av $work/input.tar.gz $work/$id/input
    cd $work/$id/
    $work/$id/gridrun.sh $nevents $id | tee $work/$id/log/log.txt
    cd -
  fi
done 2>&1 | tee log_gridsub.txt
