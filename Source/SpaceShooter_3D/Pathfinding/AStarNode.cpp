// Fill out your copyright notice in the Description page of Project Settings.


#include "AStarNode.h"
#include "Pathfinding/AStarPathGrid.h"

// Sets default values
UAStarNode::UAStarNode()
{

}

bool UAStarNode::CheckIsNodeOccupied() const
{
//    return Status == ENodeStatus::ShipOccupied;
	return false;
}

bool UAStarNode::CheckIsNodeInvalid() const
{
   // return Status == ENodeStatus::InvalidPath;
	return false;
}

