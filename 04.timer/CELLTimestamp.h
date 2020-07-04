#ifndef CELLTIMESTAMP_H
#define CELLTIMESTAMP_H

#include <chrono>
using namespace std::chrono;

class CELLTimestamp
{
private:
    time_point<high_resolution_clock> _begin; //创建时间点

public:
    CELLTimestamp()
    {
        update();
    }

    // 更新时间
    void update()
    {
        _begin = high_resolution_clock::now();
    }

    // 获取当前秒
    double getElapsedSecond()
    {
        return this->getElapsedMicroSec() * 0.000001;
    }

    // 获取毫秒
    double getElapsedMilliSec()
    {
        return this->getElapsedMicroSec() * 0.001;
    }

    // 获取微秒时间差
    int64_t getElapsedMicroSec()
    {
        auto microSec = duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
        return microSec;
    }
};

#endif // CELLTIMESTAMP_H