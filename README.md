# simple-log
简易C++异步日志系统

[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://opensource.org/licenses/MIT)

## Introduction 
简易的C++异步日志，采用了日志缓冲区的方式提高效率，写入压力大时，缓冲区个数可自动扩展  
通常情况工作时为双缓冲区  
Init初始化时创建一个后台flush线程，用于写入log文件  
性能有待提高，已进行了单线程测试，多线程尚未进行  
增加同步写入测试  

## Analysis
一开始时，经测试，发现同步竟然比异步写入要快！非常奇怪，经研究后发现是因为fwrite函数已经起到了缓冲作用，先是写到缓冲区，所以两者都有缓冲，性能差距不大，如果换成write就应该明显了。  
而本程序异步写入在fwrite之前还多了一次memcpy到Buffer的操作，所以更慢。  
因此，同步写入和异步写入的fwrite之后增加flush操作刷盘，就可以实现异步更快于同步了。  

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