# simple-log
简易C++异步日志系统

[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://opensource.org/licenses/MIT)

## Introduction 
简易的C++异步日志，采用了日志缓冲区的方式提高效率，写入压力大时，缓冲区个数可自动扩展  
通常情况工作时为双缓冲区  
Init初始化时创建一个后台flush线程，用于写入log文件  
性能有待提高，已进行了单线程测试，多线程尚未进行  
增加同步写入测试  
经测试，发现同步竟然比异步写入要快，重大BUG，应该是设计出现问题  

## Envoirment  
* CPU: Intel(R) Core(TM) i3-3220 CPU @ 3.30GHz (2 logical cores per physical)
* MEM: 32GB ddr3
* OS: Ubuntu 16.04
* kernel: 4.15.0-45-generic (# uname -a)
* Complier: gcc version 5.4.0 20160609

## Build and Run
* 进入目录，执行：
  $ g++ -g -Wall -O2 -pthread -std=c++11 logtest.cpp logger.cpp  -o logtest
* 运行：
  $ ./logtest

## Simple Performance Test
1百万条log写入，耗时3187ms。

## Other Ref Blog

[Ring Log](https://github.com/LeechanX/Ring-Log)  


Enjoy!