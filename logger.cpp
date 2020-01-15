// Copyright 2020, Chen Shuaihao.
//
// Author: Chen Shuaihao
//
// -----------------------------------------------------------------------------
// File: logger.cpp
// -----------------------------------------------------------------------------
//

#include "logger.h"

#include <thread>

const char * LevelString[5] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};
//LogBuffer

LogBuffer::LogBuffer(int size) :
    bufsize(size),
    usedlen(0),
    state(BufState::FREE)
{
    logbuffer = new char[bufsize];
    if(logbuffer == nullptr) {
        std::cerr << "mem alloc fail: new char!" << std::endl;
    }
}

LogBuffer::~LogBuffer() {
    if(logbuffer != nullptr) {
        delete [] logbuffer;
    }
}

void LogBuffer::append(const char *logline, int len) {
    memcpy(logbuffer + usedlen, logline, len);
    usedlen += len;
}

void LogBuffer::FlushToFile(FILE *fp) {
    uint32_t wt_len = fwrite(logbuffer, 1, usedlen, fp);
    if(wt_len != usedlen) {
        std::cerr << "fwrite fail!" << std::endl;
    }
    usedlen = 0;
    fflush(fp);
}

//Logger

Logger::Logger(/* args */) :
    level(LoggerLevel::INFO),
    fp(nullptr),
    currentlogbuffer(nullptr),
    buftotalnum(0),
    start(false)
{
}

Logger::~Logger()
{
    std::cout << "~Logger" << std::endl;
    //shutdown flush thread
    std::lock_guard<std::mutex> lock(mtx);
    currentlogbuffer->SetState(LogBuffer::BufState::FLUSH);
    {
        std::lock_guard<std::mutex> lock2(flushmtx);
        flushbufqueue.push(currentlogbuffer); 
        flushcond.notify_one();               
    }
    start = false;
    flushcond.notify_one();
    if(flushthread.joinable())
        flushthread.join();

    if(fp != nullptr) {
        fclose(fp);
    }
    for(uint32_t i = 0; i < freebufqueue.size(); ++i) {
        LogBuffer* p = freebufqueue.front();
        freebufqueue.pop();
        delete p;
    }
    //if(currentlogbuffer && currentlogbuffer->GetState() == LogBuffer::BufState::FREE) {
    //    delete currentlogbuffer;
    //}
}

void Logger::Init(const char* logdir, LoggerLevel lev) {
    //alloc logbuf
    //logbuffer = new char[bufsize];
    currentlogbuffer = new LogBuffer(BUFSIZE);
    buftotalnum++;

    //get time
    time_t t = time(nullptr);
    struct tm *ptm = localtime(&t);

    //create logfilename
    char logfilepath[256] = {0};
    snprintf(logfilepath, 255, "%s/log_%d_%d_%d", logdir,
        ptm->tm_year+1900, ptm->tm_mon+1,
        ptm->tm_mday);
    level = lev;
        
    fp = fopen(logfilepath, "w+");
    if(fp == nullptr) {
        printf("logfile open fail!\n");
    }

    //create flush thread
    flushthread = std::thread(&Logger::Flush, this);
    return ;
}

void Logger::Append(int level, const char *file, int line, const char *func, const char *fmt, ...) {
    //单行日志
    char logline[LOGLINESIZE] = {0};
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t t = tv.tv_sec;
    static time_t lastsec = 0;
    static struct tm *ptm = nullptr;
    //性能优化，秒数不变则不调用localtime
    if(lastsec != t) {
        ptm = localtime(&t);  
        lastsec = t;      
    }

    uint32_t n = snprintf(logline, LOGLINESIZE, "[%s][%04d-%02d-%02d %02d:%02d:%02d.%03ld]%s:%d(%s): ", LevelString[level], ptm->tm_year+1900, \
        ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, tv.tv_usec/1000, file, line, func);


    va_list args;
    va_start(args, fmt);
    int m = 0;
    m = vsnprintf(logline + n, LOGLINESIZE - n, fmt, args);
    va_end(args);

    //append to buf
    int len = n + m;
    std::lock_guard<std::mutex> lock(mtx);
    //空间足够
    if(currentlogbuffer->GetAvailLen() >= len && currentlogbuffer->GetState() == LogBuffer::BufState::FREE) {
        currentlogbuffer->append(logline, len);
    }
    //空间不足，new新的缓冲区
    else {
        if(currentlogbuffer->GetState() == LogBuffer::BufState::FREE) {
            currentlogbuffer->SetState(LogBuffer::BufState::FLUSH);
            {
                std::lock_guard<std::mutex> lock2(flushmtx);
                flushbufqueue.push(currentlogbuffer); 
                flushcond.notify_one();               
            }
            //currentlogbuffer = nullptr;
            //从FREE队列取buf
            std::lock_guard<std::mutex> lock3(freemtx);
            if(!freebufqueue.empty()) {  
                //std::cout << "get LogBuffer from freebufqueue" << buftotalnum << std::endl;
                currentlogbuffer = freebufqueue.front();    
                freebufqueue.pop();            
            }
            //new buf
            else {
                //日志缓冲占用的内存没有到达上限
                if(buftotalnum * BUFSIZE < MEM_LIMIT) {
                    currentlogbuffer = new LogBuffer(BUFSIZE);
                    buftotalnum++;
                    std::cout << "create new LogBuffer:" << buftotalnum << std::endl;
                }
                else {
                    ;//无空间了丢弃日志
                    std::cout << "drop log!"<< std::endl;
                    return;
                }            
            }
            //if(currentlogbuffer)
                currentlogbuffer->append(logline, len);             
        }
        else {
            //curbuf在flushbufqueue中，写入文件，更新
            std::lock_guard<std::mutex> lock3(freemtx);
            if(!freebufqueue.empty()) {            
                currentlogbuffer = freebufqueue.front();    
                freebufqueue.pop();              
            }
        }
    }
}

void Logger::Flush() {
    start = true;
    while (true) {
        /* code */
        LogBuffer *p;
        {
            std::unique_lock<std::mutex> lock(flushmtx);
            while(flushbufqueue.empty() && start) {
                flushcond.wait(lock);
            }     
            //日志关闭，队列为空
            if(flushbufqueue.empty() && start == false)
                return ;
            p = flushbufqueue.front();
            flushbufqueue.pop();       
        }
        p->FlushToFile(fp);
        p->SetState(LogBuffer::BufState::FREE);
        {
            std::lock_guard<std::mutex> lock2(freemtx);
            freebufqueue.push(p);
        }
    }
    
}
