#include "BoardState.h"
#include <algorithm>
#include "Board.h"
#include "Solver.h"

BoardState::BoardState(BoardState* parent, const std::vector<EStoneType>& board, EStoneType stoneTurn, int32 fromAction)
{
	Board = board;
	Turn = stoneTurn;
	BoardWidth = FMath::Sqrt(board.size());
	IsLeafState = false;
	LeafScore = -1;
	DecisiveMoveIndex = -1;
	Heuristic = 0;
	Parents.emplace(fromAction, parent);
	Hash = 0;
}

bool BoardState::IsExpanded() const
{
	return Children.size() > 0;
}

bool BoardState::IsLeaf() const
{
	return IsLeafState;
}

float BoardState::Simulate() const
{
	if(IsLeafState)
		return LeafScore;

	const EStoneType startTurn = Turn;
	
	BoardState copy = BoardState(nullptr, Board, startTurn, -1);
	
	EStoneType turn = copy.Turn;

	std::unordered_set<int32> emptyIndexes = GenerateShuffledExpansionIndexes();
	
	while(emptyIndexes.size() != 0)
	{
		int32 move = 0;
		
		for (const int32 index1 : emptyIndexes)
		{
			move = index1;

			if(copy.IsWinningMove(move, turn, 5))
			{
				if(turn == startTurn)
				{
					return 0;
				}
				return 1;
			}
			if(copy.IsWinningMove(move, Solver::ChangeTurn(turn), 5))
			{
				break;
			}
			if(copy.IsWinningMove(move, turn, 4))
			{
				break;
			}
			if(copy.IsWinningMove(move, Solver::ChangeTurn(turn), 4))
			{
				break;
			}
		}

		emptyIndexes.erase(move);
		copy.PlaceAt(move, turn);
		turn = Solver::ChangeTurn(turn);
	}

	return 0.5 + Heuristic / 50;
}

bool BoardState::CanPlayAt(int32 index) const
{
	return Board[index] == EStoneType::Empty;
}

void BoardState::PlaceAt(int32 index, EStoneType stoneType)
{
	Board[index] = stoneType;
}

void BoardState::CalculateHash()
{
	std::string sh = "";
	
	for (int i = 0; i < Board.size(); ++i)
	{
		sh += (int8)Board[i];
	}
	Hash = std::hash<std::string>{}(sh);
}

bool BoardState::IsWinningMove(int32 index, EStoneType stoneType, int32 matchLength) const
{
	return FindSequence(Board, index, stoneType, matchLength);
}

bool BoardState::FindSequence(const std::vector<EStoneType>& board, int32 index, EStoneType stoneType, int32 maxMatchLength, std::unordered_set<int32>* blockingMoves)
{
	const int32 boardWidth = FMath::Sqrt(board.size());
	
	const int32 xStart = index % boardWidth;
	const int32 yStart = index / boardWidth;

	// horizontal
	int32 matchLength = 1;
	bool dir1 = true;
	bool dir2 = true;
	bool isMatch = false;
	bool hasStonesInDir1 = false;
	bool hasStonesInDir2 = false;
	
	std::vector<int32> endPoints;
	for (int32 x = 1; x < maxMatchLength + 2; ++x)
	{
		if(dir1 && xStart + x < boardWidth)
		{
			if(board[ABoard::FlattenIndex(x + xStart, yStart, boardWidth)] == stoneType)
			{
				matchLength++;
				hasStonesInDir1 = true;
			}
			else if(x > 1 && xStart + x < boardWidth && board[ABoard::FlattenIndex(xStart + x, yStart, boardWidth)] == EStoneType::Empty)
			{
				endPoints.push_back(ABoard::FlattenIndex(x + xStart, yStart, boardWidth));
				dir1 = false;
			}
			else
			{
				dir1 = false;
			}
		}
		
		if(dir2 && xStart - x >= 0)
		{
			if(board[ABoard::FlattenIndex(xStart - x, yStart, boardWidth)] == stoneType)
			{
				matchLength++;
				hasStonesInDir2 = true;
			}
			else if(x > 1 && xStart - x >= 0 && board[ABoard::FlattenIndex(xStart - x, yStart, boardWidth)] == EStoneType::Empty)
			{
				endPoints.push_back(ABoard::FlattenIndex(xStart - x, yStart, boardWidth));
				dir2 = false;
			}
			else
			{
				dir2 = false;
			}
		}
		
		if(matchLength >= maxMatchLength)
		{
			isMatch = true;
			if(!blockingMoves)
			{
				return true;
			}
		}
	}

	if(matchLength >= maxMatchLength)
	{
		if(blockingMoves)
		{
			if(hasStonesInDir1 != hasStonesInDir2 && endPoints.size() == 1 && endPoints[0] != index)
			{
				blockingMoves->insert(endPoints[0]);
				blockingMoves->insert(index);
			}
			else if(endPoints.size() == 2)
			{
				blockingMoves->insert(endPoints[0]);
				blockingMoves->insert(endPoints[1]);
				blockingMoves->insert(index);
			}
		}	
	}

	dir1 = true;
	dir2 = true;
	endPoints.clear();
	matchLength = 1;
	hasStonesInDir1 = false;
	hasStonesInDir2 = false;
	
	// vertical
	for (int32 y = 1; y < maxMatchLength + 2; ++y)
	{
		if(dir1 && yStart + y < boardWidth)
		{
			if(board[ABoard::FlattenIndex(xStart, yStart + y, boardWidth)] == stoneType)
			{
				matchLength++;	
				hasStonesInDir1 = true;
			}
			else if(y > 1 && board[ABoard::FlattenIndex(xStart, yStart + y, boardWidth)] == EStoneType::Empty)
			{
				endPoints.push_back(ABoard::FlattenIndex(xStart, yStart + y, boardWidth));
				dir1 = false;
			}
			else
			{
				dir1 = false;
			}
		}
		
		if(dir2 && yStart - y >= 0)
		{
			if(board[ABoard::FlattenIndex(xStart, yStart - y, boardWidth)] == stoneType)
			{
				matchLength++;	
				hasStonesInDir2 = true;
			}
			else if(y > 1 && board[ABoard::FlattenIndex(xStart, yStart - y, boardWidth)] == EStoneType::Empty)
			{
				endPoints.push_back(ABoard::FlattenIndex(xStart, yStart - y, boardWidth));
				dir2 = false;
			}
			else
			{
				dir2 = false;
			}
		}

		if(matchLength >= maxMatchLength)
		{
			isMatch = true;
			if(!blockingMoves)
			{
				return true;
			}
		}
	}

	if(matchLength >= maxMatchLength)
	{
		if(blockingMoves)
		{
			if(hasStonesInDir1 != hasStonesInDir2 && endPoints.size() == 1 && endPoints[0] != index)
			{
				blockingMoves->insert(endPoints[0]);
				blockingMoves->insert(index);
			}
			else if(endPoints.size() == 2)
			{
				blockingMoves->insert(endPoints[0]);
				blockingMoves->insert(endPoints[1]);
				blockingMoves->insert(index);
			}
		}	
	}
	
	dir1 = true;
	dir2 = true;
	endPoints.clear();
	matchLength = 1;
	hasStonesInDir1 = false;
	hasStonesInDir2 = false;
	
	// left to right down 
	for (int32 y = 1; y < maxMatchLength + 2; ++y)
	{
		if(dir1 && yStart + y < boardWidth && xStart - y >= 0)
		{
			if(board[ABoard::FlattenIndex(xStart - y, yStart + y, boardWidth)] == stoneType)
			{
				hasStonesInDir1 = true;
				matchLength++;	
			}
			else if(y > 1 && board[ABoard::FlattenIndex(xStart - y, yStart + y, boardWidth)] == EStoneType::Empty)
			{
				endPoints.push_back(ABoard::FlattenIndex(xStart - y, yStart + y, boardWidth));
				dir1 = false;
			}
			else
			{
				dir1 = false;
			}
		}
		
		if(dir2 && yStart - y >= 0 && xStart + y < boardWidth)
		{
			if(board[ABoard::FlattenIndex(xStart + y, yStart - y, boardWidth)] == stoneType)
			{
				matchLength++;
				hasStonesInDir2 = true;
			}
			else if(y > 1 && board[ABoard::FlattenIndex(xStart + y, yStart - y, boardWidth)] == EStoneType::Empty)
			{
				endPoints.push_back(ABoard::FlattenIndex(xStart + y, yStart - y, boardWidth));
				dir2 = false;
			}
			else
			{
				dir2 = false;
			}
		}
		
		if(matchLength >= maxMatchLength)
		{
			isMatch = true;
			if(!blockingMoves)
			{
				return true;
			}
		}
	}

	if(matchLength >= maxMatchLength)
	{
		if(blockingMoves)
		{
			if(hasStonesInDir1 != hasStonesInDir2 && endPoints.size() == 1 && endPoints[0] != index)
            {
            	blockingMoves->insert(endPoints[0]);
            	blockingMoves->insert(index);
            }
            else if(endPoints.size() == 2)
            {
            	blockingMoves->insert(endPoints[0]);
            	blockingMoves->insert(endPoints[1]);
            	blockingMoves->insert(index);
            }
		}	
	}
	
	dir1 = true;
	dir2 = true;
	endPoints.clear();
	matchLength = 1;
	hasStonesInDir1 = false;
	hasStonesInDir2 = false;
	
	// left to right up
	for (int32 y = 1; y < maxMatchLength + 2; ++y)
	{

		if(dir1 && yStart + y < boardWidth && xStart + y < boardWidth)
		{
			if(board[ABoard::FlattenIndex(xStart + y, yStart + y, boardWidth)] == stoneType)
			{
				hasStonesInDir1 = true;
				matchLength++;	
			}
			else if(y > 1 && board[ABoard::FlattenIndex(xStart + y, yStart + y, boardWidth)] == EStoneType::Empty)
			{
				endPoints.push_back(ABoard::FlattenIndex(xStart + y, yStart + y, boardWidth));
				dir1 = false;
			}
			else
			{
				dir1 = false;
			}		
		}
		
		if(dir2 && yStart - y >= 0 && xStart - y >= 0)
		{
			if(board[ABoard::FlattenIndex(xStart - y, yStart - y, boardWidth)] == stoneType)
			{
				matchLength++;
				hasStonesInDir2 = true;
			}
			else if(y > 1 && board[ABoard::FlattenIndex(xStart - y, yStart - y, boardWidth)] == EStoneType::Empty)
			{
				endPoints.push_back(ABoard::FlattenIndex(xStart - y, yStart - y, boardWidth));
				dir2 = false;
			}
			else
			{
				dir2 = false;
			}
		}
		
		if(matchLength >= maxMatchLength)
		{
			isMatch = true;
			if(!blockingMoves)
			{
				return true;
			}
		}
	}

	if(matchLength >= maxMatchLength)
	{
		if(blockingMoves)
		{
			if(hasStonesInDir1 != hasStonesInDir2 && endPoints.size() == 1 && endPoints[0] != index)
			{
				blockingMoves->insert(endPoints[0]);
				blockingMoves->insert(index);
			}
			else if(endPoints.size() == 2)
			{
				blockingMoves->insert(endPoints[0]);
				blockingMoves->insert(endPoints[1]);
				blockingMoves->insert(index);
			}
		}	
	}

	if(blockingMoves)
	{
		return isMatch;
	}
	
	return false;	
}

float BoardState::Evaluate(int32 minMatchLength, int32 maxMatchLength) const
{
	float total = 0;

	// rows
	for (int32 y = 0; y < BoardWidth; y++)
	{
		for (int32 x = 0; x < BoardWidth;)
		{
			int8 matchLength = 1;
			EStoneType stoneType = Board[ABoard::FlattenIndex(x, y, BoardWidth)];
			
			if(stoneType != EStoneType::Empty)
			{
				int32 xx = x + matchLength;
	
				for (matchLength = 1; matchLength < maxMatchLength; matchLength++)
				{
					xx = x + matchLength;
					
					if (xx > BoardWidth - 1 || 
						Board[ABoard::FlattenIndex(xx - 1, y, BoardWidth)] != Board[ABoard::FlattenIndex(xx, y, BoardWidth)])
					{
						goto breakLabel0;
					}
				}
				breakLabel0:
			
				if (matchLength >= minMatchLength)
				{
					int32 emptySpaces = 0;
					if (xx <= BoardWidth - 1 && 
						Board[ABoard::FlattenIndex(xx, y, BoardWidth)] == EStoneType::Empty)
					{
						emptySpaces++;
					}
					if (x > 0 && 
						Board[ABoard::FlattenIndex(xx - matchLength - 1, y, BoardWidth)] == EStoneType::Empty)
					{
						emptySpaces++;
					}

					total += stoneType != Turn ? Heuristics[matchLength - 1][emptySpaces] : -Heuristics[matchLength - 1][emptySpaces];
				}	
			}
			
			x += matchLength; 
		}	
	}
	
	//cols
	for (int32 x = 0; x < BoardWidth; x++)
	{
		for (int32 y = 0; y < BoardWidth;)
		{
			int8 matchLength = 1;
			EStoneType stoneType = Board[ABoard::FlattenIndex(x, y, BoardWidth)];
			
			if (stoneType != EStoneType::Empty)
			{
				
				int32 yy = y + matchLength;
				
				for (matchLength = 1; matchLength < maxMatchLength; matchLength++)
				{
					yy = y + matchLength;
	
					if (yy > BoardWidth - 1 ||
						Board[ABoard::FlattenIndex(x, yy - 1, BoardWidth)] != Board[ABoard::FlattenIndex(x, yy, BoardWidth)])
					{
						goto breakLabel1;
					}
				}
				breakLabel1:
			
				if(matchLength >= minMatchLength)
				{
					int32 emptySpaces = 0;
					if (yy <= BoardWidth - 1 && 
						Board[ABoard::FlattenIndex(x, yy, BoardWidth)] == EStoneType::Empty)
					{
						emptySpaces++;
					}
					if (y > 0 && 
						Board[ABoard::FlattenIndex(x, yy - matchLength - 1, BoardWidth)] == EStoneType::Empty)
					{
						emptySpaces++;
					}
					
					total += stoneType != Turn ? Heuristics[matchLength - 1][emptySpaces] : -Heuristics[matchLength - 1][emptySpaces];
				}	
			}
			
			y += matchLength; 
		}	
	}
	
	// Decreasing 
	for (int32 x = 0; x < BoardWidth; x++)
	{
		int32 xx = x;

		for (int32 y = 0; y < x;)
		{
			int8 matchLength = 1;
			EStoneType stoneType = Board[ABoard::FlattenIndex(xx, y, BoardWidth)];
		
			if (stoneType != EStoneType::Empty)
			{
				int32 xxx = xx - matchLength + 1;
				int32 yy = y + matchLength - 1;
				
				for (matchLength = 1; matchLength < maxMatchLength; matchLength++)
				{
					xxx = xx - matchLength + 1;
					yy = y + matchLength - 1;	
					
					if (xxx <= 0 ||
						Board[ABoard::FlattenIndex(xxx, yy, BoardWidth)] != Board[ABoard::FlattenIndex(xxx - 1, yy + 1, BoardWidth)])
					{
						goto breakLabel2;
					}
				}
				breakLabel2:
			
				if(matchLength >= minMatchLength)
				{
					int32 emptySpaces = 0;
					if (xxx > 0 && 
						Board[ABoard::FlattenIndex(xxx - 1, yy + 1, BoardWidth)] == EStoneType::Empty)
					{
						emptySpaces++;
					}
					if (xxx + matchLength < BoardWidth - 1 && 
						Board[ABoard::FlattenIndex(xxx + matchLength, yy - matchLength, BoardWidth)] == EStoneType::Empty)
					{
						emptySpaces++;
					}

					total += stoneType != Turn ? Heuristics[matchLength - 1][emptySpaces] : -Heuristics[matchLength - 1][emptySpaces];
				}	
			}
			
			y += matchLength;
			xx -= matchLength;
		}

		if (x == BoardWidth - 1)
			continue;
		
		xx = BoardWidth - 1 - x;

		for (int32 y = BoardWidth - 1; y >= BoardWidth - 1 - x;)
		{
			int8 matchLength = 1;
			EStoneType stoneType = Board[ABoard::FlattenIndex(xx, y, BoardWidth)];
	
			if (stoneType != EStoneType::Empty)
			{
				int32 xxx = xx + matchLength - 1;
				int32 yy = y - matchLength + 1;
				
				for (matchLength = 1; matchLength < maxMatchLength; matchLength++)
				{
					xxx = xx + matchLength - 1;
					yy = y - matchLength + 1;
					
					if (xxx >= BoardWidth - 1 ||
						Board[ABoard::FlattenIndex(xxx, yy, BoardWidth)] != Board[ABoard::FlattenIndex(xxx + 1, yy - 1, BoardWidth)])
					{
						goto breakLabel3;
					}
				}
				breakLabel3:
			
				if(matchLength >= minMatchLength)
				{
					int32 emptySpaces = 0;
					if (xxx + 1 <= BoardWidth - 1 && 
						Board[ABoard::FlattenIndex(xxx + 1, yy - 1, BoardWidth)] == EStoneType::Empty)
					{
						emptySpaces++;
					}
					if (xxx - matchLength >= 0 && 
						Board[ABoard::FlattenIndex(xxx - matchLength, yy + matchLength, BoardWidth)] == EStoneType::Empty)
					{
						emptySpaces++;
					}

					total += stoneType != Turn ? Heuristics[matchLength - 1][emptySpaces] : -Heuristics[matchLength - 1][emptySpaces];
				}	
			}
			
			y -= matchLength;
			xx += matchLength;
		}
	}
	
	// Increasing
	for (int32 x = 0; x < BoardWidth; x++)
	{
		int32 xx = x;

		for (int32 y = 0; y < BoardWidth - x;)
		{
			int8 matchLength = 1;
			EStoneType stoneType = Board[ABoard::FlattenIndex(xx, y, BoardWidth)];
		
			if (stoneType != EStoneType::Empty)
			{
				int32 xxx = xx + matchLength - 1;
				int32 yy = y + matchLength - 1;
					
				for (matchLength = 1; matchLength < maxMatchLength; matchLength++)
				{
					xxx = xx + matchLength - 1;
					yy = y + matchLength - 1;
					
					if (xxx >= BoardWidth - 1 || 
						Board[ABoard::FlattenIndex(xxx, yy, BoardWidth)] != Board[ABoard::FlattenIndex(xxx + 1, yy + 1, BoardWidth)])
					{
						goto breakLabel4;
					}
				}
				breakLabel4:
			
				if(matchLength >= minMatchLength)
				{
					int32 emptySpaces = 0;
					if (xxx < BoardWidth - 1 && 
						Board[ABoard::FlattenIndex(xxx + 1, yy + 1, BoardWidth)] == EStoneType::Empty)
					{
						emptySpaces++;
					}
					if (xxx - matchLength > 0 && 
						Board[ABoard::FlattenIndex(xxx - matchLength, yy - matchLength, BoardWidth)] == EStoneType::Empty)
					{
						emptySpaces++;
					}

					total += stoneType != Turn ? Heuristics[matchLength - 1][emptySpaces] : -Heuristics[matchLength - 1][emptySpaces];
				}	
			}
			
			y += matchLength;
			xx += matchLength;
		} 

		if (x == 0 || x == 9)
			continue;
		
		xx = x;

		for (int32 y = BoardWidth - 1; y >= BoardWidth - 1 - x;)
		{
			int8 matchLength = 1;
			EStoneType stoneType = Board[ABoard::FlattenIndex(xx, y, BoardWidth)];
	
			if (stoneType != EStoneType::Empty)
			{
				int32 xxx = xx - matchLength + 1;
				int32 yy = y - matchLength + 1;
				
				for (matchLength = 1; matchLength < maxMatchLength; matchLength++)
				{
					xxx = xx - matchLength + 1;
					yy = y - matchLength + 1;
					
					if (xxx <= 0 ||
						Board[ABoard::FlattenIndex(xxx, yy, BoardWidth)] != Board[ABoard::FlattenIndex(xxx - 1, yy - 1, BoardWidth)])
					{
						goto breakLabel5;
					}
				}
				breakLabel5:
			
				if(matchLength >= minMatchLength)
				{
					int32 emptySpaces = 0;
					if (xxx > 0 && 
						Board[ABoard::FlattenIndex(xxx - 1, yy - 1, BoardWidth)] == EStoneType::Empty)
					{
						emptySpaces++;
					}
					if (xxx + matchLength < BoardWidth - 1 && 
						Board[ABoard::FlattenIndex(xxx + matchLength, yy + matchLength, BoardWidth)] == EStoneType::Empty)
					{
						emptySpaces++;
					}

					total += stoneType != Turn ? Heuristics[matchLength - 1][emptySpaces] : -Heuristics[matchLength - 1][emptySpaces];
				}	
			}
			
			y -= matchLength;
			xx -= matchLength;
		}
	}

	return total;
}

void BoardState::GenerateChildren(const std::unordered_set<int32>& indexes, std::vector<BoardStateEdge>& outChildren)
{
	for (const int32 index : indexes)
	{
		if(CanPlayAt(index))
		{
			BoardState* child = new BoardState(this, Board, Solver::ChangeTurn(Turn), index);
			child->PlaceAt(index, Turn);
			child->CalculateHash();
			child->Heuristic = child->Evaluate(2, 6);
			outChildren.push_back({child, index});
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("==========="));
		}
	}
}

void BoardState::Expand(std::vector<BoardStateEdge>& outNewChildren)
{
	const std::unordered_set<int32> indexesToExpand = GenerateShuffledExpansionIndexes();
	
	std::unordered_set<int32> winning5Moves;
	std::unordered_set<int32> blockWinning5Moves;
	std::unordered_set<int32> winning4Moves;
	std::unordered_set<int32> blockingWinning4Moves;
	std::unordered_set<int32> winning4NotFullMoves;

	for (const int32 index : indexesToExpand)
	{
		if(IsWinningMove(index, Turn, 5))
		{
			winning5Moves.insert(index);
			break;
		}

		if(IsWinningMove(index, Solver::ChangeTurn(Turn), 5))
		{
			blockWinning5Moves.insert(index);
		}
		
		std::unordered_set<int32> winning4MovesForIndex;
		
		if(FindSequence(Board, index, Turn, 4, &winning4MovesForIndex))
		{
			// I guess this is wrong
			if(winning4MovesForIndex.size() > 0)
			{
				winning4Moves.insert(index);
			}
			else
			{
				winning4NotFullMoves.insert(index);
			}
		}
		FindSequence(Board, index, Solver::ChangeTurn(Turn), 4, &blockingWinning4Moves);
	}

	if(winning5Moves.size() > 0)
	{
		IsLeafState = true;
		LeafScore = 0;
		DecisiveMoveIndex = *winning5Moves.begin();
		return;
	}
	
	if(blockWinning5Moves.size() > 0)
	{
		if(blockWinning5Moves.size() >= 2)
		{
			IsLeafState = true;
			LeafScore = 1;
			DecisiveMoveIndex = *blockWinning5Moves.begin();
		}
		else
		{
			GenerateChildren(blockWinning5Moves, outNewChildren);
		}
		return;
	}
	
	if(winning4Moves.size() > 0)
	{
		IsLeafState = true;
		LeafScore = 0;
		DecisiveMoveIndex = *winning4Moves.begin();
		return;
	}
	
	if(blockingWinning4Moves.size() > 0)
	{
		GenerateChildren(blockingWinning4Moves, outNewChildren);
		GenerateChildren(winning4NotFullMoves, outNewChildren);
	}
	else
	{
		GenerateChildren(indexesToExpand, outNewChildren);
	}
	
	if(outNewChildren.size() == 0)
	{
		IsLeafState = true;
		LeafScore = 0.5;
	}
}

std::unordered_set<int32> BoardState::GenerateExpansionIndexes() const
{
	std::vector<std::tuple<int32, int32>> occupiedSpaces;
	
	for (int32 y = 0; y < BoardWidth; ++y)
	{
		for (int32 x = 0; x < BoardWidth; ++x)
		{
			if(Board[ABoard::FlattenIndex(x, y, BoardWidth)] != EStoneType::Empty)
			{
				occupiedSpaces.push_back(std::make_tuple(x, y));
			}
		}	
	}

	/*
	 *  Only consider spaces which are in this pattern around the X
	 *  That's all diagonal, vertical or horizontal spaces up to distance 2,
	 *  Spaces which are like knights move in chess are not considered for expansion 
	 *  
	 *         |* * *|
	 *         | *** |
	 *         |**X**|
	 *         | *** |
	 *         |* * *|
	 */
	
	std::unordered_set<int32> indexes;

	for (int32 i = 0; i < occupiedSpaces.size(); ++i)
	{
		std::tuple<int32, int32> position = occupiedSpaces[i];

		for (int32 y = -2; y <= 2; ++y)
		{
			for (int32 x = -2; x <= 2; ++x)
			{
				const int32 xx = std::get<0>(position) + x;
				const int32 yy = std::get<1>(position) + y;
				
				if(xx >= 0 && xx < BoardWidth && yy >= 0 && yy < BoardWidth)
				{
					const bool xDivisible = x % 2 == 0;
					const bool yDivisible = y % 2 == 0;
					
					if(xDivisible == yDivisible || x == 0 || y == 0)
					{
						if(Board[ABoard::FlattenIndex(xx, yy, BoardWidth)] == EStoneType::Empty)
						{
							indexes.insert(ABoard::FlattenIndex(xx, yy, BoardWidth));	
						}
					}
				}
			}	
		}
	}
	
	return indexes;
}

std::unordered_set<int32> BoardState::GenerateShuffledExpansionIndexes() const
{
	const std::unordered_set<int32> indexes = GenerateExpansionIndexes();

	std::vector<int32> indexesVector;
	for (int32 index : indexes)
	{
		indexesVector.push_back(index);
	}

	size_t threadId = std::hash<std::thread::id>()(std::this_thread::get_id());
	srand(unsigned(time(NULL)) + threadId);
	std::random_shuffle(indexesVector.begin(), indexesVector.end());
	
	std::unordered_set<int32> emptyIndexes;
	for (auto index : indexesVector)
	{
		emptyIndexes.insert(index);
	}
	
	return emptyIndexes;
}
