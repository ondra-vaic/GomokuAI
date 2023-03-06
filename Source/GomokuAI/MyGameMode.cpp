// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameMode.h"
#include "MainUserWidget.h"
#include "MyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AMyGameMode::BeginPlay() 
{
	initialize2DRenderingSettings();

	MainWidget = Cast<UMainUserWidget>(CreateWidget(GetWorld(), MainWidgetClass));
	MainWidget->AddToViewport();
	
	AMyPlayerController* playerController = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
	playerController->MainUserWidget = MainWidget;
}

void AMyGameMode::initialize2DRenderingSettings() const
{
	// The command being executed.
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	
	FString Command = "ShowFlag.Tonemapper 0";
	PlayerController->ConsoleCommand(*Command);
	
	// The below commands are for the shipping
	// build.
	Command = "r.TonemapperGamma 0";
	PlayerController->ConsoleCommand(*Command);
	
	Command = "r.TonemapperFilm 0";
	PlayerController->ConsoleCommand(*Command);
	
	Command = "r.Tonemapper.Sharpen 0";
	PlayerController->ConsoleCommand(*Command);
	
	Command = "r.Tonemapper.GrainQuantization 0";
	PlayerController->ConsoleCommand(*Command);
	
	Command = "r.Tonemapper.Quality 0";
	PlayerController->ConsoleCommand(*Command);
	
	// The command being executed.
	Command = "ShowFlag.EyeAdaptation 0";
	PlayerController->ConsoleCommand(*Command);
	
	// The below commands are for the shipping
	// build.
	Command = "r.EyeAdaptation.ExponentialTransitionDistance 0";
	PlayerController->ConsoleCommand(*Command);
	
	Command = "r.EyeAdaptationQuality 0";
	PlayerController->ConsoleCommand(*Command);

	Command = "r.setRes 800x800w";
	PlayerController->ConsoleCommand(*Command);
	
	UGameViewportClient* viewportClient = GetWorld()->GetGameViewport();
	viewportClient->ViewModeIndex = EViewModeIndex::VMI_Unlit;
}
