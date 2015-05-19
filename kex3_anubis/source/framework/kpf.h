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

#ifndef __KPF_H__
#define __KPF_H__

#include "unzip.h"
#include "array.h"
#include "kstring.h"

class kexPakFile
{
public:
    kexPakFile();
    ~kexPakFile();

    void                Shutdown(void);
    void                LoadUserFiles(void);
    void                LoadZipFile(const char *file, const bool bUseBasePath = true);
    int                 OpenFile(const char *filename, byte **data, kexHeapBlock &hb) const;
    int                 OpenExternalFile(const char *name, byte **buffer) const;
    void                GetMatchingFiles(kexStrList &list, const char *search);
    void                GetMatchingExternalFiles(kexStrList &list, const char *search);
    void                Init(void);

private:
    long                HashFileName(const char *fname, int hashSize) const;

    typedef struct
    {
        char            name[MAX_FILEPATH];
        unsigned long   position;
        unz_file_info   info;
        void*           cache;
    } file_t;

    typedef struct kpf_s
    {
        unzFile         *filehandle;
        unsigned int    numfiles;
        char            filename[MAX_FILEPATH];
        file_t          *files;
        file_t          ***hashes;
        unsigned int    *hashcount;
        unsigned int    hashentries;
        struct kpf_s    *next;
    } kpf_t;

    kpf_t               *root;
    char                *base;
};

#endif
