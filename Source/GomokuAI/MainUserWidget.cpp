// Fill out your copyright notice in the Description page of Project Settings.

#include "MainUserWidget.h"

#include "Board.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UMainUserWidget::SetWinText(bool playerWin) const
{
	WinText->SetText(playerWin ? FText::FromString("You Win!") : FText::FromString("You Loose."));
}

void UMainUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TArray<AActor*> foundBoards;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoard::StaticClass(), foundBoards);
	Board = Cast<ABoard>(foundBoards[0]);
	
	StartButton->OnClicked.AddDynamic(this, &UMainUserWidget::StartButtonClicked);
}

void UMainUserWidget::StartButtonClicked()
{
	Board->Initialize(DifficultySlider->GetValue(), true);
	SetVisibility(ESlateVisibility::Hidden);
}
