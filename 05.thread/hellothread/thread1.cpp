#include <iostream>
#include <thread>
#include <mutex>
#include "../../04.timer/CELLTimestamp.h"
#include<atomic>
using namespace std;

# define TNUM   4
mutex mut;
int sum = 0;
atomic<int> num; // 定义原子变量，锁定变量
int Thread1(int t_id)
{
    for (int i = 0; i < 4; i++)
    {
        mut.lock();
        cout << t_id << " hello, Thread1..." << i << endl; /* code */
        mut.unlock();
    }
}

int Thread2(int t_id)
{
    for (int i = 0; i < 10000000; i++)
    {
        mut.lock();
        sum++;   /* code */
        mut.unlock();
    }
    
}
int Thread3(int t_id)
{
    for (int i = 0; i < 10000000; i++)
    {
        // 启动自解锁
        lock_guard<mutex> lg(mut);
        sum++;   /* code */
    }
    
}

void test1()
{
    // 创建线程
    thread t(Thread1, 0);
    //t.detach();
    t.join();
}

// 创建多个线程
void test2()
{
    thread t[TNUM];
    for (int i = 0; i < TNUM; i++)
    {
        t[i] = thread(Thread2, i);
        //t[i].detach();
    }
    CELLTimestamp timer1;
    for (int i = 0; i < TNUM; i++)
    {
        t[i].join();
    }
    cout << timer1.getElapsedMilliSec()<< endl;
}

int main()
{
    //test1();
    test2();
    cout << sum << endl;
    int sum2 = 0;
    CELLTimestamp timer2;
    for (int i = 0; i < 40000000; i++)
    {
        sum2++;   /* code */
    }
    cout << timer2.getElapsedMilliSec() << endl;
    cout << sum2 << endl;
    
    cout << "hello, main thread..." << endl;
    // while(1);
    return 0;
}
