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
//      Action Definitions
//

#include "kexlib.h"
#include "game.h"

static kexActionDefManager actionDefManagerLocal;
kexActionDefManager *kexGame::cActionDefManager = &actionDefManagerLocal;

//-----------------------------------------------------------------------------
//
// kexActionDef
//
//-----------------------------------------------------------------------------

DECLARE_ABSTRACT_KEX_CLASS(kexActionDef, kexObject)

//
// kexActionDef::kexActionDef
//

kexActionDef::kexActionDef(void)
{
}

//
// kexActionDef::~kexActionDef
//

kexActionDef::~kexActionDef(void)
{
    if(args)
    {
        delete[] args;
    }
}

//
// kexActionDef::Parse
//

void kexActionDef::Parse(kexLexer *lexer)
{
    int arg = 0;

    lexer->ExpectNextToken(TK_LPAREN);

    while(lexer->TokenType() != TK_RPAREN)
    {
        switch(argTypes[arg])
        {
        case AAT_INTEGER:
            this->args[arg].i = lexer->GetNumber();
            break;

        case AAT_FLOAT:
            this->args[arg].f = (float)lexer->GetFloat();
            break;

        case AAT_STRING:
            lexer->GetString();
            this->args[arg].s = Mem_Strdup(lexer->StringToken(), hb_static);
            break;
        }

        if(++arg >= this->numArgs)
        {
            lexer->ExpectNextToken(TK_RPAREN);
            break;
        }

        lexer->ExpectNextToken(TK_COMMA);
    }
}

//-----------------------------------------------------------------------------
//
// kexActionHitScan
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexActionHitScan, kexActionDef);
public:
    kexActionHitScan(void);
    ~kexActionHitScan(void);

    virtual void            Execute(kexActor *actor);

END_KEX_CLASS();

DECLARE_KEX_CLASS(kexActionHitScan, kexActionDef)

//
// kexActionHitScan::kexActionHitScan
//

kexActionHitScan::kexActionHitScan(void)
{
}

//
// kexActionHitScan::~kexActionHitScan
//

kexActionHitScan::~kexActionHitScan(void)
{
}

//
// kexActionHitScan::Execute
//

void kexActionHitScan::Execute(kexActor *actor)
{
    kexGameLocal *game  = kexGame::cLocal;
    kexCModel *cm       = kexGame::cLocal->CModel();
    float dist          = this->args[0].f;
    float hSpan         = this->args[1].f;
    float vSpan         = this->args[2].f;
    float height        = this->args[5].f;
    char *name          = this->args[6].s;
    float an1, an2;
    kexVec3 forward;
    kexVec3 start, end;

    if(actor->InstanceOf(&kexPuppet::info))
    {
        height += static_cast<kexPuppet*>(actor)->Owner()->Bob();
    }

    an1 = kexRand::Range(-hSpan, hSpan);
    an2 = kexRand::Range(-vSpan, vSpan);

    kexVec3::ToAxis(&forward, 0, 0, actor->Yaw() + an1, actor->Pitch() + an2, 0);
    start = actor->Origin() + kexVec3(0, 0, height);
    end = start + (forward * dist);

    if(cm->Trace(actor, actor->Sector(), start, end))
    {
        float x, y, z;
        kexVec3 pos = cm->InterceptVector() + (cm->ContactNormal() * 4);

        x = pos.x;
        y = pos.y;
        z = pos.z;

        game->SpawnActor(name, x, y, z, actor->Yaw(), cm->ContactSector() - game->World()->Sectors());
    }
}

//-----------------------------------------------------------------------------
//
// kexActionPlayerMelee
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexActionPlayerMelee, kexActionDef);
public:
    kexActionPlayerMelee(void);
    ~kexActionPlayerMelee(void);

    virtual void            Execute(kexActor *actor);

END_KEX_CLASS();

DECLARE_KEX_CLASS(kexActionPlayerMelee, kexActionDef)

//
// kexActionPlayerMelee::kexActionPlayerMelee
//

kexActionPlayerMelee::kexActionPlayerMelee(void)
{
}

//
// kexActionPlayerMelee::~kexActionPlayerMelee
//

kexActionPlayerMelee::~kexActionPlayerMelee(void)
{
}

//
// kexActionPlayerMelee::Execute
//

void kexActionPlayerMelee::Execute(kexActor *actor)
{
    kexCModel *cm           = kexGame::cLocal->CModel();
    float dist              = this->args[0].f;
    float height            = this->args[2].f;
    char *gotoFrameNoObj    = this->args[3].s;
    char *gotoFrameObj      = this->args[4].s;
    kexPlayer *player;
    kexVec3 forward;
    kexVec3 start, end;

    if(!actor->InstanceOf(&kexPuppet::info))
    {
        return;
    }
    
    player = static_cast<kexPuppet*>(actor)->Owner();
    height += player->Bob();

    kexVec3::ToAxis(&forward, 0, 0, actor->Yaw(), actor->Pitch(), 0);
    start = actor->Origin() + kexVec3(0, 0, height);
    end = start + (forward * dist);

    if(cm->Trace(actor, actor->Sector(), start, end))
    {
        if(cm->ContactActor())
        {
            player->Weapon().ChangeAnim(gotoFrameObj);
        }
        else
        {
            player->Weapon().ChangeAnim(gotoFrameNoObj);
        }
    }
}

//-----------------------------------------------------------------------------
//
// kexActionDefManager
//
//-----------------------------------------------------------------------------

//
// kexActionDefManager::kexActionDefManager
//

kexActionDefManager::kexActionDefManager(void)
{
}

//
// kexActionDefManager::~kexActionDefManager
//

kexActionDefManager::~kexActionDefManager(void)
{
}

//
// kexActionDefManager::RegisterAction
//

void kexActionDefManager::RegisterAction(const char *name, kexObject *(*Create)(void),
                                         const int t1, const int t2, const int t3, const int t4,
                                         const int t5, const int t6, const int t7, const int t8)
{
    actionDefInfo_t *info = actionDefInfos.Add(name);

    info->Create = Create;
    info->argTypes[0] = t1;
    info->argTypes[1] = t2;
    info->argTypes[2] = t3;
    info->argTypes[3] = t4;
    info->argTypes[4] = t5;
    info->argTypes[5] = t6;
    info->argTypes[6] = t7;
    info->argTypes[7] = t8;

    for(int i = 0; i < MAX_ACTION_DEF_ARGS; ++i)
    {
        if(info->argTypes[i] == AAT_INVALID)
        {
            info->numArgs = i;
            break;
        }
    }
}

//
// kexActionDefManager::CreateInstance
//

kexActionDef *kexActionDefManager::CreateInstance(const char *name)
{
    actionDefInfo_t *info = actionDefInfos.Find(name);
    kexActionDef *ad;

    if(!info)
    {
        return NULL;
    }

    ad = static_cast<kexActionDef*>(info->Create());

    ad->argTypes = info->argTypes;
    ad->numArgs = info->numArgs;

    ad->args = new actionDefArgs_t[ad->numArgs];

    return ad;
}

//
// kexActionDefManager::RegisterActions
//

void kexActionDefManager::RegisterActions(void)
{
    RegisterAction("A_HitScan", kexActionHitScan::info.Create,
                    AAT_FLOAT, AAT_FLOAT, AAT_FLOAT, AAT_INTEGER, AAT_INTEGER, AAT_FLOAT, AAT_STRING);
    RegisterAction("A_PlayerMelee", kexActionPlayerMelee::info.Create,
                    AAT_FLOAT, AAT_INTEGER, AAT_FLOAT, AAT_STRING, AAT_STRING);
}
