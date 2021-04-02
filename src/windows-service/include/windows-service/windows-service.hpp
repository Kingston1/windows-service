#ifndef LIB_TEMPLATE_CMAKE_HEADER_ONLY_H
#define LIB_TEMPLATE_CMAKE_HEADER_ONLY_H

#include <windows.h>
#include <tchar.h>
#include <comdef.h> //for _com_error

#include <iostream>
#include <string>
#include <functional>
#include <memory>



/**
 * \ingroup LibTemplateCMake_namespace
 *
 * LibTemplateCMake namespace.
 */
namespace LibTemplateCMake {

    /**
     * @brief Computes the sum of two values.
     * @param[in] op1 first input value.
     * @param[in] op2 second input value.
     * @returns sum of op1 and op2.
     */
    template<typename DatumType>
    DatumType sum(DatumType op1, DatumType op2)
    {
        return (op1 + op2);
    }


    /**
     * @brief Computes the difference of two values.
     * @param[in] op1 first input value.
     * @param[in] op2 second input value.
     * @returns difference of op1 and op2.
     */
    template<typename DatumType>
    DatumType sub(DatumType op1, DatumType op2)
    {
        return (op1 - op2);
    }

} // namespace LibTemplateCMake

#define _BUFFER_SIZE 256

// ServiceUtils.h
//
// Author: Mikko Saarinki
// Source: https://github.com/Kingston1/windows-service
// License: MIT 03/31/2021
//

namespace ServiceUtils
{
    //why microsoft doesn't provide human readable and easy
    //conversion from their silly macro world is beyond comprehension

    std::string t_convert(const TCHAR *argh)
    {
#ifdef UNICODE
        size_t i = 0;
        char *buf = static_cast<char*>(malloc(_BUFFER_SIZE));

        // Conversion
        wcstombs_s(&i,
            buf, static_cast<size_t>(_BUFFER_SIZE),
            argh, static_cast<size_t>(_BUFFER_SIZE));

        std::string r(buf);
        if (buf) free(buf); // Free multibyte character buffer
        return r;
#else
        return std::string(argh);
#endif
    }

    //convenience method for human readable GetLastError()
    std::string win32Error(DWORD error)
    {
        _com_error com_error(HRESULT_FROM_WIN32(error));
        auto msg = com_error.ErrorMessage();
        return t_convert(msg);
    }

    /**
    * Handler for service setup options given on process start.
    * These operations are convenience wrappers and not required by a service to run.
    * They can be also done using SC commands externally.
    */
    struct Setup
    {
        /**
        * Handle service setup options.
        *
        * @param serviceName This is a good idea to avoid confusion: executable == serviceName.
        * @param displayName Name displayed in Windows Services.
        * @param description Description or overview of the service shown in Windows Services.
        * @param argv command line arguments given. Only the first one is used or needed.
        */
        int options(LPCTSTR serviceName,
            LPCTSTR displayName,
            LPTSTR description,
            TCHAR *argv[]) const
        {
            if (!lstrcmpi(argv[1], TEXT("install")))
                return options(serviceName, displayName, description, Mode::Install);
            else if (!lstrcmpi(argv[1], TEXT("uninstall")))
                return options(serviceName, displayName, description, Mode::Uninstall);
            else if (!lstrcmpi(argv[1], TEXT("create")))
                return options(serviceName, displayName, description, Mode::Create);
            else if (!lstrcmpi(argv[1], TEXT("delete")))
                return options(serviceName, displayName, description, Mode::Delete);
            else if (!lstrcmpi(argv[1], TEXT("start")))
                return options(serviceName, displayName, description, Mode::Start);
            else if (!lstrcmpi(argv[1], TEXT("stop")))
                return options(serviceName, displayName, description, Mode::Stop);
            else help(serviceName);

            return EXIT_SUCCESS;
        }

    private:
        enum class Mode
        {
            Install,
            Uninstall,
            Create,
            Delete,
            Start,
            Stop
        };

        int options(LPCTSTR serviceName,
            LPCTSTR displayName,
            LPTSTR description,
            Mode mode) const
        {
            // Get a handle to the SCM database. 
            auto manager = OpenSCManager(
                NULL,                        // local computer
                NULL,                        // ServicesActive database 
                SC_MANAGER_ALL_ACCESS);      // full access rights

            if (!manager)
            {
                std::cout << "OpenSCManager error: " << win32Error(GetLastError()) << std::endl;
                return EXIT_FAILURE;
            }

            switch (mode)
            {
            case Mode::Install:
                create(serviceName, displayName, description, manager);
                start(serviceName, manager);
                break;
            case Mode::Uninstall:
                stop(serviceName, manager);
                tryDelete(serviceName, manager);
                break;
            case Mode::Create: create(serviceName, displayName, description, manager); break;
            case Mode::Delete: tryDelete(serviceName, manager); break;
            case Mode::Start:  start(serviceName, manager); break;
            case Mode::Stop:   stop(serviceName, manager); break;
            default: break;
            }

            CloseServiceHandle(manager);
            return EXIT_SUCCESS;
        }

        void create(LPCTSTR serviceName,
            LPCTSTR displayName,
            LPTSTR description,
            SC_HANDLE manager) const
        {
            TCHAR binaryPath[MAX_PATH];
            if (!GetModuleFileName(NULL, binaryPath, MAX_PATH))
            {
                std::cout << "GetModuleFileName error: " << win32Error(GetLastError()) << std::endl;
                return;
            }

            auto service = CreateService(
                manager,                   // SCM database 
                serviceName,               // name of service 
                displayName,               // service name to display 
                SERVICE_ALL_ACCESS,        // desired access 
                SERVICE_WIN32_OWN_PROCESS, // service type 
                SERVICE_AUTO_START,        // start type 
                SERVICE_ERROR_NORMAL,      // error control type 
                binaryPath,                // path to service's binary 
                NULL,                      // no load ordering group 
                NULL,                      // no tag identifier 
                NULL,                      // no dependencies 
                NULL,                      // LocalSystem account 
                NULL);                     // no password 

            if (service)
            {
                std::cout << "Service created." << std::endl;
                CloseServiceHandle(service);

                setDescription(serviceName, manager, description);
            }
            else std::cout << "CreateService error: " << win32Error(GetLastError()) << std::endl;
        }

        void tryDelete(LPCTSTR serviceName, SC_HANDLE manager) const
        {
            openService(serviceName,
                manager,
                DELETE,
                "DeleteService",
                "Service deleted.",
                [](auto service) { return DeleteService(service) != 0; });
        }

        void start(LPCTSTR serviceName, SC_HANDLE manager) const
        {
            openService(serviceName,
                manager,
                SERVICE_START,
                "StartService",
                "Service running.",
                [](auto service) { return StartService(service, 0, NULL) != 0; });
        }

        void stop(LPCTSTR serviceName, SC_HANDLE manager) const
        {
            openService(serviceName,
                manager,
                SERVICE_STOP,
                "ControlService",
                "Service stopped.",
                [](auto service) {
                    SERVICE_STATUS status;
                    return ControlService(service, SERVICE_CONTROL_STOP, &status) != 0;
            });
        }

        void setDescription(LPCTSTR serviceName, SC_HANDLE manager, LPTSTR description) const
        {
            openService(serviceName,
                manager,
                SERVICE_CHANGE_CONFIG,
                "ChangeServiceConfig2",
                "Service description added.",
                [description](auto service) {
                    SERVICE_DESCRIPTION sd{ description };
                    return ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &sd) != 0;
            });
        }

        void openService(LPCTSTR serviceName,
            SC_HANDLE manager,
            DWORD desiredAccess,
            const std::string &handlerName,
            const std::string &successMessage,
            const std::function<bool(SC_HANDLE)> &handler) const
        {
            auto service = OpenService(manager, // SCM database 
                serviceName,
                desiredAccess);

            if (service)
            {
                if (!handler(service))
                    std::cout << handlerName << " error: " << win32Error(GetLastError()) << std::endl;
                else std::cout << successMessage << std::endl;

                CloseServiceHandle(service);
            }
            else std::cout << "Unable to run " << handlerName << " due to OpenService error: " << win32Error(GetLastError()) << std::endl;
        }

        void help(LPCTSTR serviceName) const
        {
            auto service = t_convert(serviceName);
            std::cout
                << "DESCRIPTION:" << std::endl
                << "        Command line program options for setting up this service with the Service Control Manager." << std::endl
                << "USAGE:" << std::endl
                << "        " << service << " <Option>" << std::endl << std::endl
                << "        Options:" << std::endl
                << "          install---------Creates this service as an Automatic startup service and sets it Running." << std::endl
                << "          uninstall-------Stops this service and deletes it." << std::endl
                << "          create----------Creates this service as an Automatic startup service." << std::endl
                << "          delete----------Deletes this service ie. completely removed from Service Control." << std::endl
                << "          start-----------Sets this service Running if it exists." << std::endl
                << "          stop------------Stops this service if its Running." << std::endl
                << "EXAMPLE:" << std::endl
                << "        " << service << " install" << std::endl << std::endl;
        }
    };
}//namespace ServiceUtils

// Service.h
//
// Author: Mikko Saarinki
// Source: https://github.com/Kingston1/windows-service
// License: MIT 03/31/2021

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

// ServiceMain.h
//
// Author: Mikko Saarinki
// Source: https://github.com/Kingston1/windows-service
// License: MIT 03/31/2021

/**
* Windows service on stack.
*
* All you need to do in ordinary process main for it to become a full blown service:
* ServiceMain<ExampleApp>(SERVICENAME, DISPLAYNAME, DESCRIPTION, argv);
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


#endif /* LIB_TEMPLATE_CMAKE_HEADER_ONLY_H */
