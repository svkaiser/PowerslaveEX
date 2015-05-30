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

#ifndef __SDNODES_H__
#define __SDNODES_H__

typedef enum
{
    NODE_FRONT  = 0,
    NODE_BACK,
    NODE_SIDES
} nodeSide_t;

//-----------------------------------------------------------------------------
//
// node object
//
//-----------------------------------------------------------------------------

template<class type>
class kexSDNodeObj
{
public:
    kexSDNodeObj(void);

    kexBBox             bounds;
    int                 axis;
    float               dist;
    kexPlane            plane;
    kexLinklist<type>   objects;
    int                 nodeNum;
    float               radius;
    kexSDNodeObj        *children[NODE_SIDES];
};

//
// kexSDNodeObj::kexSDNodeObj
//
template<class type>
kexSDNodeObj<type>::kexSDNodeObj(void)
{
    axis = 0;
    dist = 0;
    nodeNum = -1;
    radius = 0;
    children[0] = NULL;
    children[1] = NULL;
}

//-----------------------------------------------------------------------------
//
// spatial subdivision nodes
//
//-----------------------------------------------------------------------------

template<class type>
class kexSDNode
{
public:
    kexSDNode(void);
    ~kexSDNode(void);

    void                Init(const unsigned int depth);
    void                AddBoxToRoot(const kexBBox &box);
    void                Reset(void);
    void                Destroy(void);
    void                BuildNodes(void);
    int                 PointInNode(const kexVec3 &point, const float min = 0);

    kexSDNodeObj<type>  *nodes;
    unsigned int        numNodes;

private:
    kexSDNodeObj<type>  *AddNode(int depth, kexBBox &box);

    unsigned int        maxDepth;
    unsigned int        maxNodes;
    kexVec3             rootBoundMin;
    kexVec3             rootBoundMax;
};

//
// kexSDNode::kexSDNode
//
template<class type>
kexSDNode<type>::kexSDNode(void)
{
    rootBoundMax[0] = -kexMath::infinity;
    rootBoundMax[1] = -kexMath::infinity;
    rootBoundMax[2] = -kexMath::infinity;
    rootBoundMin[0] =  kexMath::infinity;
    rootBoundMin[1] =  kexMath::infinity;
    rootBoundMin[2] =  kexMath::infinity;

    nodes = NULL;
    numNodes = 0;
    maxDepth = 8;
    maxNodes = 512;
}

//
// kexSDNode::~kexSDNode
//
template<class type>
kexSDNode<type>::~kexSDNode(void)
{
    Destroy();
}

//
// kexSDNode::Init
//
template<class type>
void kexSDNode<type>::Init(const unsigned int depth)
{
    maxDepth = depth;
    maxNodes = (2 << depth);
    numNodes = 0;

    nodes = new kexSDNodeObj<type>[maxNodes];
    Reset();
}

//
// kexSDNode::Destroy
//
template<class type>
void kexSDNode<type>::Destroy(void)
{
    if(nodes != NULL)
    {
        delete[] nodes;
        nodes = NULL;
    }
}

//
// kexSDNode::AddBoxToRoot
//
template<class type>
void kexSDNode<type>::AddBoxToRoot(const kexBBox &box)
{
    if(box.min[0] < rootBoundMin[0]) { rootBoundMin[0] = box.min[0]; }
    if(box.min[1] < rootBoundMin[1]) { rootBoundMin[1] = box.min[1]; }
    if(box.min[2] < rootBoundMin[2]) { rootBoundMin[2] = box.min[2]; }
    if(box.max[0] > rootBoundMax[0]) { rootBoundMax[0] = box.max[0]; }
    if(box.max[1] > rootBoundMax[1]) { rootBoundMax[1] = box.max[1]; }
    if(box.max[2] > rootBoundMax[2]) { rootBoundMax[2] = box.max[2]; }
}

//
// kexSDNode::Reset
//
template<class type>
void kexSDNode<type>::Reset(void)
{
    rootBoundMax[0] = -kexMath::infinity;
    rootBoundMax[1] = -kexMath::infinity;
    rootBoundMax[2] = -kexMath::infinity;
    rootBoundMin[0] =  kexMath::infinity;
    rootBoundMin[1] =  kexMath::infinity;
    rootBoundMin[2] =  kexMath::infinity;

    numNodes = 0;

    for(unsigned int i = 0; i < maxNodes; i++)
    {
        nodes[i].axis = -1;
        nodes[i].dist = 0;
        nodes[i].bounds.Clear();
        nodes[i].children[NODE_FRONT] = NULL;
        nodes[i].children[NODE_BACK] = NULL;
        nodes[i].objects.Clear();
    }
}

//
// kexSDNode::BuildNodes
//
template<class type>
void kexSDNode<type>::BuildNodes(void)
{
    kexBBox box;

    box.min = rootBoundMin;
    box.max = rootBoundMax;

    AddNode(0, box);
}

//
// kexSDNode::AddNode
//
template<class type>
kexSDNodeObj<type> *kexSDNode<type>::AddNode(int depth, kexBBox &box)
{
    kexSDNodeObj<type> *node = &nodes[numNodes++];

    node->bounds = box;
    node->nodeNum = numNodes-1;
    node->radius = node->bounds.Radius();

    if(depth == maxDepth || ((box.max - box.min) * 0.5f).Unit() <= maxNodes)
    {
        node->axis = -1;
        node->children[0] = NULL;
        node->children[1] = NULL;
    }
    else
    {
        kexVec3 size = box.max - box.min;
        kexBBox box1, box2;

        if(size.x > size.y)
        {
            node->axis = 0;
        }
        else
        {
            node->axis = 1;
        }

        node->dist = (box.max[node->axis] + box.min[node->axis]) * 0.5f;

        box1 = box;
        box2 = box;

        node->plane = node->axis == 1 ? kexPlane(0, 1, 0, node->dist) :
                                        kexPlane(1, 0, 0, node->dist);

        box1.max[node->axis] = node->dist;
        box2.min[node->axis] = node->dist;

        node->children[0] = AddNode(depth+1, box2);
        node->children[1] = AddNode(depth+1, box1);
    }

    return node;
}

//
// kexSDNode::PointInNode
//
template<class type>
int kexSDNode<type>::PointInNode(const kexVec3 &point, const float min)
{
    kexSDNodeObj<type> *n = nodes;
    float d;

    while(1)
    {
        if(n->axis == -1)
        {
            break;
        }

        d = n->plane.Distance(point);

        if(d > min)
        {
            n = n->children[0];
        }
        else if(d < -min)
        {
            n = n->children[1];
        }
        else
        {
            break;
        }
    }

    return n->nodeNum;
}

//-----------------------------------------------------------------------------
//
// node reference
//
//-----------------------------------------------------------------------------

template<class type>
class kexSDNodeRef
{
public:

    void                Link(kexSDNode<type> &sdNode, kexBBox &box);
    void                UnLink(void);

    kexLinklist<type>   link;
    kexSDNodeObj<type>  *node;
};

//
// kexSDNodeRef::Link
//
template<class type>
void kexSDNodeRef<type>::Link(kexSDNode<type> &sdNode, kexBBox &box)
{
    kexSDNodeObj<type> *n = sdNode.nodes;

    UnLink();

    while(1)
    {
        if(n->axis == -1)
        {
            break;
        }
        if(box.min[n->axis] > n->dist)
        {
            n = n->children[0];
        }
        else if(box.max[n->axis] < n->dist)
        {
            n = n->children[1];
        }
        else
        {
            break;
        }
    }

    link.AddBefore(n->objects);
    node = n;
}

//
// kexSDNodeRef::UnLink
//
template<class type>
void kexSDNodeRef<type>::UnLink(void)
{
    link.Remove();
    node = NULL;
}

#endif
