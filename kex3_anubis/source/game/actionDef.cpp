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

#define DECLARE_KEX_ACTION(cls) \
BEGIN_EXTENDED_KEX_CLASS(cls, kexActionDef);    \
public: \
    cls(void); \
    ~cls(void);    \
    virtual void            Execute(kexActor *actor);   \
END_KEX_CLASS();    \
DECLARE_KEX_CLASS(cls, kexActionDef)    \
    \
cls::cls(void) \
{   \
}   \
    \
cls::~cls(void) \
{   \
}   \
    \
void cls::Execute(kexActor *actor)

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
// kexActionScriptDef
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexActionScriptDef, kexActionDef);
public:
    kexActionScriptDef(void);
    ~kexActionScriptDef(void);

    virtual void            Execute(kexActor *actor);

    asIScriptFunction       *function;
END_KEX_CLASS();

DECLARE_KEX_CLASS(kexActionScriptDef, kexActionDef)

//
// kexActionScriptDef::kexActionScriptDef
//

kexActionScriptDef::kexActionScriptDef(void)
{
}

//
// kexActionScriptDef::~kexActionScriptDef
//

kexActionScriptDef::~kexActionScriptDef(void)
{
}

//
// kexActionScriptDef::Execute
//

void kexActionScriptDef::Execute(kexActor *actor)
{
    if(this->function == NULL)
    {
        kex::cSystem->Warning("%s contains null function pointer\n", defInfo->name.c_str());
        return;
    }

    if(!kexGame::cScriptManager->PrepareFunction(this->function))
    {
        return;
    }

    kexGame::cScriptManager->Context()->SetArgObject(0, actor);

    for(int i = 0; i < this->numArgs; ++i)
    {
        switch(this->argTypes[i])
        {
        case AAT_INTEGER:
            kexGame::cScriptManager->Context()->SetArgDWord(i+1, this->args[i].i);
            break;

        case AAT_FLOAT:
            kexGame::cScriptManager->Context()->SetArgFloat(i+1, this->args[i].f);
            break;

        case AAT_STRING:
            kexGame::cScriptManager->Context()->SetArgAddress(i+1, &this->args[i].s);
            break;

        default:
            kexGame::cScriptManager->PopState();
            return;
        }
    }

    if(!kexGame::cScriptManager->Execute())
    {
        return;
    }
}

//-----------------------------------------------------------------------------
//
// kexActionPrintf
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionPrintf)
{
    char *string = this->args[0].s;
    kex::cSystem->Printf(string);
}

//-----------------------------------------------------------------------------
//
// kexActionHitScan
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionHitScan)
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

        if(cm->ContactActor() && cm->ContactActor()->Flags() & AF_SHOOTABLE)
        {
            cm->ContactActor()->InflictDamage(actor, this->args[3].i);
        }
        else
        {
            game->SpawnActor(name, x, y, z, actor->Yaw(),
                             cm->ContactSector() - game->World()->Sectors());
        }
    }
}

//-----------------------------------------------------------------------------
//
// kexActionPlayerMelee
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionPlayerMelee)
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
            
            if(cm->ContactActor()->Flags() & AF_SHOOTABLE)
            {
                cm->ContactActor()->InflictDamage(actor, this->args[1].i);
            }
        }
        else
        {
            player->Weapon().ChangeAnim(gotoFrameNoObj);
        }
    }
}

//-----------------------------------------------------------------------------
//
// kexActionSpawn
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionSpawn)
{
    kexGameLocal *game  = kexGame::cLocal;
    char *defName       = this->args[0].s;
    float x             = this->args[1].f + actor->Origin().x;
    float y             = this->args[2].f + actor->Origin().y;
    float z             = this->args[3].f + actor->Origin().z;

    game->SpawnActor(defName, x, y, z, actor->Yaw(),
                     actor->Sector() - game->World()->Sectors());
}

//-----------------------------------------------------------------------------
//
// kexActionTossActor
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionTossActor)
{
    kexGameLocal *game  = kexGame::cLocal;
    kexActor *toss;
    char *defName       = this->args[0].s;
    float x             = this->args[1].f + actor->Origin().x;
    float y             = this->args[2].f + actor->Origin().y;
    float z             = this->args[3].f + actor->Origin().z;
    float xSpread       = this->args[4].f;
    float ySpread       = this->args[5].f;
    float zSpreadMin    = this->args[6].f;
    float zSpreadMax    = this->args[7].f;
    
    toss = game->SpawnActor(defName, x, y, z, actor->Yaw(),
                            actor->Sector() - game->World()->Sectors());
    
    toss->Velocity().x += kexRand::Range(-xSpread, xSpread);
    toss->Velocity().y += kexRand::Range(-ySpread, ySpread);
    toss->Velocity().z += kexRand::Range(zSpreadMin, zSpreadMax);
}

//-----------------------------------------------------------------------------
//
// kexActionDestroy
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionDestroy)
{
    if(actor->Removing())
    {
        kex::cSystem->Warning("A_Destroy: Actor is already being removed\n");
        return;
    }
    
    actor->Remove();
}

//-----------------------------------------------------------------------------
//
// kexActionDestroyAtRest
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionDestroyAtRest)
{
    float min = this->args[0].f;

    if(actor->Origin().z > actor->FloorHeight() ||
       actor->FloorHeight() - actor->Origin().z > actor->Height())
    {
        return;
    }
    
    if(actor->Velocity().UnitSq() > min)
    {
        return;
    }
    
    actor->Remove();
}

//-----------------------------------------------------------------------------
//
// kexActionRadialBlast
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionRadialBlast)
{
    kexGameLocal *game  = kexGame::cLocal;
    float radius        = this->args[0].f;
    int damage          = this->args[1].i;
    
    game->World()->RadialDamage(actor, radius, damage);
}

//-----------------------------------------------------------------------------
//
// kexActionConsumeAmmo
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionConsumeAmmo)
{
    int amount          = this->args[0].i;
    kexPlayer *player;
    
    if(!actor->InstanceOf(&kexPuppet::info))
    {
        return;
    }
    
    player = static_cast<kexPuppet*>(actor)->Owner();
    player->ConsumeAmmo((int16_t)amount);
}

//-----------------------------------------------------------------------------
//
// kexActionPlaySound
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionPlaySound)
{
    actor->PlaySound(this->args[0].s);
}

//-----------------------------------------------------------------------------
//
// kexActionFaceTarget
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionFaceTarget)
{
    kexActor *targ;
    float x, y;
    
    if(!actor->Target())
    {
        return;
    }
    
    targ = static_cast<kexActor*>(actor->Target());
    
    x = targ->Origin().x - actor->Origin().x;
    y = targ->Origin().y - actor->Origin().y;
    
    actor->Yaw() = kexMath::ATan2(x, y);
}

//-----------------------------------------------------------------------------
//
// kexActionCheckMelee
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionCheckMelee)
{
    char *gotoFrame = this->args[0].s;
    float extra     = this->args[1].f;
    float r;
    kexActor *targ;
    kexAI *ai;
    
    if(!actor->InstanceOf(&kexAI::info))
    {
        return;
    }
    
    ai = static_cast<kexAI*>(actor);
    
    targ = static_cast<kexActor*>(ai->Target());
    r = ((ai->Radius() * 0.6f) + targ->Radius()) + extra;
    
    if(ai->Origin().DistanceSq(targ->Origin()) > (r * r))
    {
        ai->ChangeAnim(gotoFrame);
    }
}

//-----------------------------------------------------------------------------
//
// kexActionSpawnProjectile
//
//-----------------------------------------------------------------------------

DECLARE_KEX_ACTION(kexActionSpawnProjectile)
{
    kexGameLocal *game  = kexGame::cLocal;
    char *defName       = this->args[0].s;
    float yaw           = this->args[1].f + actor->Yaw();
    float pitch         = this->args[2].f + actor->Pitch();
    float dist          = this->args[3].f + actor->Radius();
    float offset        = this->args[4].f;
    float speed         = this->args[5].f;
    kexVec3 forward;
    kexActor *proj, *targ;
    float x, y, z;
    
    kexVec3::ToAxis(&forward, 0, 0, yaw, pitch, 0);
    
    x = actor->Origin().x + (forward.x * dist);
    y = actor->Origin().y + (forward.y * dist);
    z = actor->Origin().z + (forward.z * dist);
    
    proj = game->SpawnActor(defName, x, y, z + offset, actor->Yaw(),
                            actor->Sector() - game->World()->Sectors());
    
    kexVec3::ToAxis(&forward, 0, 0, actor->Yaw(), actor->Pitch(), 0);
    
    proj->Velocity() = (forward * speed);
    proj->SetTarget(actor);

    if(actor->InstanceOf(&kexPuppet::info))
    {
        return;
    }
    
    if(!actor->Target() || speed == 0)
    {
        return;
    }
    
    targ = static_cast<kexActor*>(actor->Target());
    
    x = targ->Origin().x - proj->Origin().x;
    y = targ->Origin().y - proj->Origin().y;
    
    dist = kexMath::Sqrt(x * x + y * y) / speed;
    
    if(dist < 1)
    {
        dist = 1;
    }
    
    offset = (targ->Origin().z + (targ->Height() * 0.5f));
    proj->Velocity().z = (offset - proj->Origin().z) / dist;
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

    info->name = name;
    info->Create = Create;
    info->argTypes[0] = t1;
    info->argTypes[1] = t2;
    info->argTypes[2] = t3;
    info->argTypes[3] = t4;
    info->argTypes[4] = t5;
    info->argTypes[5] = t6;
    info->argTypes[6] = t7;
    info->argTypes[7] = t8;

    info->numArgs = MAX_ACTION_DEF_ARGS;
    
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
// kexActionDefManager::RegisterScriptAction
//

void kexActionDefManager::RegisterScriptAction(const char *name, kexStrList &argTypes, unsigned int numArgs)
{
    actionDefInfo_t *info;
    int args[MAX_ACTION_DEF_ARGS] = { AAT_INVALID, AAT_INVALID,
                                      AAT_INVALID, AAT_INVALID,
                                      AAT_INVALID, AAT_INVALID,
                                      AAT_INVALID, AAT_INVALID };

    // first arg should always be kActor@ so ignore it
    for(unsigned int i = 1; i < numArgs; ++i)
    {
             if(argTypes[i] == "float")   args[i-1] = AAT_FLOAT;
        else if(argTypes[i] == "int")     args[i-1] = AAT_INTEGER;
        else if(argTypes[i] == "kStr&")   args[i-1] = AAT_STRING;
        else if(argTypes[i] == "kStr")    args[i-1] = AAT_STRING;
        else
        {
            kex::cSystem->Warning("%s has unknown argument type (arg %i)\n", name, i);
            return;
        }
    }

    info = actionDefInfos.Add(name);
    info->name = name;
    info->numArgs = numArgs-1;
    info->Create = kexActionScriptDef::info.Create;

    for(int i = 0; i < MAX_ACTION_DEF_ARGS; ++i)
    {
        info->argTypes[i] = args[i];
    }
}

//
// kexActionDefManager::CreateInstance
//

kexActionDef *kexActionDefManager::CreateInstance(const char *name)
{
    actionDefInfo_t *info = actionDefInfos.Find(name);
    asIScriptFunction **f;
    kexActionDef *ad;

    if(!info)
    {
        return NULL;
    }

    if((f = kexGame::cScriptManager->ActionList().Find(name)))
    {
        kexActionScriptDef *asd = static_cast<kexActionScriptDef*>(info->Create());
        asd->function = *f;
        
        ad = static_cast<kexActionDef*>(asd);
    }
    else
    {
        ad = static_cast<kexActionDef*>(info->Create());
    }

    ad->argTypes = info->argTypes;
    ad->numArgs = info->numArgs;
    ad->defInfo = info;

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
    RegisterAction("A_Spawn", kexActionSpawn::info.Create,
                    AAT_STRING, AAT_FLOAT, AAT_FLOAT, AAT_FLOAT);
    RegisterAction("A_Printf", kexActionPrintf::info.Create, AAT_STRING);
    RegisterAction("A_Destroy", kexActionDestroy::info.Create);
    RegisterAction("A_TossActor", kexActionTossActor::info.Create,
                   AAT_STRING, AAT_FLOAT, AAT_FLOAT, AAT_FLOAT, AAT_FLOAT, AAT_FLOAT, AAT_FLOAT, AAT_FLOAT);
    RegisterAction("A_DestroyAtRest", kexActionDestroyAtRest::info.Create, AAT_FLOAT);
    RegisterAction("A_RadialBlast", kexActionRadialBlast::info.Create, AAT_FLOAT, AAT_INTEGER);
    RegisterAction("A_ConsumeAmmo", kexActionConsumeAmmo::info.Create, AAT_INTEGER);
    RegisterAction("A_PlayLocalSound", kexActionPlaySound::info.Create, AAT_STRING);
    RegisterAction("A_FaceTarget", kexActionFaceTarget::info.Create);
    RegisterAction("A_CheckMelee", kexActionCheckMelee::info.Create, AAT_STRING, AAT_FLOAT);
    RegisterAction("A_FireProjectile", kexActionSpawnProjectile::info.Create,
                   AAT_STRING, AAT_FLOAT, AAT_FLOAT, AAT_FLOAT, AAT_FLOAT, AAT_FLOAT);
}
