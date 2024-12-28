// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorator/BTDecorator_RandomLoop_.h"

UBTDecorator_RandomLoop_::UBTDecorator_RandomLoop_()
{
	NodeName = "RandomLoop";
}

void UBTDecorator_RandomLoop_::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	NumLoops = FMath::RandRange(MinLoopCount, MaxLoopCount);

	Super::OnNodeActivation(SearchData);
}
