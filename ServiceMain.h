// ServiceMain.h
//
// Author: Mikko Saarinki
// Copyright (c) 2016 Mikko Saarinki. All rights reserved.
//
#pragma once
#include "ServiceUtils.h"
#include "Service.h"

/**
* Windows service on stack.
*
* All you need to do in ordinary process main for it to become a full blown service:
* ServiceMain<ExampleApp> service(SERVICENAME, SERVICEDISPLAYNAME, SERVICEDESCRIPTION, argv);
*/
template <class TApp>
struct ServiceMain
{
    ServiceMain(LPTSTR serviceName,
        LPCTSTR displayName,
        LPTSTR description,
        TCHAR *argv[])
    {
        ServiceUtils::Setup setup;
        if (setup.options(serviceName, displayName, description, argv))
        {
            std::cout << "Service setup failure!" << std::endl;
            return;
        }

        /**
        * Entry point for the service.
        *
        * @param dwArgc Number of arguments in the lpszArgv array
        * @param lpszArgv Array of strings. The first string is the name of the service
        *        and subsequent strings are passed by the process that called the StartService
        *        function to start the service.
        */
        auto serviceMain = [](DWORD dwArgc, LPTSTR* lpszArgv) {
            Service<TApp> service;
            service.launch(lpszArgv[0]);
        };

        SERVICE_TABLE_ENTRY DispatchTable[] =
        {
            { serviceName, static_cast<LPSERVICE_MAIN_FUNCTION>(serviceMain) },
            { NULL, NULL }
        };

        // This call returns when the service has stopped and process should simply terminate.
        if (!StartServiceCtrlDispatcher(DispatchTable))
        {
            auto error = GetLastError();
            if (ERROR_FAILED_SERVICE_CONTROLLER_CONNECT != error) //not an error, we are in console
                std::cout << "StartServiceCtrlDispatcher error: " << ServiceUtils::win32Error(error) << std::endl;
        }
    }
};