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
//      Language translation string lookup
//

#include "kexlib.h"
#include "game.h"
#include "localization.h"

kexCvar kexTranslation::cvarLanguage("g_language", CVF_INT|CVF_CONFIG, "0", LNG_ENGLISH, NUMLANGUAGES-1, "Language Localization");

//
// kexTranslation::kexTranslation
//

kexTranslation::kexTranslation(void)
{
    this->language = LNG_ENGLISH;
}

//
// kexTranslation::~kexTranslation
//

kexTranslation::~kexTranslation(void)
{
}

//
// kexTranslation::GetString
//

const char *kexTranslation::GetString(const int index)
{
    language = static_cast<languages_t>(cvarLanguage.GetInt());

    if(index < 0 || index > (int)strings[language].Length())
    {
        return NULL;
    }

    return strings[language][index].c_str();
}

//
// kexTranslation::GetFormattedString
//

void kexTranslation::GetFormattedString(const int index, kexStrList &strList, const int value)
{
    kexStr str;

    if(index < 0 || index > (int)strings[language].Length())
    {
        return;
    }

    str = GetString(index);
    str.Split(strList, '\n');

    if(strList.Length() == 0)
    {
        strList.Push(str);
        return;
    }

    for(unsigned int i = 0; i < strList.Length(); ++i)
    {
        if(strList[i].IndexOf("%d") != -1)
        {
            str = kexStr::Format(strList[i], value);
            strList[i] = str;
        }
    }
}

//
// kexTranslation::TranslateString
//

const char *kexTranslation::TranslateString(const char *str)
{
    if(kexStr::IndexOf(str, "$str_") == 0)
    {
        int index = atoi(str + 5);

        return GetString(index);
    }

    return str;
}

//
// kexTranslation::Init
//

void kexTranslation::Init(void)
{
    kexBinFile tranfile;
    unsigned int startOffs;
    int offset;

    if(!tranfile.Open("localization/localization.dat"))
    {
        kex::cSystem->Error("kexTranslation::Init - Could not load localization file");
        return;
    }

    if(tranfile.Read32() != NUMLANGUAGES)
    {
        kex::cSystem->Error("kexTranslation::Init - Language count mismatched");
        return;
    }

    startOffs = tranfile.Read32();
    offset = startOffs;

    while(tranfile.BufferOffset() < startOffs)
    {
        for(int i = 0; i < NUMLANGUAGES; ++i)
        {
            strings[i].Push(kexStr((char*)&tranfile.Buffer()[offset]));
            offset = tranfile.Read32();
        }
    }
}
