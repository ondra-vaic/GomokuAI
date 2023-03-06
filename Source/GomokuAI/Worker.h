#pragma once
#include "BoardState.h"
#include "Solver.h"

class Solver;

class Worker : FRunnable
{

public:

	Worker(Solver* solver, std::vector<EStoneType>& board, EStoneType turnStone, int32 fromAction);

	virtual ~Worker() override;

	virtual bool Init() override; // Do your setup here, allocate memory, ect.

	virtual uint32 Run() override; // Main data processing happens here
	
	bool GetIsDone(){return this->IsDone;}
	
	std::unordered_map<int32, Evaluation> Result;
	
private:

	FRunnableThread* Thread;
	
	Solver* GomokuSolver;

	std::vector<EStoneType> Board;
	
	EStoneType TurnStone;
	
	int32 FromAction;

	bool IsDone;

};
