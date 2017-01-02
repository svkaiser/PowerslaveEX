PowerslaveEX GPL Source Code
=======================================

This is the source code to Powerslave EX, an ****UNOFFICIAL**** remake of Powerslave featured on the Playstation back in 1997. This remake is based on the first iteration of my custom game engine, Kex Engine 3.0. While most of the game logic has been reverse-engineered, the collision and AI system is completely original. The level format and how sprites are handled was also changed to a custom format to suit my engine's needs, so using data directly from the PSX CD will not work.

I also would like to note that a lot of the core code in this verrsion of Kex is already out of date and is somewhat bugged. Though there has been a lot of improvements in other projects since this build, there's currently no plans to back-port those updates just yet. All levels, textures, audio and sprites have been stripped. However I am currently looking into a solution to generate the game data from the original PSX CD.

Again I would like to stress that this is a ****UNOFFICIAL**** hobby project and is not supported by its ip-owners nor Playmates Interactive. Do ****NOT**** contact them for support.

Compiling on Win32:
-------------------

A project file for Microsoft Visual Studio 2008 is provided in kex3_anubis/msvc/
The solution file is compatible with the Express releases.

Compiling on MacOS 10.x:
-------------------

There were plans to do a MacOSX port but cancelled when the project got halted. The XCODE project files can be found in kex3_anubis/xcode/

Dependencies
-------------------

Powerslave EX uses the following third-party libraries (included in this repo)
* SDL2
* Angelscript
* FFMpeg
* OpenAL
* Vorbis
* LibPNG
* ZLib

Contributions
-------------------

I am willing to accept contributions but do please try to follow the existing format and syntax in the source code.

Technical Support
-------------------

I will happily answer any questions you may have regarding to level formats or even about the source code. I can be contacted via svkaiser--at--gmail--dot--com
