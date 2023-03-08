// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <future>

#include "CoreMinimal.h"
#include "BoardState.h"
#include "PaperSpriteActor.h"
#include <unordered_map>

#include "Solver.h"
#include "Board.generated.h"

class Worker;
class Solver;
class AMyPlayerController;


UCLASS()
class GOMOKUAI_API ABoard : public APaperSpriteActor
{
	GENERATED_BODY()

public:
	
	ABoard();
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	void Initialize(int32 DifficultyLevel, bool PlayFirstMove);
	
public:

	UPROPERTY(EditDefaultsOnly)
	TMap<EStoneType, TSubclassOf<APaperSpriteActor>> Stones;

	UPROPERTY(EditAnywhere)
	ACameraActor* Camera;

	UPROPERTY(EditDefaultsOnly)
	int32 BoardSize;

	UPROPERTY(EditDefaultsOnly)
	int32 SpriteSize;

	UPROPERTY(EditDefaultsOnly)
	int32 SpriteMargin;

public:
	
	int32 TimeToSolve;
	
	int32 IterationsToSolve;
	
public:
	
	UFUNCTION()
	void OnMouseClicked(float viewPortX, float viewPortY);

	static int32 FlattenIndex(int32 x, int32 y, int32 boardSize);
	
private:

	int32 ClickPositionToIndex(float x, float y) const;

	void PlaceStone(int32 index, EStoneType stoneType);

	void Solve(int32 fromAction);

	int32 TryGetSolution();

	bool IsWinningMove(int32 move, EStoneType stoneType) const;

	static constexpr int32 Difficulties[] = {20, 75, 300, 500, 650, 800, 1000, 1500, 2000, 3500, 5000};
	
private:

	std::vector<EStoneType> Board;

	UPROPERTY()
	TArray<APaperSpriteActor*> BoardActors;

	UPROPERTY()
	AMyPlayerController* PlayerController;

	UPROPERTY()
	APaperSpriteActor* LastPlacedStone;
	
	EStoneType TurnStone;

	EStoneType PlayersStone;
	
	EStoneType AIsStone;

	float CameraWidth;

	std::vector<Solver*> GomokuSolvers;
	
	std::vector<Worker*> Workers;
	
	// std::vector<Solver*> GomokuSolvers;
	
	std::vector<std::shared_future<std::unordered_map<int, Evaluation>>> Solutions;
	
	int32 NumFullSpaces;
	
};
