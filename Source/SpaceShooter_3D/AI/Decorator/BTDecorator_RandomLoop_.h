// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_Loop.h"
#include "BTDecorator_RandomLoop_.generated.h"

/**
 * 
 */
UCLASS()
class SPACESHOOTER_3D_API UBTDecorator_RandomLoop_ : public UBTDecorator_Loop
{
	GENERATED_BODY()
public:
    UBTDecorator_RandomLoop_();

    // Properties for min and max loop counts
    UPROPERTY(EditAnywhere, Category = "Random Loop")
    int32 MinLoopCount = 1;

    UPROPERTY(EditAnywhere, Category = "Random Loop")
    int32 MaxLoopCount = 3;


protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;

};
