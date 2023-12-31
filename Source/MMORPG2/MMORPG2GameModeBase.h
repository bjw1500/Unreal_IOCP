// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


#include "GameFramework/GameModeBase.h"
#include "CoreMinimal.h"
#include "Network/GameManager.h"
#include "Network/NetworkSession.h"
#include "MMORPG2GameModeBase.generated.h"


UCLASS()
class MMORPG2_API AMMORPG2GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
		AMMORPG2GameModeBase();
		virtual void BeginPlay() override;
		virtual void Init();


public:

	UPROPERTY()
	TSubclassOf<UUserWidget> UIClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI");
	class UUserWidget* MainUI;
	
};
