// service.cpp
//
// Author: Mikko Saarinki
// Copyright (c) 2016 Mikko Saarinki. All rights reserved.
//
#include "ServiceMain.h"

#include "ExampleApp.h"

#define SERVICENAME TEXT("ExampleApp")
#define DISPLAYNAME TEXT("Windows Service Example") //displayed in Windows Services
#define DESCRIPTION TEXT("Does nothing but sleeps and waits for service stop signal.")

void __cdecl _tmain(int argc, TCHAR *argv[])
{
    ServiceMain<ExampleApp>(SERVICENAME, DISPLAYNAME, DESCRIPTION, argv);
}
