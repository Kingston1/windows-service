// ServiceUtils.h
//
// Author: Mikko Saarinki
// Copyright (c) 2016 Mikko Saarinki. All rights reserved.
//
#pragma once
#include <windows.h>
#include <tchar.h>

#include <iostream>
#include <string>
#include <functional>

namespace ServiceUtils
{
    //why microsoft doesn't provide human readable and easy
    //conversion from their silly macro world is beyond comprehension
    std::string t_convert(const TCHAR*);

    //convenience method for human readable GetLastError()
    std::string win32Error(DWORD error);

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
