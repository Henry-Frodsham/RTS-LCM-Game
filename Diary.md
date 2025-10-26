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