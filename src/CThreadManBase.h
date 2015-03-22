#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

class CThreadManBase
{
public:

    /**
     * @brief CThreadManBase constructor.
     */
    CThreadManBase() : stop_request(false), the_thread(), thread_is_running(false) {}

    /**
     * @brief CThreadManBase destructor.
     */
    virtual ~CThreadManBase()
    {
        if (the_thread.joinable())
        {
            stop_request = true; // request the_thread to stop
            the_thread.join();
        }
    }

    /** Start the thread execution (ThreadMain())
     * @brief Start the thread
     * @throw runtime_error if thread is already running
     */
    virtual void Start()
    {
        // wait until the existing thread terminates
        if (the_thread.joinable())
            throw(std::runtime_error("Thread is already running."));

        std::lock_guard<std::mutex> lck(mutex_running);
        thread_is_running = true; /// the_thread main to set while its running

        // create new thread and move its ownership to the_thread
        the_thread = std::thread(CThreadManBase::ThreadMainWrapper,this);
    }

    /**
     * @brief Returns true if Running
     * @return true if thread is running
     */
    virtual bool Running() const
    {
        return the_thread.joinable() && thread_is_running;
    }

    /**
     * @brief Stops the thread if running, blocks calling thread
     *        until the_thread is stopped.
     */
    virtual void Stop()
    {
        if (the_thread.joinable())
        {
            stop_request = true; // request the_thread to stop
            the_thread.join();
        }
    }

    void WaitTillDone()
    {
        std::unique_lock<std::mutex> lck(mutex_running);

        // if not running exit
        if (!thread_is_running) return;

        // if running, block until unlocked
        while (thread_is_running) cv_running.wait(lck);
    }

protected:
    /**
     * @brief Thread's Main function. Shall be implemented by derived class
     */
    virtual void ThreadMain()=0;

    bool stop_request;   /// a calling thread to request the running thread terminate immediately

private:

    std::thread the_thread;
    bool thread_is_running; /// the_thread main to set while its running
    std::condition_variable cv_running; // condition variable to wait for unlock
    std::mutex mutex_running; // mutex to protect lock_sign

    static void ThreadMainWrapper(CThreadManBase *obj)
    {
        std::unique_lock<std::mutex> lck(obj->mutex_running,std::defer_lock);
        try
        {
            obj->ThreadMain();
        }
        catch (...)
        {
            // clear the running flag and notify the waiting threads
            lck.lock();
            obj->thread_is_running = false;
            obj->cv_running.notify_all();
            throw;
        }

        lck.lock();
        obj->thread_is_running = false;
        obj->cv_running.notify_all();
    }
};
