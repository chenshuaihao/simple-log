# simple-log
简易C++异步日志(学习用)

[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://opensource.org/licenses/MIT)

## Introduction 
简易的C++异步日志，采用了日志缓冲区的方式提高效率，写入压力大时，缓冲区个数可自动扩展  
通常情况工作时为双缓冲区  
Init初始化时创建一个后台flush线程，用于写入log文件  
性能测试，已进行了单线程测试，多线程测试  
增加同步写入测试  
增加多生产者线程写入测试  

## Analysis & Optimization
* 一开始时，经测试，发现同步竟然比异步写入要快！非常奇怪，经研究后发现是因为fwrite函数已经起到了缓冲作用，先是写到缓冲区，所以两者都有缓冲，性能差距不大，如果换成write就应该明显了。  
而本程序异步写入在fwrite之前还多了一次memcpy到Buffer的操作，所以更慢。  
因此，同步写入和异步写入的fwrite之后增加flush操作刷盘，就可以实现异步更快于同步了。  
* 优化性能，减少localtime()调用次数，当秒数不变时只需调用gettimeofday()更新毫秒就可以了，性能提高60%  
* 优化性能，去除logline初始化，因为snprintf会覆盖logline，所以无需初始化，性能提高17%  
* 优化性能，复用同一秒内的年月日时分秒的字符串，减少snprintf()中太多参数格式化的开销，性能提高30%  
* 多线程测试时，性能下降，猜测是加锁的原因，待优化  
* 优化多线程性能，每个线程独立拥有写入缓冲区，减少锁开销（只有map索引和queue时有锁），性能提高150%  
* 对比了存储线程缓冲区的数据结构map和unordered_map，最终使用map，map效率高40%，可能是因为数据量少时map效率更好  
* map的锁应该可以改为读写锁，性能应该提升会很大，之后有时间再改吧 暂时先这样吧  
  
## Envoirment  
* CPU: AMD Ryzen Threadripper 2990WX 32-Core Processor
* MEM: 128GB ddr4
* STORE: 500GB NVME
* OS: Ubuntu 16.04
* kernel: 4.15.0-72-generic (# uname -a)
* Complier: gcc version 5.4.0 20160609

## Build and Run
* 进入目录，执行：
  $ g++ -g -Wall -O2 -pthread -std=c++11 logtest.cpp logger.cpp  -o logtest
* 运行：
  $ ./logtest

## Simple Performance Test
单线程：1百万条log写入，耗时453ms，220w logs/second  
多线程：4线程各1百万条log写入，耗时1134ms，352w logs/second

## Other Ref Blog

[Ring Log](https://github.com/LeechanX/Ring-Log)  


Enjoy!