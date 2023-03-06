// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainUserWidget.generated.h"

class ABoard;
/**
 * 
 */
UCLASS()
class GOMOKUAI_API UMainUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetWinText(bool playerWin) const;
	
protected:
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UTextBlock* WinText;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UButton* StartButton;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class USlider* DifficultySlider; 

	virtual void NativeConstruct() override;

	UFUNCTION()
	void StartButtonClicked();

private:

	UPROPERTY()
	ABoard* Board;
};
