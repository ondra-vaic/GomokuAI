// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */

class UMainUserWidget;
DECLARE_DELEGATE_TwoParams(FMouseClicked, float, float);

UCLASS()
class GOMOKUAI_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	
	virtual void BeginPlay() override;

	void ShowEndUI(bool playerWin) const;
	
public:

	FMouseClicked OnMouseClicked;

	UPROPERTY()
	UMainUserWidget* MainUserWidget;
	
private:
	
	void OnClick();
	
};
