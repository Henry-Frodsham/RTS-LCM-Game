* Open product.sln (if a retarget prompt appears then refer to deployment perquisites) \newline
* This project uses cpplint in pre build events, run "pip install cpplint" (important that this is done in the vs python command prompt and not a regular command prompt) \newline
* You'll be presented with 3 different projects in visual studio, "product" is a static lib and "tests" + "application" is an exe \newline
* Before compiling, configure "multiple startup projects" and make sure that the static lib isn't run but the other 2 are \newline
* Clean then rebuild all using visual studio, vcpkg will download and install dependencies (usually takes ~2-~3 hours) \newline
* Please note that if the first run fails it means that vcpkg automatic linking failed. as a fallback, post build events will copy all required DLLs to the output dir meaning the second run wont fail. \newline
* Please note that this project requires OpenGL to be installed, the glsl shader will not function when this project is used with DirectX
* *VERY IMPORTANT* if you are running my project from inside visual studio you must change the debugging directory (working directory in the debugging section) for each individual project (application, product, tests)  to \$(OutDir) (must be uniform between projects), prebuild events will copy the right ogre config to the outdir so please keep in mind if you decide to build my project to a different directory (outside of visual studio) then you need to recreate this behaviour.\newline

* *VERY IMPORTANT* you must ensure that the pre and post build events have been preserved after cloning this repository. Without these, the dependancy DLLs and resource folders wont be copied to the output directory.


directory structure:
documents/ - project documents including reports
product/ - the code directory for the product project (product.sln)
application/ - the executable code directory (the code that runs not the directory storing built exes)
tests/ - the test project code directory
resources/ - contains mesh and material files loaded into the scene
config/ - the config files for settings

