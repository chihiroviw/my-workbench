#! /bin/bash

cmd="${@} &"
eval $cmd
export LD_PRELOAD=./mygl.so
eval $cmd
export -n LD_PRELOAD
cp ./mygl.c ../CG_report/mygl.c