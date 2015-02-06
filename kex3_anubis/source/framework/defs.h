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

#ifndef __DEFINITION_H__
#define __DEFINITION_H__

class kexDefManager
{
public:
    ~kexDefManager(void);

    void                            Init(const char *directory);
    kexDict                         *GetEntry(const char *name);

    kexHashList<kexDict>            defs;

private:
    void                            Parse(kexLexer *lexer);
};

#endif
