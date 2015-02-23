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

#ifndef __SPRING_H__
#define __SPRING_H__

class kexSpring
{
public:
    kexSpring(void);
    ~kexSpring(void);
    
    void                    Update(void);
    void                    SetSpring(const kexVec3 &force);
    
    float                   &Constant(void) { return constant; }
    float                   &Damping(void) { return damping; }
    void                    SetOrigin(kexVec3 &vec) { origin = &vec; }
    kexVec3                 &InitialOrigin(void) { return initialOrigin; }
    kexArray<kexSpring*>    &NeighborSprings(void) { return neighborSprings; }
    
private:
    float                   constant;
    float                   damping;
    kexVec3                 *origin;
    kexVec3                 velocity;
    kexVec3                 offset;
    kexVec3                 initialOrigin;
    kexArray<kexSpring*>    neighborSprings;
};

#endif
