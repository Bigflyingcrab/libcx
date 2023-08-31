#!/bin/bash

RED="\\033[31m"
GREEN="\\033[32m"
YELLOW="\\033[33m"
BLACK="\\033[0m"
POS="\\033[20G"
ROOT=$(cd $(dirname $0); pwd)
PIDS=""

# 判断配置文件是否存在
if [ ! -e ${ROOT}/cs.conf ];then echo -e ${POS}${BLACK}[${RED} 启动失败，未找到配置文件！${BLACK}]; exit 1; fi

# 判断程序文件是否存在
if [ ! -e ${ROOT}/CommandProxy ];then echo -e ${POS}${BLACK}[${RED} 启动失败，未找到程序文件！${BLACK}]; exit 1; fi

# 获取当前目录下的cs.pid文件
if [ -e ${ROOT}/cs.pid ];
then
        NVRPID=`cat ${ROOT}/cs.pid`;
        PIDS=`ps -ef|grep ${NVRPID} |grep CommandProxy`;
fi

if [ -n "${PIDS}" ]; then echo -e ${POS}${BLACK}[${RED} 启动失败，服务已经运行了！${BLACK}]; exit 1; else rm -rf ${ROOT}/cs.pid; fi;

# 启动程序
nohup ${ROOT}/CommandProxy ${ROOT}/cs.conf > /dev/null 2>&1 &
if [ $? -ne 0 ];then echo -e ${POS}${BLACK}[${RED} 启动失败，服务切换到后台运行出错！  ${BLACK}];exit 1; fi

# 循环判断程序是否启动成功
for i in {1..3}
do
        sleep 1
        if [ -e ${ROOT}/cs.pid ];
        then
                # 修改服务器的网络缓冲区大小，增加系统网络吞吐能力
                sysctl net.core.rmem_max=16777216;
                sysctl net.core.rmem_default=16777216;
                sysctl net.core.wmem_max=16777216;
                sysctl net.core.wmem_default=16777216;
                echo -e ${POS}${BLACK}[${GREEN} 启动成功！${BLACK}];
                exit 0;
        fi
done

echo -e ${POS}${BLACK}[${RED} 启动失败！${BLACK}]
exit 1