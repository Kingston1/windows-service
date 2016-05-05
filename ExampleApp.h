// Service.h
//
// Author: Mikko Saarinki
// Copyright (c) 2016 Mikko Saarinki. All rights reserved.
//
#pragma once
#include <mutex>
#include <condition_variable>

/**
* This example application does nothing but goes to sleep and
* waits for external stop signal from service.
*
* It implements the implicit interface required by Service class.
*
* Can be used as a template for your own applications.
*/
class ExampleApp
{
public:
    int run()
    {
        std::unique_lock<std::mutex> lock(mtx);
        sleeper.wait(lock, [this] { return !running; });
        return state();
    }

    void stop()
    {
        std::lock_guard<std::mutex> lock(mtx);
        running = false;
        sleeper.notify_all();
    }

    //@return 0 for A_OK and anything else for failures.
    int state() const { return EXIT_SUCCESS; }

private:
    mutable std::mutex mtx;
    std::condition_variable sleeper;
    bool running = true;
};
