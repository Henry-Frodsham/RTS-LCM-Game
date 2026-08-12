PROJECT GENERAL DESCRIPTION

A real time strategy game supporting local multiplayer (via splitscreen). 
The game map is a geosadic sphere with varying biomes and terrain, procedurally generated using FastNoise.
In its current prototype state the game has cities and generic units, multiple local player support, units can be moved (continously using A* pathing algorithm), units can attack, and some prototyped ui (healthbars, unit direction, interaction wheel, play button etc).

TECHNICAL OVERVIEW

This project aims to maintain low latency throughout operation, targeting scenarios with high player and entity counts. ECS and REA are used to achieve this end, significantly reducing the processing cost of one entity (ECS) and multi entity interactions (REA).

TECHNOLOGIES (all managed and version stamped via vcpkg, see vcpkg.json)

- OGRE3D (non ogre-next), lightweight and supports multiple viewport

- SDL2, industry standard input library

- Entt, Entity component system using sparse set

- Eigen (required for resource matrixes in documented resource entity action pattern), industry standard for matrix calculations

- doctest (required for unit tests and test driven development methodology)

- fmt, convenient string formatting library

- nlhomann json, json parsing library

- bshoshanny thread pool, used for thread pooling pattern

DIRECTORY STRUCTURE:
documents/ - project documents including reports

product/ - the code directory for the product project (product.sln)

application/ - the executable code directory (the code that runs not the directory storing built exes)

tests/ - the visual studio project used exclusively for unit tests via doctest

resources/font - contains true type fonts for rendering text

resources/simple/mat - contains simplistic materials denoting solid flat colours
resources/simple/mesh - contains basic meshes, mainly for testing purposes
resources/globe/mat - contains materials used on the globes surface, along with any glsl shaders
resources/globe/mesh - contains complex meshes used on the globes surface

config/default - the default config files, used as a fallback if a specific config file doesnt exist
config/custom - player specific config files, denoted as InputSettings{PlayerID} etc

Vendor/ - single header third party dependancies, along with their license


DEPLOYMENT INSTRUCTIONS (for more guidance, see the youtube video link in documents/Deployment video link.txt):

* Open product.sln (if a retarget prompt appears then refer to deployment perquisites) \newline
* This project uses cpplint in pre build events, run "pip install cpplint" (important that this is done in the vs python command prompt and not a regular command prompt) \newline
* You'll be presented with 3 different projects in visual studio, "product" is a static lib and "tests" + "application" is an exe \newline
* Before compiling, configure "multiple startup projects" and make sure that the static lib isn't run but the other 2 are \newline
* Clean then rebuild all using visual studio, vcpkg will download and install dependencies (usually takes ~2-~3 hours) \newline
* Please note that if the first run fails it means that vcpkg automatic linking failed. as a fallback, post build events will copy all required DLLs to the output dir meaning the second run wont fail. \newline
* Please note that this project requires OpenGL to be installed, the glsl shader will not function when this project is used with DirectX
* *VERY IMPORTANT* if you are running my project from inside visual studio you must change the debugging directory (working directory in the debugging section) for each individual project (application, product, tests)  to \$(OutDir) (must be uniform between projects), prebuild events will copy the right ogre config to the outdir so please keep in mind if you decide to build my project to a different directory (outside of visual studio) then you need to recreate this behaviour.\newline

* *VERY IMPORTANT* you must ensure that the pre and post build events have been preserved after cloning this repository. Without these, the dependancy DLLs and resource folders wont be copied to the output directory.
