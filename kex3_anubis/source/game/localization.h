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

#ifndef __LOCALIZATION_H__
#define __LOCALIZATION_H__

typedef enum
{
    LNG_ENGLISH     = 0,
    LNG_SPANISH,
    LNG_FRENCH,
    LNG_DUTCH,

    NUMLANGUAGES
} languages_t;

class kexTranslation
{
public:
    kexTranslation(void);
    ~kexTranslation(void);

    void                Init(void);
    const char          *GetString(const int index);
    void                GetFormattedString(const int index, kexStrList &strList, const int value = 0);
    const char          *TranslateString(const char *str);

    void                SetLanguage(const languages_t lang) { language = lang; }

    static kexCvar      cvarLanguage;

private:
    kexStrList          strings[NUMLANGUAGES];
    languages_t         language;
};

#endif
