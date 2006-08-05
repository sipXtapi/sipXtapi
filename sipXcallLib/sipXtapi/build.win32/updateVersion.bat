call setRepoEnvVar
sed -i "s/SIPXTAPI_BUILDNUMBER.*\".*\"/SIPXTAPI_BUILDNUMBER \"%REPRO_VERSION%\"/g" ..\..\include\tapi\sipXtapi.h 
sed -i "s/SIPXTAPI_BUILD_WORD [0-9,,]*/SIPXTAPI_BUILD_WORD 2,9,1,%REPRO_VERSION%/g" ..\..\include\tapi\sipXtapi.h
sed -i "s/SIPXTAPI_FULL_VERSION.*\".*\"/SIPXTAPI_FULL_VERSION \"2.9.1.%REPRO_VERSION%\"/g" ..\..\include\tapi\sipXtapi.h
sed -i "s/SIPXTAPI_BUILDDATE.*\".*\"/SIPXTAPI_BUILDDATE \"%date:~10,4%-%date:~4,2%-%date:~7,2%\"/g" ..\..\include\tapi\sipXtapi.h
