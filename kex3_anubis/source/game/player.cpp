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
//      Player and puppet classes. The player object must have an
//      active puppet actor that exists in the world that it can control.
//      The puppet actor handles all the movement and interaction behavior
//      while the player controller contains all the player-specific data
//      such as ammo, inventory, artifacts, etc
//

#include "kexlib.h"
#include "game.h"

kexCvar kexPlayer::cvarAutoAim("g_autoaim", CVF_BOOL|CVF_CONFIG, "1", "Enable auto aiming");

#define PMOVE_MIN               0.125f
#define PMOVE_SPEED             0.976f
#define PMOVE_WATER_SPEED       0.48828125f
#define PMOVE_SPEED_JUMP        10.25f
#define PMOVE_JUMP_BOOST        15.25f

const int16_t kexPlayer::maxHealth = 200;

//-----------------------------------------------------------------------------
//
// kexPuppet
//
// The actor in the world controlled by the client
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexPuppet, kexActor)

kexVec3 kexPuppet::mapDestinationPosition;
bool kexPuppet::bScheduleNextMapWarp = false;

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
    if(playerFlags & PF_DEAD)
    {
        return true;
    }

    if(cmodel->ContactFace())
    {
        mapFace_t *face = cmodel->ContactFace();

        if(face->flags & FF_FORCEFIELD)
        {
            if(owner->Artifacts() & PA_SCEPTER)
            {
                if(face->sector >= 0)
                {
                    kexWorld *world = kexGame::cWorld;
                    mapSector_t *sector = &world->Sectors()[face->sector];

                    for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
                    {
                        mapFace_t *f = &world->Faces()[i];

                        if(f->flags & FF_FORCEFIELD && f->sector == face->sectorOwner)
                        {
                            f->flags &= ~(FF_SOLID|FF_FORCEFIELD);
                            f->flags |= (FF_INVISIBLE|FF_HIDDEN);
                            f->polyStart = f->polyEnd = -1;
                        }
                    }
                }

                face->flags &= ~(FF_SOLID|FF_FORCEFIELD);
                face->flags |= (FF_INVISIBLE|FF_HIDDEN);
                face->polyStart = face->polyEnd = -1;

                PlaySound("sounds/forcefieldoff.wav");
            }
            else
            {
                playerFlags |= PF_ELECTROCUTE;
                InflictDamage(NULL, 50);
            }
        }
        if(face->flags & FF_LAVA)
        {
            LavaDamage(face);
        }
        if(face->flags & FF_SLIME)
        {
            SlimeDamage();
        }
    }
    else if(cmodel->ContactActor())
    {
        if(cmodel->ContactActor()->InstanceOf(&kexTravelObject::info))
        {
            cmodel->ContactActor()->OnTouch(this);
        }
    }

    return true;
}

//
// kexPuppet::OnDamage
//

void kexPuppet::OnDamage(kexActor *instigator)
{
    if(playerFlags & PF_DEAD)
    {
        return;
    }

    if(health <= 0)
    {
        if(playerFlags & PF_GOD)
        {
            // set to 1 just so enemies stay active
            health = 1;
            return;
        }

        kexGame::cLocal->PlayLoop()->DamageFlash();
        playerFlags |= PF_DEAD;

        if(flags & AF_INWATER && !(playerFlags & PF_ABOVESURFACE))
        {
            kexGame::cLocal->PlayLoop()->DamageFlash();
            PlaySound("sounds/pdrown.wav");
        }
        else if(instigator)
        {
            kexGame::cLocal->PlayLoop()->DamageFlash();
            PlaySound("sounds/pdeath03.wav");
        }
        else if(playerFlags & PF_ELECTROCUTE)
        {
            kexGame::cLocal->PlayLoop()->ElectrocuteFlash();
            PlaySound("sounds/pdeath01.wav");
        }

        owner->HoldsterWeapon();
        return;
    }

    if(instigator && instigator->InstanceOf(&kexProjectile::info) &&
        static_cast<kexProjectile*>(instigator)->ProjectileFlags() & PF_STUNTARGET)
    {
        kexGame::cLocal->PlayLoop()->DamageFlash();
        PlaySound("sounds/pshock.wav");

        playerFlags |= PF_STUNNED;
        owner->LockTime() = 60;
        owner->ShakeTime() = 30;
    }

    if(playerFlags & PF_ELECTROCUTE)
    {
        playerFlags &= ~PF_ELECTROCUTE;

        owner->LockTime() = 60;
        owner->ShakeTime() = 30;
        velocity.Clear();
        movement.Clear();

        kexGame::cLocal->PlayLoop()->ElectrocuteFlash();
        PlaySound("sounds/pelectrocute.wav");
        return;
    }

    kexGame::cLocal->PlayLoop()->DamageFlash();

    if(lavaTicks > 0 || slimeTicks > 0)
    {
        PlaySound("sounds/psizzle.wav");
        return;
    }

    if(flags & AF_INWATER && !(playerFlags & PF_ABOVESURFACE))
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
// kexPuppet::CheckFallDamage
//

void kexPuppet::CheckFallDamage(void)
{
    float amt = velocity.z + 28.5f;

    if(amt >= 0)
    {
        return;
    }

    InflictDamage(NULL, (int)(-amt * 16.0f) * (owner->Ankahs()+1));

    if(playerFlags & PF_DEAD)
    {
        PlaySound("sounds/pxdeath.wav");
    }
}

//
// kexPuppet::SlimeDamage
//

void kexPuppet::SlimeDamage(void)
{
    if((int)velocity.z != 0)
    {
        slimeTicks = 0;
    }
    else
    {
        if(owner->Artifacts() & PA_ANKLETS)
        {
            if((++slimeTicks & 63) == 1)
            {
                InflictDamage(NULL, 5);
            }
        }
        else
        {
            if((++slimeTicks & 31) == 1)
            {
                InflictDamage(NULL, 375);
            }
        }

        if(playerFlags & PF_DEAD)
        {
            PlaySound("sounds/psizzle.wav");
            PlaySound("sounds/pdeath02.wav");
        }
    }
}

//
// kexPuppet::LavaDamage
//

void kexPuppet::LavaDamage(mapFace_t *face)
{
    if((int)velocity.z != 0 && kexMath::Fabs(face->plane.c) > 0.5f)
    {
        lavaTicks = 0;
    }
    else if((++lavaTicks & 31) == 1)
    {
        int damage = (owner->Artifacts() & PA_ANKLETS) ? 50 : health;
        InflictDamage(NULL, damage);

        if(playerFlags & PF_DEAD)
        {
            PlaySound("sounds/psizzle.wav");
            PlaySound("sounds/pdeath02.wav");
        }
    }
}

//
// kexPuppet::Jump
//

void kexPuppet::Jump(kexPlayerCmd *cmd)
{
    if(cmd->Buttons() & BC_JUMP)
    {
        if(!(playerFlags & PF_USERJUMPED))
        {
            if(!(playerFlags & PF_JUMPING))
            {
                if(origin.z - floorHeight >= 1 &&
                    !(owner->Abilities() & PAB_VULTURE))
                {
                    return;
                }

                // let the actor object know that
                // it is now jumping
                playerFlags |= (PF_JUMPING|PF_JUMPWASHELD);

                if(owner->Abilities() & PAB_VULTURE)
                {
                    PlaySound("sounds/fly.wav");
                }
                else
                {
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
            }

            // handle longer jumps if holding down
            // the jump key/button
            if(velocity.z > 0 || jumpTicks == 0)
            {
                if(owner->Artifacts() & PA_SANDALS)
                {
                    velocity.z = PMOVE_JUMP_BOOST;
                    velocity.z -= (0.34f * (float)jumpTicks);
                }
                else
                {
                    velocity.z = PMOVE_SPEED_JUMP;
                    velocity.z -= (0.325f * (float)jumpTicks);
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
        playerFlags &= ~PF_JUMPWASHELD;

        if(owner->Abilities() & PAB_VULTURE)
        {
            playerFlags &= ~(PF_USERJUMPED|PF_JUMPING);
            jumpTicks = 0;
            return;
        }

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
        if(!(playerFlags & PF_JUMPWASHELD) && velocity.z <= 0 && cmd->Buttons() & BC_JUMP &&
            owner->Artifacts() & (PA_SHAWL|PA_FEATHER))
        {
            if(owner->Artifacts() & PA_FEATHER)
            {
                velocity.z = 0;
                playerFlags |= PF_FLOATING;
            }
            else if(owner->Artifacts() & PA_SHAWL)
            {
                velocity.z = -4;
            }
        }
        else
        {
            playerFlags &= ~PF_FLOATING;
            velocity.z -= gravity;
        }
    }
    else
    {
        playerFlags &= ~PF_FLOATING;

        if(sector->floorFace->flags & FF_LAVA)
        {
            if(origin.z - kexGame::cLocal->CModel()->GetFloorHeight(origin, sector) <= 0)
            {
                LavaDamage(sector->floorFace);
            }
        }
        else
        {
            lavaTicks = 0;
        }

        if(sector->floorFace->flags & FF_SLIME)
        {
            if(origin.z - kexGame::cLocal->CModel()->GetFloorHeight(origin, sector) <= 0)
            {
                SlimeDamage();
            }
        }
        else
        {
            slimeTicks = 0;
        }
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

        CheckFallDamage();

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
}

//
// kexPuppet::TryClimbOutOfWater
//

void kexPuppet::TryClimbOutOfWater(void)
{
    float vx, vy, vz;
    sectorList_t *sectorList;
    mapSector_t *startSector;

    // must be swimming along the water surface and must actually
    // be in water to begin with
    if( !(playerFlags & PF_ABOVESURFACE) ||
        !(flags & AF_INWATER) ||
        !(sector->ceilingFace->flags & FF_WATER) ||
        sector->ceilingFace->sector <= -1)
    {
        return;
    }

    vx = kexMath::Fabs(velocity.x);
    vy = kexMath::Fabs(velocity.y);
    vz = kexMath::Fabs(velocity.z);

    // only attempt to climb out of water if xy velocity is dominant
    if(vz >= vx && vz >= vy)
    {
        return;
    }

    // make absolutely sure we're in the right sector
    startSector = &kexGame::cWorld->Sectors()[sector->ceilingFace->sector];

    if(!(startSector->floorFace->flags & FF_WATER))
    {
        // if ceilingface is flagged as water, then we know we're underwater
        return;
    }

    if((float)startSector->floorHeight - floorHeight < height)
    {
        // we're in shallow water. don't bother trying to climb out
        return;
    }

    // determine the vector from our current position to where we're
    // expecting to climb out of
    kexVec3 start = origin;
    start.z = (float)startSector->floorHeight + stepHeight;

    kexVec3 forward;
    kexVec3::ToAxis(&forward, 0, 0, yaw, 0, 0);

    kexVec3 end = start + (forward * (radius*1.05f));

    // scan surrounding sectors
    sectorList = kexGame::cWorld->FloodFill(start, startSector, radius*1.05f);
    for(unsigned int j = 0; j < sectorList->CurrentLength(); ++j)
    {
        if((*sectorList)[j] == sector)
        {
            continue;
        }

        // see if we can climb out
        for(int i = (*sectorList)[j]->faceStart; i <= (*sectorList)[j]->faceEnd; ++i)
        {
            mapFace_t *face = &kexGame::cWorld->Faces()[i];
            float fHeight, amount;
            mapSector_t *s;

            if(face->sector <= -1)
            {
                // we can't climb solid walls
                continue;
            }

            // must be moving towards this face
            if(face->plane.Normal().Dot(velocity) >= 0)
            {
                continue;
            }

            // our movement vector must be crossing this face
            if( face->plane.Distance(end) >= 0 ||
                face->plane.Distance(start) < 0)
            {
                continue;
            }

            s = &kexGame::cWorld->Sectors()[face->sector];

            if(s->flags & SF_WATER || s->floorFace->flags & FF_WATER)
            {
                // this sector is not on land
                continue;
            }

            if(s->floorFace->plane.c <= 0.5f)
            {
                // this floor is too steep
                continue;
            }

            fHeight = kexGame::cLocal->CModel()->GetFloorHeight(end, s);

            if(end.z - fHeight < 0)
            {
                // to high for us to climb out
                continue;
            }

            if(!kexGame::cLocal->CModel()->PointWithinSectorEdges(end, s))
            {
                // end point is not actually inside the sector
                continue;
            }

            // we've found land and we can climb out now
            owner->StepViewZ() = origin.z - fHeight;
            amount = s->floorFace->plane.c * (4*radius);

            // nudge out and attempt to climb over.
            // for slopes, nudge our movement forward a bit depending on steepness
            movement.x += (forward.x * (amount * kexMath::Fabs(s->floorFace->plane.a)));
            movement.y += (forward.y * (amount * kexMath::Fabs(s->floorFace->plane.b)));
            origin.z += (fHeight - origin.z);

            forward = origin;
            forward.x = origin.x + movement.x;
            forward.y = origin.y + movement.y;

            floorHeight = kexGame::cLocal->CModel()->GetFloorHeight(forward, s);

            // update sector
            SetSector(s);
            return;
        }
    }
}

//
// kexPuppet::WaterMove
//

void kexPuppet::WaterMove(kexPlayerCmd *cmd)
{
    kexVec3 forward, right;
    mapSector_t *oldSector;
    float swimSpeed;
    
    yaw += cmd->Angles()[0];
    pitch += cmd->Angles()[1];
    roll -= (cmd->Angles()[0] * 0.25f);
    
    kexMath::Clamp(pitch.an, kexMath::Deg2Rad(-90), kexMath::Deg2Rad(90));
    kexMath::Clamp(roll.an, -0.1f, 0.1f);

    kexVec3::ToAxis(&forward, NULL, NULL, yaw, pitch, 0);
    kexVec3::ToAxis(NULL, NULL, &right, yaw, 0, 0);

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

    if(owner->Abilities() & PAB_DOLPHIN)
    {
        swimSpeed = PMOVE_WATER_SPEED * 2;
    }
    else
    {
        swimSpeed = PMOVE_WATER_SPEED;
    }

    if(cmd->Buttons() & BC_FORWARD)
    {
        velocity += (forward * swimSpeed);
    }
    else if(cmd->Movement()[0] > 0)
    {
        velocity += (forward * cmd->Movement()[0]) * swimSpeed;
    }

    if(cmd->Buttons() & BC_BACKWARD)
    {
        velocity -= (forward * (swimSpeed*0.5f));
    }
    else if(cmd->Movement()[0] < 0)
    {
        velocity += (forward * cmd->Movement()[0]) * (swimSpeed*0.5f);
    }

    if(!(cmd->Buttons() & (BC_STRAFELEFT|BC_STRAFERIGHT)))
    {
        if(cmd->Movement()[1] != 0)
        {
            velocity.x += (right.x * cmd->Movement()[1]) * (swimSpeed*0.5f);
            velocity.y += (right.y * cmd->Movement()[1]) * (swimSpeed*0.5f);
        }
    }

    if(cmd->Buttons() & BC_STRAFELEFT)
    {
        velocity.x -= right.x * (swimSpeed*0.5f);
        velocity.y -= right.y * (swimSpeed*0.5f);
    }

    if(cmd->Buttons() & BC_STRAFERIGHT)
    {
        velocity.x += right.x * (swimSpeed*0.5f);
        velocity.y += right.y * (swimSpeed*0.5f);
    }

    if(sector->ceilingFace->flags & FF_WATER)
    {
        float waterZ = kexGame::cLocal->CModel()->GetCeilingHeight(origin, sector);
        float waterheight = waterZ - origin.z;

        if(waterheight < ((height + (owner->ViewZ() * 0.5f)) * 0.5f))
        {
            float t = (waterheight < owner->ViewZ()) ? 0.035f : 0.075f;
            origin.z = ((waterZ - owner->ViewZ()) - origin.z) * t + origin.z;
            playerFlags |= PF_ABOVESURFACE;
        }
        else
        {
            playerFlags &= ~PF_ABOVESURFACE;
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

    TryClimbOutOfWater();
    
    if(!kexGame::cLocal->CModel()->MoveActor(this))
    {
        velocity.Clear();
        movement.Clear();
    }

    if(!(sector->flags & SF_WATER))
    {
        this->flags &= ~AF_INWATER;
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
// kexPuppet::DeadMove
//

void kexPuppet::DeadMove(kexPlayerCmd *cmd)
{
    pitch -= kexMath::Deg2Rad(4);
    roll -= kexMath::Deg2Rad(4);

    kexMath::Clamp(pitch.an, kexMath::Deg2Rad(-90), kexMath::Deg2Rad(90));
    kexMath::Clamp(roll.an, kexMath::Deg2Rad(-45), kexMath::Deg2Rad(45));

    velocity.x *= friction;
    velocity.y *= friction;

    if(origin.z > floorHeight)
    {
        velocity.z -= gravity;
    }

    // bump floor
    if(origin.z + velocity.z <= floorHeight)
    {
        origin.z = floorHeight;
        velocity.z = 0;
    }

    movement = velocity;
    
    if(!kexGame::cLocal->CModel()->MoveActor(this))
    {
        velocity.Clear();
        movement.Clear();
    }
}

//
// kexPuppet::ScheduleWarpForNextMap
//

void kexPuppet::ScheduleWarpForNextMap(const kexVec3 &destination)
{
    kexPuppet::bScheduleNextMapWarp = true;
    mapDestinationPosition = destination;
}

//
// kexPuppet::Tick
//

void kexPuppet::Tick(void)
{
    kexPlayerCmd *cmd = &owner->Cmd();

    if(playerFlags & PF_DEAD)
    {
        DeadMove(cmd);
        gameTicks++;
        return;
    }
    
    roll = (0 - roll) * 0.35f + roll;

    if(owner->LockTime() > 0)
    {
        if(playerFlags & PF_STUNNED && origin.z > floorHeight)
        {
            // nothing
        }
        else
        {
            return;
        }
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

    kexGame::cWorld->EnterSectorSpecial(this, sector);
    gameTicks++;
}

//
// kexPuppet::Spawn
//

void kexPuppet::Spawn(void)
{
    float r, h;

    owner = kexGame::cLocal->Player();
    kexGame::cLocal->Player()->SetActor(this);

    radius      = 95.25f;
    height      = 160;
    stepHeight  = 48;
    health      = 200;
    friction    = 0.9375f;
    flags       = (AF_SOLID|AF_SHOOTABLE);
    lavaTicks   = 0;
    slimeTicks  = 0;
    playerFlags = PF_ABOVESURFACE;

    r = radius * 0.5f;
    h = height;
    
    bounds.min.Set(-r, -r, 0);
    bounds.max.Set(r, r, h);
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
    this->lockTime = 0;
    this->shakeTime = 0;
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
    ankahs = 0;
    ankahFlags = 0;
    actor = NULL;

    cmd.Reset();

    health = maxHealth;
    bob = 0;
    bobTime = 0;
    bobSpeed = 0;
    landTime = 0;
    stepViewZ = 0;
    viewZ = 64.0f;
    airSupply = 64;
    airSupplyTime = 0;
    lockTime = 0;
    shakeTime = 0;
    shakeVector.Clear();

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
    questItems = 0;
    teamDolls = 0;
    abilities = 0;
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
    viewZ = 64.0f;
    airSupply = 64;
    airSupplyTime = 0;
    lockTime = 0;
    shakeTime = 0;
    shakeVector.Clear();

    weapon.ChangeAnim(WS_RAISE);
    
    if(actor)
    {
        actor->Health() = health;
        
        if(kexPuppet::bScheduleNextMapWarp == true)
        {
            kexPuppet::bScheduleNextMapWarp = false;
            actor->Origin() = kexPuppet::mapDestinationPosition;
            actor->FindSector(actor->Origin());
        }
        
        kexGame::cWorld->EnterSectorSpecial(actor, actor->Sector());
    }
}

//
// kexPlayer::UpdateViewBob
//

void kexPlayer::UpdateViewBob(void)
{
    if(actor->Origin().z > actor->FloorHeight() &&
        !(actor->Flags() & AF_INWATER) && !(actor->PlayerFlags() & PF_FLOATING))
    {
        bob = 0;
        bobTime = 0;
        bobSpeed = 0;
    }
    else
    {
        if( cmd.Buttons() & (BC_FORWARD|BC_BACKWARD|BC_STRAFELEFT|BC_STRAFERIGHT) ||
            actor->Flags() & AF_INWATER || actor->PlayerFlags() & PF_FLOATING ||
            (cmd.Movement()[0] != 0 || cmd.Movement()[1] != 0))
        {
            float speed;
            
            if(actor->Flags() & AF_INWATER || actor->PlayerFlags() & PF_FLOATING)
            {
                speed = 0.25f;
            }
            else
            {
                speed = 1;
            }

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

    if(!(actor->Flags() & AF_INWATER) || actor->PlayerFlags() & PF_ABOVESURFACE)
    {
        if(actor->PlayerFlags() & PF_NEEDTOGASP)
        {
            if(abilities & PAB_DOLPHIN)
            {
                if(kexRand::Byte() & 1)
                {
                    actor->PlaySound("sounds/dolphin01.wav");
                }
                else
                {
                    actor->PlaySound("sounds/dolphin02.wav");
                }

                actor->Velocity().z += PMOVE_SPEED_JUMP;
            }
            else
            {
                actor->PlaySound("sounds/pwgasp.wav");
            }

            actor->PlayerFlags() &= ~PF_NEEDTOGASP;
        }

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

    if(abilities & PAB_DOLPHIN)
    {
        actor->PlayerFlags() |= PF_NEEDTOGASP;
    }
    
    if(airSupplyTime >= maxAirSupplyTime)
    {
        if(airSupply > 0)
        {
            airSupplyTime = 0;
            airSupply -= 2;

            if(!(actor->PlayerFlags() & PF_NEEDTOGASP) &&  airSupply <= 48)
            {
                actor->PlayerFlags() |= PF_NEEDTOGASP;
            }

            if(airSupply < 0)
            {
                airSupply = 0;
            }
            else if(artifacts & PA_MASK && (airSupply & 3) == 2)
            {
                if(!(abilities & PAB_DOLPHIN))
                {
                    actor->PlaySound("sounds/pwbreathe.wav");
                }
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
            kexGame::cWorld->UseWallSpecial(this, useFace);
        }
    }
}

//
// kexPlayer::AutoAim
//

kexActor *kexPlayer::AutoAim(const kexVec3 &start, kexAngle &yaw, kexAngle &pitch,
                             const float dist, const float aimYaw, const float aimPitch)
{
    sectorList_t *sectorList;
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
    sectorList = kexGame::cWorld->FloodFill(start, actor->Sector(), dist);

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

            if(kexMath::Fabs(yaw.Diff(aYaw)) > aimYaw) continue;
            if(kexMath::Fabs(pitch.Diff(aPitch)) > aimPitch) continue;

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
    int max = maxHealth * (ankahs+1);

    if(actor->Health() >= max)
    {
        return false;
    }

    actor->Health() += amount;

    if(actor->Health() > max)
    {
        actor->Health() = max;
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
// kexPlayer::IncreaseMaxHealth
//

bool kexPlayer::IncreaseMaxHealth(const int bits)
{
    ankahs++;

    if(ankahs > 8 || ankahFlags & bits)
    {
        ankahs = 8;
        return false;
    }

    actor->Health() = maxHealth * (ankahs+1);
    ankahFlags |= bits;
    return true;
}

//
// kexPlayer::HasAmmo
//

bool kexPlayer::HasAmmo(const int weaponID)
{
    int max = kexGame::cLocal->WeaponInfo(weaponID)->maxAmmo;

    if(max <= 0)
    {
        return true;
    }

    return (GetAmmo(weaponID) > 0 && max > 0);
}

//
// kexPlayer::CycleNextWeapon
//

void kexPlayer::CycleNextWeapon(const bool bCheckAmmo)
{
    int setWeapon = pendingWeapon;

    do
    {
        if(++setWeapon >= NUMPLAYERWEAPONS)
        {
            setWeapon = PW_MACHETE;
        }

        if(bCheckAmmo && !HasAmmo(setWeapon))
        {
            continue;
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

void kexPlayer::CyclePrevWeapon(const bool bCheckAmmo)
{
    int setWeapon = pendingWeapon;

    do
    {
        if(--setWeapon < 0)
        {
            setWeapon = NUMPLAYERWEAPONS-1;
        }

        if(bCheckAmmo && !HasAmmo(setWeapon))
        {
            continue;
        }

        if(weapons[setWeapon] == true)
        {
            pendingWeapon = static_cast<playerWeapons_t>(setWeapon);
            break;
        }

    } while(setWeapon != currentWeapon);
}

//
// kexPlayer::HoldsterWeapon
//

void kexPlayer::HoldsterWeapon(void)
{
    weapon.ChangeAnim(WS_HOLDSTER);
}

//
// kexPlayer::Tick
//

void kexPlayer::Tick(void)
{
    if(actor->PlayerFlags() & PF_DEAD)
    {
        viewZ = (2 - viewZ) * 0.35f + viewZ;
        landTime = 0;
        weapon.Update();
        return;
    }

    UpdateAirSupply();

    if(shakeTime > 0)
    {
        shakeVector.x = kexRand::CFloat() * 32;
        shakeVector.y = kexRand::CFloat() * 32;

        if(--shakeTime <= 0)
        {
            shakeVector.Clear();
        }
    }

    if(lockTime > 0)
    {
        if(actor->PlayerFlags() & PF_STUNNED && actor->Origin().z > actor->FloorHeight())
        {
            lockTime--;
        }
        else
        {
            if(weapon.State() == WS_HOLDSTER)
            {
                weapon.Update();
            }

            lockTime--;
            return;
        }
    }

    if(lockTime <= 0 && actor->PlayerFlags() & PF_STUNNED)
    {
        actor->PlayerFlags() &= ~PF_STUNNED;
    }

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

    UpdateViewBob();
    weapon.Update();
}
