// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MyAnimInstance.generated.h"

/**
 * 
 */

DECLARE_MULTICAST_DELEGATE(FOnAttackHit);

UCLASS()
class MMORPG2_API UMyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	UMyAnimInstance();
		virtual void NativeUpdateAnimation (float DeltaSeconds) override;
		void PlayAttackMontage();

		UFUNCTION()
		void AnimNotify_AttackHit();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Move, Meta = (AllowPrivateAccess = True))
	float IsRunning;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Move, Meta = (AllowPrivateAccess = True))
	bool IsFalling;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Move, Meta = (AllowPrivateAccess = True))
	UAnimMontage* AttackMontage;

public:
	FOnAttackHit OnAttackHit;

};