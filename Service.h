// Service.h
//
// Author: Mikko Saarinki
// Copyright (c) 2016 Mikko Saarinki. All rights reserved.
//
#pragma once
#include "ServiceUtils.h"

#include <memory>

/**
* Generic Windows service and lifetime.
* Use this for converting well behaved applications to Windows services.
* Wrapping is done through templating for robustness.
*
* See the ExampleApp.h for the implicit interface.
*/
template <class TApp>
class Service
{
public:
    Service()
    {
        // These SERVICE_STATUS members remain as set here
        mStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        mStatus.dwServiceSpecificExitCode = 0;
    }

    /**
    * Entry point for the service to be launched.
    * Can be used directly from service main.
    * Returns when the service has stopped.
    */
    void launch(LPCTSTR serviceName)
    {
        // Register HandlerEx function for the service
        mStatusHandle = RegisterServiceCtrlHandlerEx(
            serviceName,
            &Service::controlHandler,
            this);

        if (!mStatusHandle)
            return setStatus(SERVICE_STOPPED, ERROR_APP_INIT_FAILURE, 0);

        // Report initial status to the SCM
        setStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

        // Perform service-specific initialization and work.
        mApp = std::make_unique<TApp>();

        if (mApp->state())
            return setStatus(SERVICE_STOPPED, ERROR_APP_INIT_FAILURE, 0);

        // Report running status when initialization is complete.
        setStatus(SERVICE_RUNNING, NO_ERROR, 0);

        // Begin service
        if (mApp->run())
            setStatus(SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR, 0);
        else
            setStatus(SERVICE_STOPPED, NO_ERROR, 0);
    }

private:
    /**
    * Called by SCM whenever a control code is sent to the service.
    * using the ControlService function.
    *
    * @param dwControl control code
    * @param lpContext user defined context
    */
    static DWORD WINAPI controlHandler(DWORD dwControl,
        DWORD dwEventType,
        LPVOID lpEventData,
        LPVOID lpContext)
    {
        switch (dwControl)
        {
        case SERVICE_CONTROL_STOP:
            if (lpContext) reinterpret_cast<Service*>(lpContext)->stop();
            break;
        case SERVICE_CONTROL_INTERROGATE:
            break;
        default:
            break;
        }
        return EXIT_SUCCESS;
    }

    /**
    * Sets the current service status and reports it to the SCM.
    *
    * @param dwCurrentState current state (see SERVICE_STATUS)
    * @param dwWin32ExitCode system error code
    * @param dwWaitHint estimated time for pending operation in milliseconds
    */
    void setStatus(DWORD dwCurrentState,
        DWORD dwWin32ExitCode,
        DWORD dwWaitHint)
    {
        // Fill in the SERVICE_STATUS structure.
        mStatus.dwCurrentState = dwCurrentState;
        mStatus.dwWin32ExitCode = dwWin32ExitCode;
        mStatus.dwWaitHint = dwWaitHint;
        mStatus.dwControlsAccepted = 0;
        mStatus.dwCheckPoint = 0;

        if (dwCurrentState != SERVICE_START_PENDING)
            mStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

        if (dwCurrentState != SERVICE_RUNNING ||
            dwCurrentState != SERVICE_STOPPED)
            mStatus.dwCheckPoint = mCheckPoint++;

        // Report the status of the service to the SCM.
        SetServiceStatus(mStatusHandle, &mStatus);
    }

    void stop()
    {
        setStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

        // Signal the service to stop.
        if (mApp) mApp->stop();

        setStatus(mStatus.dwCurrentState, NO_ERROR, 0);
    }

private:
    SERVICE_STATUS mStatus;
    SERVICE_STATUS_HANDLE mStatusHandle = nullptr;
    std::unique_ptr<TApp> mApp;
    DWORD mCheckPoint = 1;
};
