#include <LibTemplateCMake/LibTemplateCMake.h>

#include <cstdlib>
#include <cmath>
#include <iostream>

int main()
{
    std::cout << "Running test on the exported library!" << std::endl;

    LibTemplateCMake::summationClass sumClass;
    LibTemplateCMake::differenceClass diffClass;

    double tol = 1e-10;
    double op1 = 15.0;
    double op2 = 10.0;

    if( fabs(sumClass.doSomething(op1, op2) - (op1 + op2)) > tol )
    {
        std::cerr << "[ERR] sumClass.doSomething(" << op1 << "," << op2
                  << ") is equal to " << sumClass.doSomething(op1, op2)
                  << " instead of the expected " << op1 + op2 << std::endl;
        return EXIT_FAILURE;
    }

    if( fabs(diffClass.doSomething(op1, op2) - (op1 - op2)) > tol )
    {
        std::cerr << "[ERR] sumClass.doSomething(" << op1 << "," << op2
                  << ") is equal to " << diffClass.doSomething(op1, op2)
                  << " instead of the expected " << op1 - op2 << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*
#include <LibHeaderOnlyTemplateCMake/LibHeaderOnlyTemplateCMake.hpp>

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <mutex>
#include <condition_variable>

/**
* This example application does nothing but goes to sleep and
* waits for external stop signal from service.
*
* It implements the implicit interface required by Service class.
*
* Can be used as a template for your own applications.
* /
class ExampleApp
{
public:
    int run()
    {
        std::unique_lock<std::mutex> lock(mtx);
        sleeper.wait(lock, [this] { std::cout << "oh shit" << std::endl; return !running; });
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

#define SERVICENAME TEXT("ExampleApp")
#define DISPLAYNAME TEXT("Windows Service Example") //displayed in Windows Services
#define DESCRIPTION TEXT("Does nothing but sleeps and waits for service stop signal.")

int __cdecl _tmain(int argc, TCHAR *argv[])
{
    // If no args, just run our app code
    if (argc < 2)
    {
        Service<ExampleApp> service;
        service.launch(SERVICENAME);
        return EXIT_SUCCESS;
    }

    ServiceMain<ExampleApp>(SERVICENAME, DISPLAYNAME, DESCRIPTION, argv);
    return EXIT_SUCCESS;
}

*/