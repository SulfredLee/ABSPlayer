#include "linuxThread.h"
#include "Logger.h"

linuxThread::linuxThread()
{
    m_run = false;
    m_thread = 0;
}

linuxThread::~linuxThread()
{
    stopThread();
    if (m_thread) pthread_join(m_thread, NULL);
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

void linuxThread::joinThread()
{
    if (m_thread) pthread_join(m_thread, NULL);
    m_thread = 0;
}

void* linuxThread::MainProxy(void* context)
{
    return ((linuxThread* )context)->Main();
}
