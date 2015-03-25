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
//      Player class
//

#include "kexlib.h"
#include "game.h"

kexCvar cvarAutoAim("g_autoaim", CVF_BOOL|CVF_CONFIG, "1", "Enable auto aiming");

#define PMOVE_MIN               0.125f
#define PMOVE_SPEED             0.976f
#define PMOVE_WATER_SPEED       0.48828125f
#define PMOVE_SPEED_JUMP        10.25f
#define PMOVE_JUMP_BOOST        1.5f
#define PMOVE_MAX_JUMPTICKS     13

const int16_t kexPlayer::maxHealth = 200;

//-----------------------------------------------------------------------------
//
// kexPuppet
//
// The actor in the world controlled by the client
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexPuppet, kexActor)

//
// kexPuppet::kexPuppet
//

kexPuppet::kexPuppet(void)
{
    this->owner = NULL;
    this->playerFlags = 0;
    this->jumpTicks = 0;
}

//
// kexPuppet::~kexPuppet
//

kexPuppet::~kexPuppet(void)
{
}

//
// kexPuppet::OnCollide
//

bool kexPuppet::OnCollide(kexCModel *cmodel)
{
    if(cmodel->ContactFace())
    {
        mapFace_t *face = cmodel->ContactFace();

        if(face->flags & FF_FORCEFIELD)
        {
            owner->LockTime() = 60;
            InflictDamage(NULL, 50);
            velocity.Clear();
            movement.Clear();
        }
        if(face->flags & FF_LAVA)
        {
            // TODO
        }
        if(face->flags & FF_SLIME)
        {
            // TODO
        }
    }

    return true;
}

//
// kexPuppet::OnDamage
//

void kexPuppet::OnDamage(kexActor *instigator)
{
    if(owner->LockTime() > 0)
    {
        kexGame::cLocal->PlayLoop()->ElectrocuteFlash();
        PlaySound("sounds/pelectrocute.wav");
        return;
    }

    kexGame::cLocal->PlayLoop()->DamageFlash();

    if(flags & AF_INWATER && !(playerFlags & PF_INWATERSURFACE))
    {
        switch(kexRand::Max(5))
        {
        case 0:
            PlaySound("sounds/pwpain01.wav");
            break;
        case 1:
            PlaySound("sounds/pwpain02.wav");
            break;
        case 2:
            PlaySound("sounds/pwpain03.wav");
            break;
        case 3:
            PlaySound("sounds/pwpain04.wav");
            break;
        case 4:
            PlaySound("sounds/pwpain05.wav");
            break;
        }
    }
    else
    {
        switch(kexRand::Max(3))
        {
        case 0:
            PlaySound("sounds/ppain01.wav");
            break;
        case 1:
            PlaySound("sounds/ppain02.wav");
            break;
        case 2:
            PlaySound("sounds/ppain03.wav");
            break;
        }
    }
}

//
// kexPuppet::Jump
//

void kexPuppet::Jump(kexPlayerCmd *cmd)
{
    if(cmd->Buttons() & BC_JUMP && !(sector->floorFace->flags & FF_WATER))
    {
        if(!(playerFlags & PF_USERJUMPED))
        {
            if(!(playerFlags & PF_JUMPING))
            {
                // let the actor object know that
                // it is now jumping
                playerFlags |= PF_JUMPING;

                switch(kexRand::Max(5))
                {
                case 0:
                    PlaySound("sounds/pjump01.wav");
                    break;
                case 1:
                    PlaySound("sounds/pjump02.wav");
                    break;
                case 2:
                    PlaySound("sounds/pjump03.wav");
                    break;
                case 3:
                    PlaySound("sounds/pjump04.wav");
                    break;
                case 4:
                    PlaySound("sounds/pjump05.wav");
                    break;
                }
            }

            // handle longer jumps if holding down
            // the jump key/button
            if(jumpTicks < PMOVE_MAX_JUMPTICKS)
            {
                velocity.z = PMOVE_SPEED_JUMP;
                if(owner->Artifacts() & PA_SANDALS)
                {
                    velocity.z *= PMOVE_JUMP_BOOST;
                }

                jumpTicks++;
            }
            else
            {
                // let client know that we're jumping and
                // no longer have control of vertical movement
                playerFlags |= PF_USERJUMPED;
            }
        }
    }
    else
    {
        // jump key/button was released, check if we're actually moving first
        if((velocity.z >= PMOVE_MIN || velocity.z <= -(PMOVE_MIN * 48)) || playerFlags & PF_JUMPING)
        {
            // once the jump key/button is released, we
            // no longer have control of vertical movement
            playerFlags |= PF_USERJUMPED;
        }
        else if(playerFlags & PF_USERJUMPED)
        {
            // allow jump inputs from the client
            playerFlags &= ~PF_USERJUMPED;
        }
    }
}

//
// kexPuppet::GroundMove
//

void kexPuppet::GroundMove(kexPlayerCmd *cmd)
{
    kexVec3 forward, right;
    mapSector_t *oldSector;
    
    // update angles
    yaw += cmd->Angles()[0];
    pitch += cmd->Angles()[1];
    roll -= (cmd->Angles()[0] * 0.25f);
    
    kexMath::Clamp(pitch.an, kexMath::Deg2Rad(-90), kexMath::Deg2Rad(90));
    kexMath::Clamp(roll.an, -0.1f, 0.1f);
    
    kexVec3::ToAxis(&forward, NULL, &right, yaw, 0, 0);

    // apply friction
    velocity.x *= friction;
    velocity.y *= friction;

    // handle jumping
    Jump(cmd);
    
    // check for drop-offs
    if(origin.z > floorHeight)
    {
        velocity.z -= gravity;
    }

    if(kexMath::Fabs(velocity.x) < PMOVE_MIN)
    {
        velocity.x = 0;
    }

    if(kexMath::Fabs(velocity.y) < PMOVE_MIN)
    {
        velocity.y = 0;
    }

    if(!(cmd->Buttons() & (BC_FORWARD|BC_BACKWARD)))
    {
        if(cmd->Movement()[0] != 0)
        {
            velocity.x += (forward.x * cmd->Movement()[0]) * PMOVE_SPEED;
            velocity.y += (forward.y * cmd->Movement()[0]) * PMOVE_SPEED;
        }
    }

    if(!(cmd->Buttons() & (BC_STRAFELEFT|BC_STRAFERIGHT)))
    {
        if(cmd->Movement()[1] != 0)
        {
            velocity.x += (right.x * cmd->Movement()[1]) * PMOVE_SPEED;
            velocity.y += (right.y * cmd->Movement()[1]) * PMOVE_SPEED;
        }
    }

    if(cmd->Buttons() & BC_FORWARD)
    {
        velocity.x += forward.x * PMOVE_SPEED;
        velocity.y += forward.y * PMOVE_SPEED;
    }

    if(cmd->Buttons() & BC_BACKWARD)
    {
        velocity.x -= forward.x * PMOVE_SPEED;
        velocity.y -= forward.y * PMOVE_SPEED;
    }

    if(cmd->Buttons() & BC_STRAFELEFT)
    {
        velocity.x -= right.x * PMOVE_SPEED;
        velocity.y -= right.y * PMOVE_SPEED;
    }

    if(cmd->Buttons() & BC_STRAFERIGHT)
    {
        velocity.x += right.x * PMOVE_SPEED;
        velocity.y += right.y * PMOVE_SPEED;
    }

    // bump ceiling
    if((origin.z + height) + velocity.z >= ceilingHeight)
    {
        origin.z = ceilingHeight - height;
        velocity.z = -1;
        playerFlags |= PF_USERJUMPED;
        playerFlags &= ~PF_JUMPING;
    }
    
    // bump floor
    if(origin.z + velocity.z <= floorHeight)
    {
        if(velocity.z < -3.5f)
        {
            owner->LandTime() = velocity.z;
        }

        origin.z = floorHeight;
        velocity.z = 0;
        jumpTicks = 0;
        playerFlags &= ~PF_JUMPING;
    }

    oldSector = sector;
    movement = velocity;
    
    if(!kexGame::cLocal->CModel()->MoveActor(this))
    {
        velocity.Clear();
        movement.Clear();
    }

    if(oldMovement.ToVec2().Dot(movement.ToVec2()) < 0)
    {
        movement.x = movement.y = 0;
        velocity.x = velocity.y = 0;
    }

    oldMovement = movement;

    // handle smooth stepping when going down on slopes
    if(oldSector == sector && velocity.z <= 0 && sector->floorFace->plane.IsFacing(velocity.ToYaw()))
    {
        float diff = origin.z - floorHeight;

        if(diff > 0 && diff <= 8)
        {
            origin.z = floorHeight;
        }
    }

    if(!(oldSector->flags & SF_WATER) && sector->flags & SF_WATER)
    {
        this->PlaySound("sounds/splash02.wav");
        this->flags |= AF_INWATER;
    }
    else if(!(sector->flags & SF_WATER))
    {
        this->flags &= ~AF_INWATER;
    }

    if(oldSector != sector)
    {
        kexGame::cLocal->World()->EnterSectorSpecial(this, sector);
    }
}

//
// kexPuppet::WaterMove
//

void kexPuppet::WaterMove(kexPlayerCmd *cmd)
{
    kexVec3 forward, right;
    mapSector_t *oldSector;
    
    yaw += cmd->Angles()[0];
    pitch += cmd->Angles()[1];
    roll -= (cmd->Angles()[0] * 0.25f);
    
    kexMath::Clamp(pitch.an, kexMath::Deg2Rad(-90), kexMath::Deg2Rad(90));
    kexMath::Clamp(roll.an, -0.1f, 0.1f);

    kexVec3::ToAxis(&forward, NULL, &right, yaw, pitch, 0);

    velocity *= friction;

    if(kexMath::Fabs(velocity.x) < PMOVE_MIN)
    {
        velocity.x = 0;
    }

    if(kexMath::Fabs(velocity.y) < PMOVE_MIN)
    {
        velocity.y = 0;
    }

    if(kexMath::Fabs(velocity.z) < PMOVE_MIN)
    {
        velocity.z = 0;
    }

    if(cmd->Buttons() & BC_FORWARD)
    {
        velocity += (forward * PMOVE_WATER_SPEED);
    }
    else if(cmd->Movement()[0] > 0)
    {
        velocity += (forward * cmd->Movement()[0]) * PMOVE_WATER_SPEED;
    }

    if(cmd->Buttons() & BC_BACKWARD)
    {
        velocity -= (forward * (PMOVE_WATER_SPEED*0.5f));
    }
    else if(cmd->Movement()[0] < 0)
    {
        velocity += (forward * cmd->Movement()[0]) * (PMOVE_WATER_SPEED*0.5f);
    }

    if(sector->ceilingFace->flags & FF_WATER)
    {
        float waterZ = kexGame::cLocal->CModel()->GetCeilingHeight(origin, sector);
        float waterheight = waterZ - origin.z;

        if(waterheight < ((height + (owner->ViewZ() * 0.5f)) * 0.5f))
        {
            float t = (waterheight < owner->ViewZ()) ? 0.035f : 0.075f;
            origin.z = ((waterZ - owner->ViewZ()) - origin.z) * t + origin.z;
            playerFlags |= PF_INWATERSURFACE;
        }
        else
        {
            playerFlags &= ~PF_INWATERSURFACE;
        }
    }

    // bump ceiling
    if((origin.z + height) + velocity.z >= ceilingHeight)
    {
        origin.z = ceilingHeight - height;
    }
    
    // bump floor
    if(origin.z + velocity.z <= floorHeight)
    {
        origin.z = floorHeight;
    }

    oldSector = sector;
    movement = velocity;
    
    if(!kexGame::cLocal->CModel()->MoveActor(this))
    {
        velocity.Clear();
        movement.Clear();
    }

    if(!(sector->flags & SF_WATER))
    {
        this->flags &= ~AF_INWATER;
    }

    if(oldSector != sector)
    {
        kexGame::cLocal->World()->EnterSectorSpecial(this, sector);
    }
}

//
// kexPuppet::FlyMove
//

void kexPuppet::FlyMove(kexPlayerCmd *cmd)
{
    kexVec3 forward, right;
    
    yaw += cmd->Angles()[0];
    pitch += cmd->Angles()[1];

    kexMath::Clamp(pitch.an, kexMath::Deg2Rad(-90), kexMath::Deg2Rad(90));
    kexVec3::ToAxis(&forward, NULL, &right, yaw, pitch, 0);

    velocity *= friction;

    if(kexMath::Fabs(velocity.x) < PMOVE_MIN)
    {
        velocity.x = 0;
    }

    if(kexMath::Fabs(velocity.y) < PMOVE_MIN)
    {
        velocity.y = 0;
    }

    if(kexMath::Fabs(velocity.z) < PMOVE_MIN)
    {
        velocity.z = 0;
    }

    if(cmd->Buttons() & BC_FORWARD)
    {
        velocity += (forward * PMOVE_SPEED);
    }

    if(cmd->Buttons() & BC_BACKWARD)
    {
        velocity -= (forward * PMOVE_SPEED);
    }

    if(cmd->Buttons() & BC_STRAFELEFT)
    {
        velocity.x -= right.x * PMOVE_SPEED;
        velocity.y -= right.y * PMOVE_SPEED;
    }

    if(cmd->Buttons() & BC_STRAFERIGHT)
    {
        velocity.x += right.x * PMOVE_SPEED;
        velocity.y += right.y * PMOVE_SPEED;
    }

    movement = velocity;
    
    if(!(playerFlags & PF_NOCLIP))
    {
        if(!kexGame::cLocal->CModel()->MoveActor(this))
        {
            velocity.Clear();
            movement.Clear();
        }
    }
    else
    {
        origin += movement;
        FindSector(origin);
        LinkArea();
    }
}

//
// kexPuppet::Tick
//

void kexPuppet::Tick(void)
{
    kexPlayerCmd *cmd = &owner->Cmd();
    
    roll = (0 - roll) * 0.35f + roll;

    if(owner->LockTime() > 0)
    {
        return;
    }
    
    if(playerFlags & (PF_NOCLIP|PF_FLY))
    {
        FlyMove(cmd);
    }
    else if(flags & AF_INWATER)
    {
        WaterMove(cmd);
    }
    else
    {
        GroundMove(cmd);
    }

    gameTicks++;
}

//
// kexPuppet::Spawn
//

void kexPuppet::Spawn(void)
{
    owner = kexGame::cLocal->Player();
    kexGame::cLocal->Player()->SetActor(this);

    radius      = 95.25f;
    height      = 160;
    stepHeight  = 48;
    health      = 200;
    friction    = 0.9375f;
    flags       = (AF_SOLID|AF_SHOOTABLE);
}

//-----------------------------------------------------------------------------
//
// kexPlayer
//
//-----------------------------------------------------------------------------

//
// kexPlayer::kexPlayer
//

kexPlayer::kexPlayer(void)
{
    this->weapon.owner = this;
}

//
// kexPlayer::~kexPlayer
//

kexPlayer::~kexPlayer(void)
{
}

//
// kexPlayer::Reset
//

void kexPlayer::Reset(void)
{
    health = maxHealth;
    ankahs = 0;
    actor = NULL;

    cmd.Reset();

    bob = 0;
    bobTime = 0;
    bobSpeed = 0;
    landTime = 0;
    stepViewZ = 0;
    viewZ = 64.0f;
    airSupply = 64;
    airSupplyTime = 0;

    memset(weapons, 0, NUMPLAYERWEAPONS);

    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        if(kexGame::cLocal->WeaponInfo(i)->bPersistent)
        {
            weapons[i] = true;
            currentWeapon = static_cast<playerWeapons_t>(i);
            pendingWeapon = currentWeapon;
        }
        
        ammo[i] = kexGame::cLocal->WeaponInfo(i)->maxAmmo;
    }

    artifacts = 0;
    keys = 0;
    transmitter = 0;
    teamDolls = 0;
}

//
// kexPlayer::Ready
//

void kexPlayer::Ready(void)
{
    bob = 0;
    bobTime = 0;
    bobSpeed = 0;
    landTime = 0;
    stepViewZ = 0;
    keys = 0;
    airSupply = 64;
    airSupplyTime = 0;

    weapon.ChangeAnim(WS_RAISE);
}

//
// kexPlayer::UpdateViewBob
//

void kexPlayer::UpdateViewBob(void)
{
    if(actor->Origin().z > actor->FloorHeight() && !(actor->Flags() & AF_INWATER))
    {
        bob = 0;
        bobTime = 0;
        bobSpeed = 0;
    }
    else
    {
        if( cmd.Buttons() & (BC_FORWARD|BC_BACKWARD|BC_STRAFELEFT|BC_STRAFERIGHT) ||
            actor->Flags() & AF_INWATER || (cmd.Movement()[0] != 0 || cmd.Movement()[1] != 0))
        {
            float speed = (actor->Flags() & AF_INWATER) ? 0.25f : 1;

            bobSpeed = (0.148f - bobSpeed) * 0.35f + bobSpeed;
            bob = kexMath::Sin(bobTime * speed) * 7.0f;
            bobTime += bobSpeed;
        }
        else
        {
            bobSpeed = 0;
            bobTime = 0;
            bob = (0 - bob) * 0.1f + bob;
        }
    }

    if(landTime < 0)
    {
        landTime += 2;
        if(landTime >= 0)
        {
            landTime = 0;
        }
    }

    if(stepViewZ < 0)
    {
        stepViewZ += 8;
        if(stepViewZ >= 0)
        {
            stepViewZ = 0;
        }
    }
}

//
// kexPlayer::UpdateAirSupply
//

void kexPlayer::UpdateAirSupply(void)
{
    int maxAirSupplyTime;

    if(!(actor->Flags() & AF_INWATER) || actor->PlayerFlags() & PF_INWATERSURFACE)
    {
        airSupplyTime = 0;
        airSupply += 8;
        if(airSupply > 64)
        {
            airSupply = 64;
        }
        return;
    }

    airSupplyTime++;
    maxAirSupplyTime = (artifacts & PA_MASK) ? 100 : 5;
    
    if(airSupplyTime >= maxAirSupplyTime)
    {
        if(airSupply > 0)
        {
            airSupplyTime = 0;
            airSupply -= 2;
            if(airSupply < 0)
            {
                airSupply = 0;
            }
            else if(artifacts & PA_MASK && (airSupply & 3) == 2)
            {
                actor->PlaySound("sounds/pwbreathe.wav");
            }
        }
        else
        {
            if((airSupplyTime & 0x7f) == 0)
            {
                actor->InflictDamage(NULL, airSupplyTime >> 2);
            }
        }
    }
}

//
// kexPlayer::TryUse
//

void kexPlayer::TryUse(void)
{
    kexVec3 start = actor->Origin() + kexVec3(0, 0, viewZ);
    kexVec3 end, forward;
    
    kexVec3::ToAxis(&forward, 0, 0, actor->Yaw(), actor->Pitch(), 0);
    end = start + (forward * (actor->Radius() + 64));
    
    if(kexGame::cLocal->CModel()->Trace(actor, actor->Sector(), start, end))
    {
        mapFace_t *useFace = kexGame::cLocal->CModel()->ContactFace();
        
        if(useFace)
        {
            kexGame::cLocal->World()->UseWallSpecial(this, useFace);
        }
    }
}

//
// kexPlayer::AutoAim
//

kexActor *kexPlayer::AutoAim(const kexVec3 &start, kexAngle &yaw, kexAngle &pitch,
                             const float dist, const float aimAngle)
{
    kexStack<mapSector_t*> *sectorList;
    kexActor *aimActor;
    kexVec3 aVec, aDir;
    kexAngle bestYaw, bestPitch;
    kexAngle aYaw, aPitch;
    float maxDist = kexMath::infinity;

    if(cvarAutoAim.GetBool() == false)
    {
        return NULL;
    }

    bestYaw = yaw;
    bestPitch = pitch;
    aimActor = NULL;
    sectorList = kexGame::cLocal->World()->FloodFill(start, actor->Sector(), dist);

    for(unsigned int i = 0; i < sectorList->CurrentLength(); ++i)
    {
        for(kexActor *a = (*sectorList)[i]->actorList.Next(); a != NULL; a = a->SectorLink().Next())
        {
            if(a->Flags() & AF_HIDDEN)
            {
                continue;
            }

            if(a == actor || a->Health() <= 0 || !(a->Flags() & AF_SHOOTABLE))
            {
                continue;
            }

            aVec = a->Origin() + kexVec3(0, 0, a->Height() * 0.5f);

            if(!actor->CanSee(aVec, maxDist))
            {
                continue;
            }

            aDir = (aVec - start);

            aYaw = aDir.ToYaw();
            aPitch = -aDir.ToPitch();

            if(kexMath::Fabs(yaw.Diff(aYaw)) > aimAngle) continue;
            if(kexMath::Fabs(pitch.Diff(aPitch)) > aimAngle) continue;

            bestYaw = aYaw;
            bestPitch = aPitch;
            aimActor = a;
            maxDist = start.Distance(aVec);
        }
    }

    yaw = bestYaw;
    pitch = bestPitch;

    return aimActor;
}

//
// kexPlayer::GiveWeapon
//

bool kexPlayer::GiveWeapon(const int weaponID, const bool bAutoSwitch)
{
    if(weaponID <= -1 || weaponID >= NUMPLAYERWEAPONS)
    {
        return false;
    }
    
    if(weapons[weaponID])
    {
        return false;
    }
    
    weapons[weaponID] = true;
    
    if(bAutoSwitch)
    {
        pendingWeapon = static_cast<playerWeapons_t>(weaponID);
    }
    return true;
}

//
// kexPlayer::WeaponOwned
//

bool kexPlayer::WeaponOwned(const int weaponID)
{
    if(weaponID <= -1 || weaponID >= NUMPLAYERWEAPONS)
    {
        return false;
    }
    
    return weapons[weaponID];
}

//
// kexPlayer::ConsumeAmmo
//

void kexPlayer::ConsumeAmmo(const int16_t amount)
{
    if(amount < 0)
    {
        return;
    }
    
    ammo[currentWeapon] -= amount;
    
    if(ammo[currentWeapon] < 0)
    {
        ammo[currentWeapon] = 0;
    }
}

//
// kexPlayer::GetAmmo
//

const int16_t kexPlayer::GetAmmo(const int weaponID)
{
    return ammo[weaponID];
}

//
// kexPlayer::GiveAmmo
//

void kexPlayer::GiveAmmo(const int weaponID, int16_t amount)
{
    ammo[weaponID] += amount;
    if(ammo[weaponID] > kexGame::cLocal->WeaponInfo(weaponID)->maxAmmo)
    {
        ammo[weaponID] = kexGame::cLocal->WeaponInfo(weaponID)->maxAmmo;
    }
}

//
// kexPlayer::GiveHealth
//

bool kexPlayer::GiveHealth(const int amount)
{
    if(actor->Health() >= maxHealth)
    {
        return false;
    }

    actor->Health() += amount;

    if(actor->Health() > maxHealth)
    {
        actor->Health() = maxHealth;
    }

    return true;
}

//
// kexPlayer::GiveKey
//

bool kexPlayer::GiveKey(const int key)
{
    int keyFlag = BIT(key);

    if(this->keys & keyFlag)
    {
        return false;
    }

    this->keys |= keyFlag;
    return true;
}

//
// kexPlayer::CycleNextWeapon
//

void kexPlayer::CycleNextWeapon(void)
{
    int setWeapon = pendingWeapon;

    do
    {
        if(++setWeapon >= NUMPLAYERWEAPONS)
        {
            setWeapon = PW_MACHETE;
        }

        if(weapons[setWeapon] == true)
        {
            pendingWeapon = static_cast<playerWeapons_t>(setWeapon);
            break;
        }

    } while(setWeapon != currentWeapon);
}

//
// kexPlayer::CyclePrevWeapon
//

void kexPlayer::CyclePrevWeapon(void)
{
    int setWeapon = pendingWeapon;

    do
    {
        if(--setWeapon < 0)
        {
            setWeapon = NUMPLAYERWEAPONS-1;
        }

        if(weapons[setWeapon] == true)
        {
            pendingWeapon = static_cast<playerWeapons_t>(setWeapon);
            break;
        }

    } while(setWeapon != currentWeapon);
}

//
// kexPlayer::Tick
//

void kexPlayer::Tick(void)
{
    if(cmd.Buttons() & BC_WEAPONLEFT)
    {
        CyclePrevWeapon();
    }
    if(cmd.Buttons() & BC_WEAPONRIGHT)
    {
        CycleNextWeapon();
    }
    if(cmd.Buttons() & BC_USE)
    {
        TryUse();
    }
    if(cmd.Buttons() & BC_MAPZOOMIN)
    {
        kexGame::cLocal->PlayLoop()->ZoomAutomap(-128);
    }
    if(cmd.Buttons() & BC_MAPZOOMOUT)
    {
        kexGame::cLocal->PlayLoop()->ZoomAutomap(128);
    }

    if(lockTime > 0)
    {
        lockTime--;
        return;
    }

    UpdateViewBob();
    UpdateAirSupply();

    weapon.Update();
}
