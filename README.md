# simple-log
简易C++异步日志系统

[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://opensource.org/licenses/MIT)

## Introduction 
简易的C++异步日志，采用了日志缓冲区的方式提高效率，写入压力大时，缓冲区个数可自动扩展  
通常情况工作时为双缓冲区  
性能有待提高   

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