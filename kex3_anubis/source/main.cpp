//
// Copyright(C) 2014-2015 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//      Main application
//

#include "SDL.h"
#include "kexlib.h"

kexCvar kex::cvarDeveloper("developer", CVF_BOOL|CVF_CONFIG, "0", "Developer mode");

//
// main
//

#if defined(KEX_WIN32) && !defined(_DEBUG)
extern int __cdecl I_W32ExceptionHandler(PEXCEPTION_POINTERS ep);
int main(int argc, char *argv[])
{
    __try
    {
        kex::cSystem->Main(argc, argv);
    }
    __except(I_W32ExceptionHandler(GetExceptionInformation()))
    {
        kex::cSystem->Error("Exception caught in main: see CRASHLOG.TXT for info\n");
    }

    return 0;
}
#else
int main(int argc, char *argv[])
{
    kex::cSystem->Main(argc, argv);
    return 0;
}
#endif
