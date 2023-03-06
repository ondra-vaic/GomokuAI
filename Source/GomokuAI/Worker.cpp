#include "Worker.h"
#include "Solver.h"

Worker::Worker(Solver* solver, std::vector<EStoneType>& board, EStoneType turnStone, int32 fromAction)
{
	Board = board;
	TurnStone = turnStone;
	FromAction = fromAction;
	GomokuSolver = solver;
	IsDone = false;
	Thread = FRunnableThread::Create(this, TEXT("worker"));
}

Worker::~Worker()
{
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
	}
}

bool Worker::Init()
{
	return true;
}

uint32 Worker::Run()
{
	Result = GomokuSolver->Run(Board, TurnStone, FromAction);
	if(Result.size() == 0)
	{
		int32 a = 0;
	}
	IsDone = true;
	return 0;
}
