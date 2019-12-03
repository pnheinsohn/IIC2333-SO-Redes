#pragma once
#include <stdbool.h>

typedef enum msgType
{
  StartConnection = 1,
  ConnectionEstablished = 2,
  AskNickname = 3,
  RetNickname = 4,
  Oponentfound = 5,
  Start = 6,
  Scores = 7,
  WhosFirst = 8,
  BoardState = 9,
  SendMove = 10,
  ErrMove = 11,
  OkMove = 12,
  End = 13,
  GameWinnerLoser = 14,
  AskNewGame = 15,
  AnswerNewGame = 16,
  Disconnect = 17,
  ErrBadPkg = 18,
  SendMsg = 19,
  SpreadMsg = 20,
  Image = 64
} MsgType;
