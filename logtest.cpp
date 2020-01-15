// Copyright 2020, Chen Shuaihao.
//
// Author: Chen Shuaihao
//
// -----------------------------------------------------------------------------
// File: logtest.cpp
// -----------------------------------------------------------------------------
//

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "logger.h"

int64_t get_current_millis(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void log(int i, FILE *fp) {
    char logline[LOGLINESIZE] = {0};
    time_t t = time(nullptr);
    struct tm *ptm = localtime(&t);

    int n = snprintf(logline, LOGLINESIZE, "[%s][%04d-%02d-%02d %02d:%02d:%02d]%s:%d(%s): log test %d\n", "ERROR", ptm->tm_year+1900, 
        ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, __FILE__, __LINE__, __FUNCTION__, i);

    uint32_t wt_len = fwrite(logline, 1, n, fp);
    if(wt_len != n) {
        std::cerr << "fwrite fail!" << std::endl;
    }   
}

void synctest() {
    printf("1 million times logtest synctest...\n");
    char logfilepath[256] = "./log_synctest";
    FILE *fp = fopen(logfilepath, "w+");
    if(fp == nullptr) {
        printf("logfile open fail!\n");
    }
    uint64_t start_ts = get_current_millis();
    for (int i = 0;i < 10000000; ++i)
    {
        //LOG(LoggerLevel::ERROR, "log test %d\n", i);
        log(i, fp);
    }
    uint64_t end_ts = get_current_millis();
    fclose(fp);
    printf("1 million times logtest, time use %lums\n", end_ts - start_ts);
}

int main(int argc, char** argv)
{
    //synctest
    synctest();

    LOG_INIT(".", LoggerLevel::INFO);
    uint64_t start_ts = get_current_millis();
    for (int i = 0;i < 10000000; ++i)
    {
        LOG(LoggerLevel::ERROR, "log test %d\n", i);
    }
    uint64_t end_ts = get_current_millis();
    printf("1 million times logtest, time use %lums\n", end_ts - start_ts);
}