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
//      File System
//

#include "kexlib.h"
#include "kpf.h"
#include "unzip.h"

#ifndef KEX_WIN32
#include <unistd.h>
#endif

static kexPakFile pakFileLocal;
kexPakFile *kex::cPakFiles = &pakFileLocal;

#define FILE_MAX_HASH_SIZE  32768

//
// kexPakFile::kexPakFile
//

kexPakFile::kexPakFile()
{
}

//
// kexPakFile::~kexPakFile
//

kexPakFile::~kexPakFile()
{
}

//
// kexPakFile::Shutdown
//

void kexPakFile::Shutdown(void)
{
    kpf_t *pack;

    kex::cSystem->Printf("Shutting down file system\n");

    for(pack = root; pack; pack = pack->next)
    {
        unzClose(pack->filehandle);
    }

    Mem_Purge(hb_file);
}

//
// kexPakFile::HashFileName
//

long kexPakFile::HashFileName(const char *fname, int hashSize) const
{
    unsigned int hash   = 0;
    char *str           = (char*)fname;
    char c;

    while((c = *str++))
    {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash & (hashSize-1);
}

//
// kexPakFile::LoadZipFile
//

void kexPakFile::LoadZipFile(const char *file, const bool bUseBasePath)
{
    unzFile uf;
    unz_global_info gi;
    unz_file_info fi;
    char filename[MAX_FILEPATH];
    kpf_t *pack;
    unsigned int i;
    unsigned int entries;
    long hash;
    const char *filepath;
    kexStr fPath;

    if(bUseBasePath)
    {
        filepath = kex::cvarBasePath.GetValue();

        fPath = filepath;
        fPath = fPath + "/" + file;
    }
    else
    {
        fPath = file;
    }

    fPath.NormalizeSlashes();

    kex::cSystem->Printf("kexPakFile::LoadZipFile: Loading %s\n", fPath.c_str());

    // open zip file
    if(!(uf = unzOpen(fPath.c_str())))
    {
        kex::cSystem->Error("kexPakFile::LoadZipFile: Unable to find %s", fPath.c_str());
    }

    // get info on zip file
    if(unzGetGlobalInfo(uf, &gi) != UNZ_OK)
    {
        return;
    }

    // allocate new pack file
    pack = (kpf_t*)Mem_Calloc(sizeof(kpf_t), hb_file);
    pack->filehandle = (unzFile*)uf;
    strcpy(pack->filename, fPath.c_str());
    pack->numfiles = gi.number_entry;
    pack->next = root;
    root = pack;

    // point to start of zip files
    unzGoToFirstFile(pack->filehandle);

    // setup hash entires
    for(entries = 1; entries < FILE_MAX_HASH_SIZE; entries <<= 1)
    {
        if(entries > pack->numfiles)
        {
            break;
        }
    }

    // allocate file/hash list
    pack->hashentries = entries;
    pack->files = (file_t*)Mem_Calloc(sizeof(file_t) * pack->numfiles, hb_file);
    pack->hashes = (file_t***)Mem_Calloc(sizeof(file_t**) * pack->hashentries, hb_file);
    pack->hashcount = (unsigned int*)Mem_Calloc(sizeof(int) * pack->hashentries, hb_file);

    // fill in file lookup lists
    for(i = 0; i < pack->numfiles; i++)
    {
        file_t *fp;

        if(unzGetCurrentFileInfo(uf, &fi, filename, sizeof(filename),
                                 NULL, 0, NULL, 0) != UNZ_OK)
        {
            break;
        }

        pack->files[i].cache = NULL;
        fp = &pack->files[i];

        unzGetCurrentFileInfoPosition(pack->filehandle, &fp->position);
        strcpy(fp->name, filename);
        fp->info = fi;

        // get hash number
        hash = HashFileName(filename, pack->hashentries);

        // resize hash table if needed
        pack->hashes[hash] = (file_t**)Mem_Realloc(
                                 pack->hashes[hash],
                                 sizeof(file_t*) * (pack->hashcount[hash]+1), hb_file);

        pack->hashes[hash][pack->hashcount[hash]++] = fp;

        unzGoToNextFile(pack->filehandle);
    }
}

//
// kexPakFile::OpenFile
//

int kexPakFile::OpenFile(const char *filename, byte **data, kexHeapBlock &hb) const
{
    long hash;

    if(kex::cvarDeveloper.GetBool())
    {
        int len = OpenExternalFile(filename, data);

        if(len != -1)
        {
            return len;
        }
    }

    for(kpf_t *pack = root; pack; pack = pack->next)
    {
        hash = HashFileName(filename, pack->hashentries);

        if(pack->hashes[hash])
        {
            unsigned int i;

            for(i = 0; i < pack->hashcount[hash]; i++)
            {
                file_t *file = pack->hashes[hash][i];

                if(!strcmp(file->name, filename))
                {
                    if(!file->cache)
                    {
                        file->cache = Mem_Malloc(file->info.uncompressed_size+1, hb);
                        // automatically set cache to NULL when freed so we can
                        // recache it later
                        Mem_CacheRef(&file->cache);

                        unzSetCurrentFileInfoPosition(pack->filehandle, file->position);
                        unzOpenCurrentFile(pack->filehandle);
                        unzReadCurrentFile(pack->filehandle, file->cache,
                                           file->info.uncompressed_size);
                        unzCloseCurrentFile(pack->filehandle);
                    }

                    *data = (byte*)file->cache;
                    return file->info.uncompressed_size;
                }
            }
        }
    }

    *data = NULL;
    return 0;
}

//
// kexPakFile::LoadUserFiles
//

void kexPakFile::LoadUserFiles(void)
{
    kexStrList list;
    int p;

    if((p = kex::cSystem->CheckParam("-file")))
    {
        while(++p != kex::cSystem->Argc() && kex::cSystem->Argv()[p][0] != '-')
        {
            LoadZipFile(kex::cSystem->Argv()[p]);
        }
    }
    else
    {
        // handle drag and drop files
        for(int i = 1; i < kex::cSystem->Argc(); ++i)
        {
            if( kexStr::IndexOf(kex::cSystem->Argv()[i], ".kpf") != -1 ||
                kexStr::IndexOf(kex::cSystem->Argv()[i], ".KPF") != -1)
            {
                kexStr fileName = kex::cSystem->Argv()[i];

                LoadZipFile(kex::cSystem->Argv()[i], false);
            }
        }
    }

    // auto-load files from /mods directory
    GetMatchingExternalFiles(list, "mods/");

    for(unsigned int i = 0; i < list.Length(); ++i)
    {
        if( list[i].IndexOf(".kpf\0") == -1 &&
            list[i].IndexOf(".KPF\0") == -1)
        {
            continue;
        }
        
        LoadZipFile(list[i].c_str());
    }
}

//
// kexPakFile::GetMatchingExternalFiles
//

void kexPakFile::GetMatchingExternalFiles(kexStrList &list, const char *search)
{
    DIR *dir;
    struct dirent *ent;
    int idx;
    kexStr path = kexStr(kex::cvarBasePath.GetValue()) + "/" + search;
    path.NormalizeSlashes();

    if((dir = opendir(path.c_str())) != NULL)
    {
        while((ent = readdir(dir)) != NULL)
        {
            idx = kexStr::IndexOf(ent->d_name, ".");

            if(ent->d_name[0] == '.')
            {
                continue;
            }

            if(idx == -1 || idx == 0)
            {
                GetMatchingExternalFiles(list, (kexStr(search) + ent->d_name + "/").c_str());
                continue;
            }

            list.Push(kexStr(search) + ent->d_name);
        }
        closedir(dir);
    }
}

//
// kexPakFile::GetMatchingFiles
//

void kexPakFile::GetMatchingFiles(kexStrList &list, const char *search)
{
    // for development mode, scan local directories that's not part of the pak file
    if(kex::cvarDeveloper.GetBool())
    {
        GetMatchingExternalFiles(list, search);
    }

    // scan files inside pak files
    for(kpf_t *pack = root; pack; pack = pack->next)
    {
        for(unsigned int i = 0; i < pack->numfiles; i++)
        {
            file_t *file = &pack->files[i];

            if(strstr(file->name, search))
            {
                if(file->name[0] == '.')
                {
                    continue;
                }

                if(kexStr::IndexOf(file->name, ".") == -1)
                {
                    GetMatchingFiles(list, (kexStr(search) + file->name + "/").c_str());
                    continue;
                }

                // check to make sure we don't open the same file twice if we
                // already found it in the local directory
                bool bFoundDupFile = false;

                for(unsigned int j = 0; j < list.Length(); ++j)
                {
                    if(list[j] == file->name)
                    {
                        bFoundDupFile = true;
                        break;
                    }
                }

                if(bFoundDupFile)
                {
                    kex::cSystem->DPrintf("Duplicate file found: %s\n", file->name);
                    continue;
                }

                list.Push(file->name);
            }
        }
    }
}

//
// kexPakFile::OpenExternalFile
//

int kexPakFile::OpenExternalFile(const char *name, byte **buffer) const
{
    kexStr fPath;
    kexBinFile fp;

    fPath = kexStr::Format("%s\\%s", kex::cvarBasePath.GetValue(), name);
    fPath.NormalizeSlashes();

    // must call OpenStream here and not Open, otherwise it'll recurse infinitely
    if(fp.OpenStream(fPath.c_str()))
    {
        size_t length = fp.Length();

        *buffer = (byte*)Mem_Calloc(length+1, hb_file);

        if(fp.ReadStream(0, *buffer, length) == length)
        {
            fp.Close();
            return length;
        }

        Mem_Free(*buffer);
        *buffer = NULL;
        fp.Close();
    }

    return -1;
}

//
// kexPakFile::Init
//

void kexPakFile::Init(void)
{
    kex::cvarBasePath.Set(kex::cSystem->GetBaseDirectory());
    kex::cSystem->Printf("File System Initialized\n");
}
