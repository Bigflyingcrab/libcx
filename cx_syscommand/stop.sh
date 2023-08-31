#!/bin/bash

RED="\\033[31m"
GREEN="\\033[32m"
YELLOW="\\033[33m"
BLACK="\\033[0m"
POS="\\033[20G"
ROOT=$(cd $(dirname $0); pwd)

# 获取cs.pid
if [ -e ${ROOT}/cs.pid ];
then
        NVRPID=`cat ${ROOT}/cs.pid`;
        PIDS=`ps -ef|grep ${NVRPID} |grep CommandProxy`;
else
        echo -e ${POS}${BLACK}[${RED} 停止服务失败，未找到pid文件！${BLACK}]
        exit -1;
fi

if [ -n "${PIDS}" ];
then
        kill -9 ${NVRPID};
        rm -rf ${ROOT}/cs.pid;
fi;
echo -e ${POS}${BLACK}[${GREEN} 停止服务成功！${BLACK}]
exit 0