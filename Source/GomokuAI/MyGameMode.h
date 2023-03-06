// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MyGameMode.generated.h"

class ABoard;

/**
 * 
 */
UCLASS()
class GOMOKUAI_API AMyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	
	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditAnywhere, Category="Class Types")
	TSubclassOf<UUserWidget> MainWidgetClass;

	UPROPERTY(VisibleInstanceOnly, Category="Runtime")
	class UMainUserWidget* MainWidget;
	
private:
	
	void initialize2DRenderingSettings() const; 
};
