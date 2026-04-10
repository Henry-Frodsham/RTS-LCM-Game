11/10/25 - researched convenient way to implement TDD
11/10/25 - configured and succesfully setup project
12/10/25 - configured cpplint to check c++ google standard
12/10/25 - designed general structure of ECS Registry and Event bus
12/10/25 - created initial test templates
12/10/25 - created EventBus and EventQueue tests
13/10/25 - researched ogre render system patterns
13/10/25 - initial ViewPortController and RenderSystem classes and tests
14/10/25 - investigated the best way of harvesting input simultaneously from multiple controllers (arrived at SDL2, integrated into ogre by default)
15/10/25 - created initial event templates and InputListener class
15/10/25 - shifted focus to ECS storage, researched how entt registry should be managed and who should own it. decided on a WorldManager singleton
16/10/25 to 19/10/25 - modeling runtime scripts and researching other ogre3d projects
20/10/25 - race condition tests, initial implementation of runtime scripts using app states
21/10/25 - write base renderSystem class, designed error and logging system 
22/10/25 - wrote EventBus and EventQueue class as prequisite to error and logging system 
22/10/25 - realised EventQueue class erases type information, redesigned and placed queue items in a wrapper class
24/10/25 - wrote passing tests for eventQueue
25/10/25 - when doing ogre runtime tests i realised that the vcpkg version of ogre is incorrect, porting fully to a manual build
26/10/25 - identified that the issue was due to string corruption of the package name, ported back to vcpkg and fixed by linking debug libraries when neccessary
27/10/25 - wrote passing viewport tests (apart from an unidentified issue regarding viewport z-orders)
28/10/25 - passing tests across the board (bar 2 viewport tests), created a responsive ogre render window
29/10/25 - integrated and initialised SDL2 and Dear Imgui
30/10/25 - partial implementation of error reporting (without handling yet)
30/10/25 - looked at sample programs in the ogre and sdl doc, realised input listener cant be on a per instance basis, redesigned such that a singular input handler dispatches to each instance
31
4/11/25 - which data structure and key is appropriate for having an input device bound to an sdl id? (same issue for input device and queue)
4/11/25 - read https://www.geeksforgeeks.org/cpp/map-vs-unordered_map-c/
4/11/25 - read https://www.geeksforgeeks.org/cpp/set-vs-unordered_set-c-stl/
4/11/25 - read https://www.geeksforgeeks.org/cpp/advantages-of-vector-over-array-in-c/
4/11/25 - settled on unordered set instead of vectors, mainly for O(1) lookups but also since its a set and it only needs to store unique data
5/11/25 - how can i replicate case sensitivy for sdl when sdl2 doesnt generate capital letter events? 
5/11/25 - read https://discourse.libsdl.org/t/case-sensitive-key-down/10807
5/11/25 - realized sdl2 has global "mod" events, used them to check for shift and capslock state to replicate case sensitivity
6/11/25 - test port to secondary pc
6/11/25 - realized that plugins.cfg used by ogre reads exclusively from the build output dir which isnt replicated on gitlab
6/11/25 - added a prebuild event to copy from product/ to $(Outdir)
7/11/25 - how can i add specific error information for my error handling?
7/11/25 - used previously familiar format library
7/11/25 - encountered issue where format would read string values at compile time as const
7/11/25 - realized limitation of format library with a static lib and researched alternatives
7/11/25 - read https://stackoverflow.com/questions/76329898/fmt-compile-time-format-string-check-without-generating-asm-code-for-the-check
7/11/25 - implemented fmt library instead as dependancy then used fmt in CreateError calls to force runtime string building
8/11/25 - researched Imgui implementation and consulted ogre doc with imgui example at https://github.com/OGRECave/ogre/tree/master/Samples/Simple/include
8/11/25 - realized this example didnt apply to me due to their use of ApplicationBase (used to make cookie cutter ogre3d apps not relevant to me )
8/11/25 - attempted custom implementation of imgui in my project, repeated failure
8/11/25 - consulted countless forum posts on the ogre forum, many outdated and irrelevant to the issue
8/11/25 - realized imguis IO string fields "BackEndRenderer" was corrupt/inaccesible
9/11/25 - realized that im building imgui and ogre as dynamic library with no way of accessing a shared IO
9/11/25 - edited project properties to build as static lib (for unified memory)
9/11/25 - major edits to project setup for compatibility with static lib versions of ogre and sdl2 etc
10/11/25 - using ogre in static lib mode proved to be a major pain, reverting back to last stable build
10/11/25 - after revert, used native ogre overlay support to add OverlayController (class owned by RenderSystem and is responsible soley for overlays)
11/11/25 - stopped checking controller GUID, realised it was manufacturer id and anyone using generic controllers (like me) would have their controllers lumped onto the same id
11/11/25 - added a debug overlay that shows activity of all input devices
11/11/25 - when refining before final merge, i noticed that SDL2 actually has a limitation with generic controllers, that being it has literally no idea which is which, this isnt a problem if they dont disconnect but finding the right controller to connect back to is now proving extremely difficult
11/11/25 - my first idea for solving this problem is to just reconnect controllers to the slot that doesnt have a valid controller connected, this would work perfectly fine however a foreseeable issue is if two controllers disconnect at the same time then they may end up in the wrong slot. this is the most practical solution for now however once the project is at a later stage my definitive solution is to have a prompt pop up for each disconnected user saying "press any key (player)" so the user can manually put it back in the right slot
11/11/25 - implemented fix to controller reconnection, as suspected theres an issue where disconnected 2 controllers can cause the slots to switch
12/11/25 - noticed that the input sensitivity of controllers depended on the frame rate the game was running on when testing on multiple clamped frame rates
12/11/25 - implemented a dynamic delta time value that can easily be retrieved from RenderSystem and scaled input sensitivity with it
13/11/25 - 22/12/25 - time dedicated to multiple assignments for other modules
23/11/25 - asked myself what should i be doing for users with different display types or render preferences
24/11/25 - utilized nlohmann json to make a generic ConfigManager usable for any config elsewhere in my codebase
25/11/25 - implemented the new config system for video settings, including render resolution, the default render resolution is 1920x1080 since its the most common resolution
27/11/25 - researched ECS implementations for use cases similar to mine
28/11/25 - researched the ogre docs to see what information i should be storing for my ECS entities
29/11/25 - initial ECS implementation with a test cube (different coloured faces) for the demo

12-20/12/25 - final refinements to interim report (extension used)

--- out of term development 
12/12/25 - research similar split overlay implementations (having overlays unique to a viewport)
14/12/25 - implemented extended functionality to rendersystem through an owned class (PreViewPortUpdateListener)
15/12/25 - implemented unique overlay functionality by hiding other overlays at the precise moment they are about to rendered, this is done using the PreViewPortUpdate event thats dispatched automatically by ogre

27/12/25 - limit the cursor position to the size of a unique overlay, this functionality will be extended in future by preventing the cursor from moving over other viewports

--- term 2 start
19/01/26 - revisited https://liacs.leidenuniv.nl/~plaata1/papers/abbadi-resources_entities_actions_a_generalized_design_pattern-118.pdf for a better understanding of resource entity action
20/01/26 - wrote failing tests for the transformation matrix class
20-25/01/26 - implementation of a rudimentary cursor implementation, noticed significant stability issues from this implementation
26/01/26 - following testing of the cursor implementation, i decided that sticking to the default OS cursor for mouse was a better option for the mouse but keeping the system in place for controllers
27/01/26 - refactored the dispatching order for events such that events are processed the same frame they are created, reducing the perceived delay from the cursor
28/01/26 - created the entity interaction class, a class thats responsibility is to process all entity interactions and route matrixable interactions accordingly.
29-31/01/26 - edge cases for matrix tests, stability improvements following detailed analysis from performance profiler.

02/02/26 - read https://stackoverflow.com/questions/26516683/reusing-thread-in-loop-c to learn about reusing threads
02/02/26 - added BS thread pool as a dependancy, implemented into the current thread pool pattern, improved fps and stability remarkability (500->2000fps) and cpu usage of the gameinstance->run method dropped from 12.8% of 1 core to 2.7%
03/02/26 - noticed another significant performance limiter, updated application->run to follow the updated threading model and increased fps from 2000fps to 6000fps (cpu time dropped from 24% to 4.3%)
9/02/26 - designed an initial game map implementation, attempting to replicate the world map as a single mesh\newline
10/02/26 - after experimenting with the initial game map, i realised a cleaner approach would be to seperate the land masses as distinct meshes. this realisation came after investigating the ogre doc, specifically the ray tracing functionality. since ogre ray tracing handles mesh collision checks automatically, having the land masses be seperate meshes was a clean way of identifying land the units could spawn on (land masses) and ones they couldnt (the sea)\newline
11/02/26 - redesigned the world map after taking this into account, with countries also pertruding from the sea for an easier time with ogre raytracing.\newline
12/02/26 - updated world manager to load the new globe, each country is added to the same scenenode to make moving the globe easier if need be. the implemented world map does have a couple rendering errors, that being that some countries are visible through the globes surface (when the camera is facing the other side of the globe)\newline

16/02/26 - investigated how input translator currently propogates input events\newline
17/02/26 - realized that input translator up until this point has mainly been focused on only communicating with the same instance thread it runs in (input events arent sent elsewhere but instead the instance thread reads the states manually)\newline
18/02/26 - designed a new struct for input events sent to the render thread to be identifiable back to the instance thread it was sent from (ActionContext), along with other context such as on a MousePressed event it includes the screen coordinates pressed and whether the button was pressed for the first time or is being held down (instead of a MousePressed event only indicating that that device was pressed and nothing else)\newline

27/02/26 - looked into how to fix the rendering errors present from the globe implementation. read https://forums.ogre3d.org/viewtopic.php?t=81112\newline
28/02/26 - since ive been using blender for my 3d meshes, the z order should be managed automatically so i deduced that the materials are the issue\newline
28/02/26 - read https://forums.ogre3d.org/viewtopic.php?t=57151 to identify the issue\newline
28/02/26 - added a depth \detokenize{pre_pass} level to the material scripts for the country material, this fixed the issue\newline

6/03/26 - setup for raytracing functionality, initialised an Ogre:rayscenequery on startup\newline
7/03/26 - using the new functionality in input translator, Player general control now starts a raytrace when the mouse is pressed. the result isnt used yet but has been verified to be correct for future implementation\newline

9/03/26 - revisited older overlay code to try and figure out a clean method to add pressable buttons\newline
10/03/26 - decided on using a callback system for button functionality, buttons would register an event to be queued back to an event queue pointer once the button was clicked allowing OverlayController to handle all of the mouse hovering and mouse pressed checks.\newline
11/03/26 - implemented the new functionality into overlaycontroller and added a new button class to make adding a functional button easier (GenericButton.cpp)\newline

14/03/26 - implemented a basic state machine in preparation for a main menu\newline
15/03/26 - using the new button class (Generic button) i created a main menu button that issues ChangeState event when pressed\newline
15/03/26 - added functionality to rendersystem to hide or show a mesh when needed, allowing me to hide the globe by default then show the globe when the play button is pressed\newline

20/03/26 - using previously unintegrated raytracing functionality, players can now select a unit or city\newline
20/03/26 - added functionality to highlight the selected unit or city by changing its colour to red\newline
21/03/26 - extended selection functionality by only allowing one city or unit to be selected, and pressing on a valid land mass will store the coordinates for future movement\newline

22/03/26 - added movement functionality to units, using the stored coordinates then the interaction wheel to move them\newline

25/03/26 - enemy units now appear as a different colour, this was done using a glsl fragment shader that does the ownership check on the gpu. units and cities now use a new material. \detokenize{unit_fp} that includes this check and once their initialised they have their ownership stored within it\newline

26/03/26 - added a new entity component, ProducesUnitsComponent. in the entitiy loop, this component adds to the available units passively\newline

30/03/26 - added attacking units functionality, final refinements to code\newline

6/04/26 - cpplint compliance 100 percent \newline