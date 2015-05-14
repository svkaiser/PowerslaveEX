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

#ifndef __SCRIPT_GAME_OBJ_H__
#define __SCRIPT_GAME_OBJ_H__

class kexScriptObjGame
{
public:
    static void         Init(void);

    void                CallDelayedMapScript(const kexStr &str, kexActor *instigator, const float delay);
    void                CallDelayedMapScript(const int scriptID, kexActor *instigator, const float delay);
    void                HaltMapScript(const int scriptID);
    void                FireRemoteEventFromTag(const int tag);
    void                FireActorEventFromTag(const int tag);
    void                PlaySound(const kexStr &str);
    void                PlayMusic(const kexStr &str, const bool bLoop);
    void                StopMusic(void);
    void                ChangeMap(const kexStr &map);
    void                EndGame(const bool bGoodEnding);
    const bool          LevelIsMapped(void) const;
    void                SpawnLight(kexActor *source, const float radius,
                                   const kexVec3 &color, const float fadeTime, const int passes);
    void                MoveScriptedSector(const int tag, const float height,
                                           const float speed, const bool bCeiling);
};

#endif
