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
//      Definition system
//

#include "kexlib.h"

//-----------------------------------------------------------------------------
//
// kexDefManager
//
//-----------------------------------------------------------------------------

//
// kexDefManager::~kexDefManager
//

kexDefManager::~kexDefManager(void)
{
}

//
// kexDefManager::ParseBlock
//

void kexDefManager::ParseBlock(kexDict *defEntry, kexLexer *lexer)
{
    kexStr key;
    kexStr val;

    lexer->ExpectNextToken(TK_LBRACK);
    while(1)
    {
        lexer->Find();
        if(lexer->TokenType() == TK_RBRACK || lexer->TokenType() == TK_EOF)
        {
            break;
        }

        key = lexer->Token();

        lexer->Find();
        if(lexer->TokenType() == TK_RBRACK || lexer->TokenType() == TK_EOF)
        {
            break;
        }

        val = lexer->Token();

        defEntry->Add(key.c_str(), val.c_str());
    }
}

//
// kexDefManager::Parse
//

void kexDefManager::Parse(kexLexer *lexer)
{
    kexDict *defEntry;

    while(lexer->CheckState())
    {
        lexer->Find();

        switch(lexer->TokenType())
        {
        case TK_EOF:
            return;

        case TK_IDENIFIER:
            defEntry = defs.Add(lexer->Token());
            ParseBlock(defEntry, lexer);
            break;

        case TK_STRING:
            defEntry = defs.Add(lexer->Token());
            ParseBlock(defEntry, lexer);
            break;

        default:
            break;
        }
    }
}

//
// kexDefManager::LoadFilesInDirectory
//

void kexDefManager::LoadFilesInDirectory(const char *directory)
{
    kexStrList list;

    kex::cPakFiles->GetMatchingFiles(list, directory);

    for(unsigned int i = 0; i < list.Length(); ++i)
    {
        // must be a valid text file
        if(list[i].IndexOf(".txt\0") == -1)
        {
            continue;
        }

        LoadFile(list[i].c_str());
    }
}

//
// kexDefManager::LoadFile
//

void kexDefManager::LoadFile(const char *defFile)
{
    kexLexer *lexer;
    
    if(!(lexer = kex::cParser->Open(defFile)))
    {
        return;
    }
    
    Parse(lexer);
    
    // we're done with the file
    kex::cParser->Close();
}

//
// kexDefManager::GetEntry
//

kexDict *kexDefManager::GetEntry(const char *name)
{
    return defs.Find(name);
}

//-----------------------------------------------------------------------------
//
// kexIndexDefManager
//
//-----------------------------------------------------------------------------

//
// kexIndexDefManager::~kexIndexDefManager
//

kexIndexDefManager::~kexIndexDefManager(void)
{
}

//
// kexIndexDefManager::Parse
//

void kexIndexDefManager::Parse(kexLexer *lexer)
{
    kexDict *defEntry;
    kexHashList<kexDict>::hashKey_t *hashKey;
    kexStr key;
    kexStr val;
    kexStr defName;
    int defIndex;
    
    while(lexer->CheckState())
    {
        lexer->Find();
        
        switch(lexer->TokenType())
        {
            case TK_EOF:
                return;
            case TK_IDENIFIER:
                defName = lexer->Token();
                defIndex = lexer->GetNumber();

                // we need to add two entries, one with a hash to the index
                // and another with a hash to the name. we also need to store
                // a reference to that index for the hashed name entry
                defEntry = defs.Add(defName, defIndex);
                hashKey = defs.AddAndReturnHashKey(defName);
                hashKey->refIndex = defIndex;
                
                lexer->ExpectNextToken(TK_LBRACK);
                while(1)
                {
                    lexer->Find();
                    if(lexer->TokenType() == TK_RBRACK || lexer->TokenType() == TK_EOF)
                    {
                        break;
                    }
                    
                    key = lexer->Token();
                    
                    lexer->Find();
                    if(lexer->TokenType() == TK_RBRACK || lexer->TokenType() == TK_EOF)
                    {
                        break;
                    }
                    
                    val = lexer->Token();
                    
                    defEntry->Add(key.c_str(), val.c_str());
                    hashKey->data.Add(key.c_str(), val.c_str());
                }
                break;
            default:
                break;
        }
    }
}

//
// kexIndexDefManager::GetEntry
//

kexDict *kexIndexDefManager::GetEntry(const int index)
{
    return defs.Find(index);
}
