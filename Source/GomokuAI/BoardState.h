#pragma once

#include <vector>
#include <unordered_set>
#include <map>
#include <unordered_map>

class BoardState;

UENUM(BlueprintType, Blueprintable)
enum class EStoneType : uint8
{
	Empty = 0,
	X = 1,
	O = 2
};

struct BoardStateEdge
{
	BoardState* State;
	
	int32 Action;
};

class BoardState
{
	
public:

	BoardState(BoardState* parent, const std::vector<EStoneType>& board, EStoneType stoneTurn, int32 fromAction);

public:

	bool IsExpanded() const;
	
	EStoneType GetStoneToPlay() const { return Turn; };

	bool IsLeaf() const;
	
	float Simulate() const;
	
	std::unordered_set<int32> GenerateExpansionIndexes() const;
	
	std::unordered_set<int32> GenerateShuffledExpansionIndexes() const;

	bool CanPlayAt(int32 index) const;

	bool IsWinningMove(int32 index, EStoneType stoneType, int32 matchLength) const;

	static bool FindSequence(const std::vector<EStoneType>& board, int32 index, EStoneType stoneType, int32 matchLength, std::unordered_set<int32>* blockingMoves = nullptr);
	
	void PlaceAt(int32 index, EStoneType stoneType);

	void CalculateHash();
	
	float Evaluate(int32 minMatchLength, int32 maxMatchLength) const;

	void GenerateChildren(const std::unordered_set<int32>& indexes, std::vector<BoardStateEdge>& outChildren);
	
	void Expand(std::vector<BoardStateEdge>& outNewChildren);

	static constexpr float Heuristics[5][3] =
	{
		{0, 0, 0},
		{0, 1, 4},
		{0, 2.25f, 9},
		{0, 4, 16},
		{25, 25, 25},
	};

public:
	
	float LeafScore;
	
	std::vector<EStoneType> Board;
	
	std::unordered_map<int32, BoardState*> Children;

	std::unordered_map<int32, BoardState*> Parents;

	float Heuristic;
	
	EStoneType Turn;

	bool IsLeafState;

	int32 DecisiveMoveIndex;

	size_t Hash;

public:
	
	int32 BoardWidth;
	
};
