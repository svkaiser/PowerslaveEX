This folder contains all notes and scratch work related to disassembling the Playstation version of Powerslave. This includes the following:

* Code snippets of diassembled code that I attempted to convert from MIPS assembly to C++
* IDA Pro database file used to disassemble the PSX executable
* Code used to extract data from the original game (scratch.cpp)

Scratch.cpp is horribly unorganized, but has some comments about the .ZED format as well as code that extracts sounds and movies from the original game. A lot of the extracted contents has been cherry picked and manually stitched together so it will require some work if you want to try building the game data yourself.

Textures are cached per level in a similar manner to how Quake stores its textures, so multiple levels may have the same texture stored. This will require a lot of time to organize all unique textures from all levels. This also applies to sprites, sounds and animation sprite data which will also need sorting out.

There's still a lot of guesswork in figuring out the ZED format so there's a few magic offset values that were used.
