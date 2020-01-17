// Copyright 2020, Chen Shuaihao.
//
// Author: Chen Shuaihao
//
// -----------------------------------------------------------------------------
// File: logger.h
// -----------------------------------------------------------------------------
//

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <sys/time.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#include <iostream>
#include <queue>
#include <map>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>

#define BUFSIZE 8*1024*1024 //16MB
#define LOGLINESIZE 4096 //4KB
#define MEM_LIMIT 512*1024*1024 //512MB

class Logger;

#define LOG_INIT(logdir, lev) \
    do { \
        Logger::GetInstance()->Init(logdir, lev); \
    } while (0)

#define LOG(level, fmt, ...) \
    do {  \
        if(Logger::GetInstance()->GetLevel() <= level) {  \
            Logger::GetInstance()->Append(level, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__);  \
        }  \
    } while (0)  
    
enum LoggerLevel {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

//const char * LevelString[5] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};

class LogBuffer
{
public:
    enum BufState { FREE = 0, FLUSH = 1 }; //FREE 空闲状态 可写入日志, FLUSH 待写入或正在写入文件    
    LogBuffer(int size = BUFSIZE);
    ~LogBuffer();
    int Getusedlen() const {
        return usedlen;
    }
    int GetAvailLen() const {
        return bufsize - usedlen;
    }
    int GetState() const {
        return state;
    }
    void SetState(BufState s) {
        state = s;
    }
    void append(const char *logline, int len);
    void FlushToFile(FILE *fp);

private:
    //log缓冲区
    char *logbuffer;
    //log缓冲区总大小
    uint32_t bufsize;
    //log缓冲区used长度
    uint32_t usedlen;
    //缓冲区状态
    int state;
};


class Logger
{
private:
    //日志等级
    int level;
    //打开的日志文件指针
    FILE *fp;
    //当前使用的缓冲区
    //LogBuffer *currentlogbuffer;
    //std::unordered_map<std::thread::id, LogBuffer *> threadbufmap;
    std::map<std::thread::id, LogBuffer *> threadbufmap;
    //mutex
    std::mutex mtx;
    //缓冲区总数
    int buftotalnum;
    //flushmutex
    std::mutex flushmtx;
    //flushcond
    std::condition_variable flushcond;
    //flush队列
    std::queue<LogBuffer*> flushbufqueue;
    //freeutex
    std::mutex freemtx;
    //FREE队列
    std::queue<LogBuffer*> freebufqueue;
    //flush thread
    std::thread flushthread;
    //flushthread state
    bool start;
    //save_ymdhms数组，保存年月日时分秒以便复用
    char save_ymdhms[64];
    //tid hash
    //std::hash<std::thread::id> tid_hash;

public:
    Logger(/* args */);
    ~Logger();

    //单例模式
    static Logger * GetInstance() {
        static Logger logger;
        return &logger;
    }

    //初始化
    void Init(const char* logdir, LoggerLevel lev);

    //获取日志等级
    int GetLevel() const {
        return level;
    }

    //写日志__FILE__, __LINE__, __func__,
    void Append(int level, const char *file, int line, const char *func, const char *fmt, ...);

    //flush func
    void Flush();
};

#endif //_LOGGER_H_