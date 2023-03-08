#include "Board.h"

#include <future>

#include "MyPlayerController.h"
#include "PaperSpriteComponent.h"
#include "Solver.h"
#include "Worker.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"

ABoard::ABoard()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABoard::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	PlayerController = Cast<AMyPlayerController>(playerController);
	PlayerController->OnMouseClicked.BindUObject(this, &ABoard::OnMouseClicked);
	
	BoardActors.Init(nullptr, BoardSize * BoardSize);

	Initialize(3, false);
	
	const int32 boardWidth = BoardSize * (SpriteSize + SpriteMargin);
	CameraWidth = boardWidth + SpriteMargin;
	
	Camera->GetCameraComponent()->SetOrthoWidth(CameraWidth);
	Camera->SetActorLocation(FVector(boardWidth * .5f - (SpriteSize + SpriteMargin) * .5f, 10, boardWidth * .5f - (SpriteSize + SpriteMargin) * .5f));
}

void ABoard::Tick(float DeltaSeconds)
{
	if(TurnStone != PlayersStone)
	{
		const int32 move = TryGetSolution();
		if(move != -1)
		{
			PlaceStone(move, TurnStone);
			if(IsWinningMove(move, AIsStone))
			{
				LastPlacedStone->GetRenderComponent()->SetSpriteColor(FLinearColor(1, 0, 0));
				PlayerController->ShowEndUI(false);
			}
		}
	}
}

void ABoard::Initialize(int32 DifficultyLevel, bool PlayFirstMove)
{
	IterationsToSolve = Difficulties[DifficultyLevel];
	TimeToSolve = DifficultyLevel * 2;
	
	for (auto gomokuSolver : GomokuSolvers)
	{
		delete gomokuSolver;
	}

	GomokuSolvers.clear();
	Solutions.clear();
	
	Board = std::vector<EStoneType>(BoardSize * BoardSize, EStoneType::Empty);
	
	srand(unsigned(time(NULL)));
	bool playerStarts = rand() % 2 == 0;
	PlayersStone = playerStarts ? EStoneType::X : EStoneType::O;
	AIsStone = playerStarts ? EStoneType::O : EStoneType::X;
	
	if(!playerStarts && PlayFirstMove)
	{
		Board[FlattenIndex(BoardSize / 2, BoardSize / 2, BoardSize)] = EStoneType::X;
	}
	
	for (int32 y = 0; y < BoardSize; ++y)
	{
		for (int32 x = 0; x < BoardSize; ++x)
		{
			PlaceStone(FlattenIndex(x, y, BoardSize), Board[FlattenIndex(x, y, BoardSize)]);
		}
	}
	
	TurnStone = PlayersStone;
	NumFullSpaces = 0;
}

void ABoard::PlaceStone(int32 index, EStoneType stoneType)
{
	const int32 x = index % BoardSize;
	const int32 y = index / BoardSize;

	const FVector location = FVector(x * (SpriteSize + SpriteMargin), 1, y * (SpriteSize + SpriteMargin));
	const FRotator rotation = FRotator(0, 0, 0);
	APaperSpriteActor* sprite = GetWorld()->SpawnActor<APaperSpriteActor>(Stones[stoneType], location, rotation);

	if(stoneType != EStoneType::Empty)
	{
		sprite->GetRenderComponent()->SetSpriteColor(FLinearColor(0.5, 0.5, 0.5));

		if(LastPlacedStone != nullptr)
		{
			LastPlacedStone->GetRenderComponent()->SetSpriteColor(FLinearColor(1, 1, 1));
		}

		LastPlacedStone = sprite;
	}
	
	if(BoardActors[index] != nullptr)
	{
		BoardActors[index]->Destroy();
	}
	
	BoardActors[index] = sprite;
	Board[index] = stoneType;
	
	if(stoneType == EStoneType::X)
	{
		TurnStone = EStoneType::O;
	}
	else if(stoneType == EStoneType::O)
	{
		TurnStone = EStoneType::X;
	}
	
	NumFullSpaces++;
}

void ABoard::Solve(int32 fromAction)
{
	Solutions.clear();
	
	if(GomokuSolvers.size() == 0)
	{
		for (int i = 0; i < 2; ++i)
		{
			Solver* solver = new Solver(TimeToSolve, IterationsToSolve);
			GomokuSolvers.push_back(solver);
		}
	}
	
	for (const auto& solver : GomokuSolvers)
	{
		Worker* worker = new Worker(solver, Board, TurnStone, fromAction);
		Workers.push_back(worker);
	}
}

int32 ABoard::TryGetSolution()
{
	if(Workers.size() == 0)
	{
		return -1;
	}
	for (const auto worker : Workers)
	{
		if(!worker->GetIsDone())
			return -1;
	}
	
	int32 move = 0;
	std::unordered_map<int, Evaluation> total;

	for (auto worker : Workers)
	{
		std::unordered_map<int, Evaluation> edges = worker->Result;

		if(edges.size() == 0)
		{
			int32 a = 0;
		}
		
		for (auto& edge : edges)
		{
			if(total.find(edge.first) == total.end())
			{
				total.emplace(edge.first, edge.second);
			}
			else
			{
				total[edge.first].NumVisited += edge.second.NumVisited;
				total[edge.first].SumScore += edge.second.SumScore;
			}
		}
	}

	std::vector<std::pair<int, Evaluation>> sumEdgeVector;
	for (auto edge : total)
	{
		sumEdgeVector.push_back(std::make_pair(edge.first, edge.second));
	}
	
	std::sort(sumEdgeVector.begin(), sumEdgeVector.end(),  [](const std::pair<int, Evaluation> & a, const std::pair<int, Evaluation> & b) -> bool
	{ 
		return a.second.NumVisited > b.second.NumVisited; 
	});
	
	if(sumEdgeVector.size() > 0)
	{
		move = sumEdgeVector[0].first;
	}

	for (const auto worker : Workers)
	{
		delete worker;
	}
	Workers.clear();

	return move;
}

bool ABoard::IsWinningMove(int32 move, EStoneType stoneType) const
{
	return BoardState::FindSequence(Board, move, stoneType, 5);
}

void ABoard::OnMouseClicked(float viewPortX, float viewPortY)
{
	// UE_LOG(LogTemp, Warning, TEXT("viewPortX %f, viewPortY %f"), viewPortX, viewPortY);

	float x = viewPortX * CameraWidth;	
	float y = viewPortY * CameraWidth;

	UE_LOG(LogTemp, Warning, TEXT("x %f, y %f"), x, y);

	if (PlayersStone == TurnStone)
	{
		const int32 index = ClickPositionToIndex(x, y);
		
		if(Board[index] == EStoneType::Empty)
		{
			PlaceStone(index, PlayersStone);
			if(!IsWinningMove(index, PlayersStone))
			{
				if(NumFullSpaces < Board.size())
				{
					Solve(GomokuSolvers.size() == 0 ? -1 : index);
				}
			}
			else
			{
				LastPlacedStone->GetRenderComponent()->SetSpriteColor(FLinearColor(0, 1, 0));
				PlayerController->ShowEndUI(true);
			}
		}
	}
}

int32 ABoard::ClickPositionToIndex(const float x, const float y) const
{
	const int32 xx = x / (SpriteSize + SpriteMargin);
	const int32 yy = y / (SpriteSize + SpriteMargin);

	return FlattenIndex(xx, yy, BoardSize);
}

int32 ABoard::FlattenIndex(const int32 x, const int32 y, int32 boardSize)
{
	return x + y * boardSize;
}
