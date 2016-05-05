# windows-service
Promote any application to a first class Windows service. Inject your application into the Service class, give it a name and description (service.cpp). See ExampleApp (in Service.h) for the simple interface required for injection and use it as a template for your own service.


## building the example application
```
cmake -G "Visual Studio 14 2015"
msbuild ExampleApp.sln
```

Run the app from command line and it will tell you its capabilities.