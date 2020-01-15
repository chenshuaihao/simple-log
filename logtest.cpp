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

int main(int argc, char** argv)
{
    LOG_INIT(".", LoggerLevel::INFO);
    uint64_t start_ts = get_current_millis();
    for (int i = 0;i < 1000000; ++i)
    {
        LOG(LoggerLevel::ERROR, "log test %d\n", i);
    }
    uint64_t end_ts = get_current_millis();
    printf("1 million times logtest, time use %lums\n", end_ts - start_ts);
}