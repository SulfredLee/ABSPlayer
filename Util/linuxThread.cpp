#include "linuxThread.h"
#include "Logger.h"

linuxThread::linuxThread()
{
    m_run = false;
}

linuxThread::~linuxThread()
{
    stopThread();
    pthread_join(m_thread, NULL);
}

bool linuxThread::startThread()
{
    int ret = pthread_create(&m_thread, NULL, linuxThread::MainProxy, (void*)this);
    if (ret)
    {
        LOGMSG_ERROR("pthread_create fail, code: %d", ret);
        return false;
    }
    m_run = true;
    return true;
}

bool linuxThread::isThreadRunning()
{
    return m_run;
}

void linuxThread::stopThread()
{
    m_run = false;
}

void* linuxThread::MainProxy(void* context)
{
    return ((linuxThread* )context)->Main();
}
