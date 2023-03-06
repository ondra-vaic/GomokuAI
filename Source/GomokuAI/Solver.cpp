#include "Solver.h"
#include <chrono>

Solver::Solver(float minTime, int32 minIteration)
{
	TimeToSolve = minTime;
	IterationsToSolve = minIteration;
}

void Solver::Initialize(std::vector<EStoneType>& board, EStoneType turnStone)
{
	for (auto node : Graph)
	{
		delete node.first;
	}
	Graph.clear();
	Graph.reserve(IterationsToSolve * 10);
	
	TreeRoot = new BoardState(nullptr, board, turnStone, -1);
	TreeRoot->CalculateHash();
	RootEvaluation = Evaluation{0, 0};
	InsertBoardState(TreeRoot);
}

void Solver::CleanDataStructures(std::vector<EStoneType>& board, EStoneType turnStone, int32 fromAction)
{
	BoardState* mock = new BoardState(nullptr, board, turnStone, fromAction);
	mock->CalculateHash();
	BoardState* newRoot = Graph.find(mock)->first;
	
	if(Graph.find(mock) == Graph.end() || newRoot->Children.size() == 0)
	{
		Initialize(board, turnStone);
		return;
	}
	delete mock;

	std::unordered_set<BoardStateEdge, BoardStateEdgeHash, BoardStateEdgeEquals> edgesToKeep;
	std::unordered_set<BoardState*, BoardStateHash, BoardStateEquals> childrenToKeep;
	std::vector<BoardState*> childrenToDelete;

	std::stack<BoardState*> currentChildren;
	currentChildren.push(newRoot);
	
	while(currentChildren.size() != 0)
	{
		BoardState* currentNode = currentChildren.top();
		currentChildren.pop();

		for (auto& parent : currentNode->Parents)
		{
			edgesToKeep.insert({parent.second, parent.first});
			
			for (auto& parent2 : parent.second->Parents)
			{
				edgesToKeep.insert({parent2.second, parent2.first});
			}
		}
		
		for (auto& child : currentNode->Children)
		{
			currentChildren.push(child.second);
			childrenToKeep.insert(child.second);
		}
	}

	auto graphIt = Graph.begin();

	while(graphIt != Graph.end())
	{
		bool hasUsefulEdge = false;
		auto edgesIt = graphIt->second.begin();
		
		while (edgesIt != graphIt->second.end())
		{
			if(edgesToKeep.find(BoardStateEdge{graphIt->first, edgesIt->first}) != edgesToKeep.end())
			{
				hasUsefulEdge = true;
				++edgesIt;
			}
			else
			{
				BoardState* child = graphIt->first->Children.find(edgesIt->first)->second;
				child->Parents.erase(edgesIt->first);
				graphIt->first->Children.erase(edgesIt->first);
				edgesIt = graphIt->second.erase(edgesIt);
			}
		}
		
		if(!hasUsefulEdge && childrenToKeep.find(graphIt->first) == childrenToKeep.end())
		{
			childrenToDelete.push_back(graphIt->first);
			graphIt = Graph.erase(graphIt);
		}
		else
		{
			++graphIt;	
		}
	}

	for (auto child : childrenToDelete)
	{
		delete child;
	}

	TreeRoot = newRoot;
}

void Solver::Solve()
{
	auto start = std::chrono::high_resolution_clock::now();
	int32 numIterations = 0;
			
	while (numIterations < IterationsToSolve)
	{
		if(numIterations > 100000) break;
		
		// find not expanded
		std::vector<BoardStateEdge> path;
		path.push_back(BoardStateEdge{TreeRoot, -1});
		
		BoardStateEdge currentEdge = path[0];

		while(currentEdge.State->IsExpanded())
		{
			currentEdge = FindBestUcbChild(currentEdge.State);
			path.push_back(currentEdge);
		}
		
		// if visited expand
		if(currentEdge.Action == -1 || IsVisited(currentEdge.State) && !currentEdge.State->IsLeaf())
		{
			Expand(currentEdge.State);
			if(!currentEdge.State->IsLeaf())
			{
				currentEdge = FindBestUcbChild(currentEdge.State);
				path.push_back(currentEdge);
			}
		}

		// simulate
		float score = currentEdge.State->Simulate();
		
		// back prop
		for (int32 i = path.size() - 1; i >= 1; --i)
		{
			UpdateEdge(path[i - 1].State, path[i].Action, score);
			score = 1 - score;
		}

		RootEvaluation.NumVisited++;
		RootEvaluation.SumScore += score;
		
		numIterations++;
		Cache.clear();
	}
	
	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	UE_LOG(LogTemp, Warning, TEXT("iter %d time %d"), numIterations, (int32)(elapsed.count()/1000));
}

 std::unordered_map<int32, Evaluation> Solver::Run(std::vector<EStoneType>& board, EStoneType turnStone, int32 fromAction)
{
	if(fromAction != -1)
	{
		CleanDataStructures(board, turnStone, fromAction);

		// if can win just win and know it
		if(TreeRoot->DecisiveMoveIndex != -1 && TreeRoot->LeafScore == 1)
		{ 
			return std::unordered_map<int32, Evaluation>{{TreeRoot->DecisiveMoveIndex, Evaluation{1000000, 1000000}}};
		}

		// check if can win
		for (int i = 0; i < TreeRoot->Board.size(); ++i)
		{
			if(TreeRoot->IsWinningMove(i, TreeRoot->Turn, 5) && TreeRoot->CanPlayAt(i))
			{
				return std::unordered_map<int32, Evaluation>{{i, Evaluation{1000000, 1000000}}};
			}
		}
		
		// if opponent can win play his move
		for (int i = 0; i < TreeRoot->Board.size(); ++i)
		{
			if(TreeRoot->IsWinningMove(i, ChangeTurn(TreeRoot->Turn), 5) && TreeRoot->CanPlayAt(i))
			{
				return std::unordered_map<int32, Evaluation>{{i, Evaluation{1000000, 1000000}}};
			}
		}
	}
	else
	{
		Initialize(board, turnStone);
	}

	Solve();
	return GetEdgeEvaluations(TreeRoot);
}

EStoneType Solver::ChangeTurn(EStoneType stoneType)
{
	if(stoneType == EStoneType::X)
	{
		return EStoneType::O;
	}
	
	return EStoneType::X;
}

void Solver::Expand(BoardState* boardState)
{
	std::vector<BoardStateEdge> outNewChildren;
	boardState->Expand(outNewChildren);

	for (auto& newChild : outNewChildren)
	{
		if(Graph.find(newChild.State) != Graph.end())
		{
			// add to boardStates children the pointer to child which we already have 
			boardState->Children.emplace(newChild.Action, Graph.find(newChild.State)->first);

			// add a new parent to the child
			Graph.find(newChild.State)->first->Parents.emplace(newChild.Action, boardState);

			// delete the newly created child because we already have it
			delete newChild.State;
		}
		else
		{
			// we dont have the new child so we add it to the boardState's children 
			boardState->Children.emplace(newChild.Action, newChild.State);
			
			// insert the new child to the graph 
			InsertBoardState(newChild.State);
		}

		// insert a new edge from the boardState to new child
		InsertEdge(boardState, newChild.Action);
	}
}

float Solver::GetUcb(BoardState* boardState)
{
	if(boardState->DecisiveMoveIndex != -1)
	{
		if(boardState->LeafScore == 0)
		{
			return -1000;
		}
		return 1000;
	}
	
	const Evaluation stateEvaluation = GetGameStateEvaluation(boardState);
	
	int32 sumParentVisited = 0;
	for (const auto parentState : boardState->Parents)
	{
		Evaluation e = GetGameStateEvaluation(parentState.second);
		sumParentVisited += e.NumVisited;
	}

	if(stateEvaluation.NumVisited == 0)
		return 1000;

	const float avg = stateEvaluation.SumScore / stateEvaluation.NumVisited;
	const float expansion = 1.4f * FMath::Sqrt(FMath::Loge(sumParentVisited) / stateEvaluation.NumVisited);
	const float heuristics = boardState->Heuristic / (stateEvaluation.NumVisited + 1);
	
	return avg + expansion + heuristics;
}

Evaluation Solver::GetGameStateEvaluation(BoardState* boardState)
{
	Evaluation nodeEvaluation = {0, 0};

	const auto cacheIt = Cache.find(boardState);  
	if(cacheIt != Cache.end())
	{
		return cacheIt->second; 
	}
	
	for (const auto parentState : boardState->Parents)
	{
		const Evaluation e = GetEdgeEvaluation(parentState.second, parentState.first);
		nodeEvaluation.NumVisited += e.NumVisited;
		nodeEvaluation.SumScore += e.SumScore;
	}
	
	Cache.emplace(boardState, nodeEvaluation);
	return nodeEvaluation;
}

BoardStateEdge Solver::FindBestUcbChild(BoardState* boardState)
{
	BoardStateEdge bestChild = BoardStateEdge{nullptr, -1};
	
	for (const auto childState : boardState->Children)
	{
		if(bestChild.State == nullptr)
		{
			bestChild.Action = childState.first;
			bestChild.State = childState.second;
		}

		float currentChildUCB = GetUcb(childState.second);
		float bestChildUCB = GetUcb(bestChild.State);
		
		if(currentChildUCB > bestChildUCB)
		{
			bestChild.Action = childState.first;
			bestChild.State = childState.second;
		}
	}

	return bestChild;
}

Evaluation Solver::GetEdgeEvaluation(BoardState* boardState, int32 action) const
{
	if(boardState == nullptr)
	{
		return RootEvaluation;
	}
	
	const std::unordered_map<int, Evaluation>& edges = Graph.find(boardState)->second;
	const Evaluation evaluation = edges.find(action)->second;
    return evaluation;
}

void Solver::InsertEdge(BoardState* boardState, int32 action)
{
	if(Graph.find(boardState)->second.find(action) == Graph.find(boardState)->second.end())
	{
		Graph.find(boardState)->second.emplace(action, Evaluation{0, 0});
	} 
}

void Solver::InsertBoardState(BoardState* boardState)
{
	Graph.emplace(boardState, std::unordered_map<int32, Evaluation>{});
}

void Solver::UpdateEdge(BoardState* boardState, int32 action, float scoreDelta)
{
	std::unordered_map<int, Evaluation>& edges = Graph.find(boardState)->second;
	Evaluation& evaluation = edges.find(action)->second;
	evaluation.SumScore += scoreDelta;
	evaluation.NumVisited++;
}

bool Solver::IsVisited(BoardState* boardState)
{
	return GetGameStateEvaluation(boardState).NumVisited > 0; 
}

std::unordered_map<int32, Evaluation> Solver::GetEdgeEvaluations(BoardState* boardState)
{
	if(boardState->Children.size() == 0)
	{
		if(boardState->DecisiveMoveIndex != -1)
		{
			return std::unordered_map<int32, Evaluation>{{boardState->DecisiveMoveIndex, Evaluation{1000000, 1000000}}};
		}
		
		return std::unordered_map<int32, Evaluation>{};
	}

	return Graph.find(boardState)->second;
}

std::unordered_map<int, Evaluation> Solver::GetRootEdgeEvaluations()
{
	return GetEdgeEvaluations(TreeRoot);
}
