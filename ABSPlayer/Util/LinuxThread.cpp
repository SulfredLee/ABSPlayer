#include "LinuxThread.h"
#include "Logger.h"

LinuxThread::LinuxThread()
{
    m_run = false;
    m_thread = 0;
}

LinuxThread::~LinuxThread()
{
    stopThread();
    if (m_thread) pthread_join(m_thread, NULL);
}

bool LinuxThread::startThread()
{
    int ret = pthread_create(&m_thread, NULL, LinuxThread::MainProxy, (void*)this);
    if (ret)
    {
        LOGMSG_ERROR("pthread_create fail, code: %d", ret);
        return false;
    }
    m_run = true;
    return true;
}

bool LinuxThread::isThreadRunning()
{
    return m_run;
}

void LinuxThread::stopThread()
{
    m_run = false;
}

void LinuxThread::joinThread()
{
    if (m_thread) pthread_join(m_thread, NULL);
    m_thread = 0;
}

void* LinuxThread::MainProxy(void* context)
{
    return ((LinuxThread* )context)->Main();
}
