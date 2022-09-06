#!/bin/bash

_PREFIX=$(pwd)

_INPUT_COMMAN=$@

_LOAD_PATH="/home/zpw/HME-Quartz-broadwell-master/scripts/setupdev.sh"

_RUN_PATH="/home/zpw/HME-Quartz-broadwell-master/scripts/runenv.sh"

_ROOT="sudo"

_LOAD="load"

_RELOAD="reload"

echo ${_INPUT_COMMAN}
Init()
{
    ${_ROOT} bash ${_LOAD_PATH} ${_RELOAD}
    ${_ROOT} bash ${_LOAD_PATH} ${_LOAD}
}

Start()
{
    numactl --cpunodebind=0 --membind=0 bash ${_RUN_PATH} ${_INPUT_COMMAN}
}

Init
Start
