# windows-service
Promote any application to a first class Windows service. Inject your application into the Service class, give it a name and description (service.cpp). ExampleApp.h has the simple interface required for injection. Use it as a template for your own service.


## building the example application with Visual Studio 2017 (2015 also supported)
```
mkdir x64 && cd x64
cmake -G "Visual Studio 15 2017 Win64" ..
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\Common7\Tools\VsDevCmd.bat"
msbuild /p:Configuration=Release ExampleService.sln
```

Run the app from command line and it will tell you its capabilities.
