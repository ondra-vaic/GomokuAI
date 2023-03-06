// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "MainUserWidget.h"
#include "Kismet/GameplayStatics.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bEnableClickEvents = true;
	bShowMouseCursor = true;
	InputComponent->BindAction("MouseLeftClicked", IE_Pressed, this, &AMyPlayerController::OnClick);
}


void AMyPlayerController::ShowEndUI(bool playerWin) const
{
	MainUserWidget->SetVisibility(ESlateVisibility::Visible);
	MainUserWidget->SetWinText(playerWin);
}

void AMyPlayerController::OnClick()
{
	if(MainUserWidget->IsVisible()) return;
	
	UGameViewportClient* viewportClient = GetWorld()->GetFirstLocalPlayerFromController()->ViewportClient;
	
	FVector2D mousePosition;
	viewportClient->GetMousePosition(mousePosition);

	FVector2D viewportSize;
	viewportClient->GetViewportSize(viewportSize);

	float x;
	float y;
	
	if(viewportSize.X > viewportSize.Y)
	{
		y = 1 - mousePosition.Y / viewportSize.Y;
		x = (mousePosition.X - (viewportSize.X - viewportSize.Y) * 0.5f) / viewportSize.Y;
		x = FMath::Min<float>(x, 0.99);
	}
	else
	{
		x = mousePosition.X / viewportSize.X;
		y = (mousePosition.Y - (viewportSize.Y - viewportSize.X) * 0.5f) / viewportSize.X;
		y = 1 - FMath::Min<float>(y, 0.99);
	}
	
	OnMouseClicked.Execute(x, y);
}
