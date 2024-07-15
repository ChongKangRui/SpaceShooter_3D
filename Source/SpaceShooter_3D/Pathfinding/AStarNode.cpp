// Fill out your copyright notice in the Description page of Project Settings.


#include "AStarNode.h"

// Sets default values
UAStarNode::UAStarNode()
{

}

bool UAStarNode::CheckIsNodeOccupied() const
{
    return Status == ENodeStatus::ShipOccupied;
}

bool UAStarNode::CheckIsNodeInvalid() const
{
    return Status == ENodeStatus::InvalidPath;
}

