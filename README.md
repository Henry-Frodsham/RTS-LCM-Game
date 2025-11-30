# Final Year Project

This repository has been created to store your final year project.

You may edit it as you like, but please do not remove the default topics or the project members list. These need to stay as currently defined in order for your supervisor to be able to find your project.



deployment instructions:

* ensure you have the latest version of msredist and visual studio 2022 (17.14.15) with desktop c++ and python development components
* under "individual components" in vs installer check "vcpkg package manager" 
* to install vcpkg run "vcpkg integrate install" in the vcpkg dir (cmd)
* open product.sln (if a retarget prompt appears then refer to above steps)
* this project uses cpplint in pre build events, run "pip install cpplint" (important that this is done in the vs python command prompt and not a regular command prompt)
* you'll be presented with 3 different projects in visual studio, "product" is a static lib and "tests" + "application" is an exe
* before compiling, configure "multiple startup projects" and make sure that the static lib isn't run but the other 2 are
* clean then rebuild all using visual studio, vcpkg will download and install dependancies (usually takes ~2-~3 hours)
* please note that if the first run fails it means that vcpkg automatic linking failed. as a fallback, post build events will copy all required DLLs to the output dir meaning the second run wont fail.
* if you get a fatal error (code 5000+) on runtime ("failed to link plugin OpenGL") and a subsequent ("failed to set DirectX as a fallback") then you must install either of them then restart.
* *VERY IMPORTANT* if you are running my project from inside visual studio you must change the debugging directory (working directory in the debugging section) for each individual project (application, product, tests) to $(OutDir) (must be uniform between projects), prebuild events will copy the right ogre config to the outdir so please keep in mind if you decide to build my project to a different directory (outside of visual studio) then you need to recreate this behaviour.