#pragma once

#include "BoardState.h"
#include <unordered_map>
#include <stack>

class BoardState;

struct Evaluation
{
	int32 NumVisited;
	
	float SumScore;
};

struct BoardStateEdgeHash {
	size_t operator()(const BoardStateEdge& boardStateEdge) const
	{
		return boardStateEdge.State->Hash * 31 + boardStateEdge.Action;
	}
};

struct BoardStateEdgeEquals {
	size_t operator()(const BoardStateEdge& boardStateEdgeL, const BoardStateEdge& boardStateEdgeR) const
	{
		if(boardStateEdgeL.Action != boardStateEdgeR.Action) return false; 
		
		if(boardStateEdgeL.State == boardStateEdgeR.State)
		{
			return true;
		}
		
		if(boardStateEdgeL.State == nullptr || boardStateEdgeR.State == nullptr) return false;
		
		for (int i = 0; i < boardStateEdgeL.State->Board.size(); ++i)
		{
			if(boardStateEdgeL.State->Board[i] != boardStateEdgeR.State->Board[i]) return false;
		}

		return true;
	}
};

struct BoardStateHash {
	size_t operator()(const BoardState* boardState) const
	{
		return boardState->Hash;
	}
};

struct BoardStateEquals {
	size_t operator()(const BoardState* boardStateL, const BoardState* boardStateR) const
	{
		if(boardStateL == boardStateR) return true;
		if(boardStateL == nullptr || boardStateR == nullptr) return false;
		
		for (int i = 0; i < boardStateL->Board.size(); ++i)
		{
			if(boardStateL->Board[i] != boardStateR->Board[i]) return false;
		}

		return true;
	}
};


class Solver
{

public:

	Solver(float minTime, int32 minIteration);
	
	void Solve();
	
	void Initialize(std::vector<EStoneType>& board, EStoneType turnStone);

	 std::unordered_map<int32, Evaluation> Run(std::vector<EStoneType>& board, EStoneType turnStone, int32 fromAction);

	std::unordered_map<int, Evaluation> GetEdgeEvaluations(BoardState* boardState);

	std::unordered_map<int, Evaluation> GetRootEdgeEvaluations();

public:
	
	static EStoneType ChangeTurn(EStoneType stoneType);

private:

	float GetUcb(BoardState* boardState);

	Evaluation GetGameStateEvaluation(BoardState* boardState);
	
	BoardStateEdge FindBestUcbChild(BoardState* boardState);

	Evaluation GetEdgeEvaluation(BoardState* boardState, int32 action) const;

	void InsertEdge(BoardState* boardState, int32 action);

	void InsertBoardState(BoardState* boardState);

	void UpdateEdge(BoardState* boardState, int32 action, float scoreDelta);

	void Expand(BoardState* boardState);

	bool IsVisited(BoardState* boardState);

	void CleanDataStructures(std::vector<EStoneType>& board, EStoneType turnStone, int32 fromAction);
	
private:
	
	std::unordered_map<BoardState*, std::unordered_map<int32, Evaluation>, BoardStateHash, BoardStateEquals> Graph;
	
	std::unordered_map<BoardState*, Evaluation, BoardStateHash, BoardStateEquals> Cache;

	BoardState* TreeRoot;
	
	Evaluation RootEvaluation;

	float TimeToSolve;
	
	int32 IterationsToSolve;
	
};
