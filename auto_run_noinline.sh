#!/usr/bin/env bash


echo 'Start auto test - eraser'

cd G-parser && sh auto_run_noinline.sh $1 $2 && cd ..

cd pass && sh auto_run.sh && cd ..

cd rtLib && sh auto_run.sh && cd ..




dir=$PWD

#if [[ -z "${DEPLOY_ENV}" ]]; then
#  echo 'skip test'
#
#else
#   cd /home/cxworks/workspace/splash2/splash2-origin/codes && sh auto_run.sh && cd /home/cxworks/workspace/splash2/splash2-llvm/codes && sh auto_run.sh && cd $dir
#fi

echo 'End building eraser'

#echo 'Start auto test - use_after_free'
#
#cd G-parser && sh auto_run.sh use_after_free && cd ..
#
#cd eraser && sh auto_run.sh && cd ..
#
#cd rtLib && sh auto_run.sh && cd ..
#
#if [[ -z "${DEPLOY_ENV}" ]]; then
#  echo 'skip test'
#
#else
#   cd /home/cxworks/workspace/splash2/splash2-origin/codes && sh auto_run.sh && cd /home/cxworks/workspace/splash2/splash2-llvm/codes && sh auto_run.sh && cd $dir
#fi
#
#echo 'End building use_after_free'
