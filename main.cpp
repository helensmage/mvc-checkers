#include "Board.h"
#include "Board.cpp"
#include "Cell.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

using movePos = std::pair<pos, pos>;

class Observer {
public:
    virtual void update() = 0;
};


class Observable {
public:
    void addObserver(Observer* observer) {
      _observers.push_back(observer);
    }
    void notifyUpdate() {
      int size = _observers.size();
      for (int i = 0; i < size; i++)
      {
         _observers[i]->update();
      }
   }
private:
   vector<Observer*> _observers;
};


class Model : public Observable { 
public:
   Model() : mWhiteScore(0), mBlackScore(0), mLastPlayer(Player::NONE), bIsSurrender(false) {}
   enum class Player { NONE, BLACK, WHITE };
   bool bIsSurrender;

   void up() {
     notifyUpdate();
   }

   size_t GetWhiteScore() {
     return mWhiteScore;
   }

   size_t GetBlackScore() {
     return mBlackScore;
   }

   std::string GetCurrentPlayer() const {
      std::string player;
      if (mLastPlayer == Player::WHITE) {
        player = "Black";
      }
      else {
        player = "White";
      }
      return std::move(player);
   }

   Player GetWinner() const {
     Player winner(Player::NONE);
      if (bIsSurrender) {
        if (mLastPlayer == Player::WHITE) {
          winner = Player::BLACK;
        }
        else if (mLastPlayer == Player::BLACK) {
          winner = Player::WHITE;
        }
      }
      else {
        if (mWhiteScore == 12) {
          winner = Player::WHITE;
        }
        else if (mBlackScore == 12) {
          winner = Player::BLACK;
        }
      }
      return winner;
   }

   bool GetDirection() const {
     bool direction;
      switch(mLastPlayer) {
      case Player::WHITE:
        direction = false;
        break;
      default:
        direction = true;
        break;
      }
      return direction;
   }

   movePos GetMove(std::string line1, std::string line2) {
      movePos position;
      size_t p1, p2;
      bool isLine1Number = true, isLine2Number = true;
      std::for_each(line1.cbegin(), line1.cend(), [&](char c) { if (!isdigit(c)) isLine1Number = false; });
      std::for_each(line2.cbegin(), line2.cend(), [&](char c) { if (!isdigit(c)) isLine2Number = false; });
      if (!isLine1Number || !isLine2Number) {
        if (line1 == "ff") {
          position.first.first = 0;
          position.first.second = 0;
          position.second.first = 0;
          position.second.second = 0;
        }
        else {
          position.first.first = 1;
          position.first.second = 1;
          position.second.first = 1;
          position.second.second = 1;
        }
      }
      else {
        p1 = static_cast<size_t>(std::stoi(line1));
        p2 = static_cast<size_t>(std::stoi(line2));
        position.first.first = p1 / 10;
        position.first.second = p1 - 10 * position.first.first;
        position.second.first = p2 / 10;
        position.second.second = p2 - 10 * position.second.first;
      }
      return std::move(position);
    }

    void UpdateScore() {
      if (mLastPlayer == Player::WHITE) {
        mBlackScore++;
      }
      else {
        mWhiteScore++;
      }
    }

    void SwitchPlayer() {
      if (mLastPlayer == Player::WHITE) {
        mLastPlayer = Player::BLACK;
      }
      else {
        mLastPlayer = Player::WHITE;
      }
    }

    std::string CastPlayer(Player player) const {
      std::string playerStr;
      switch (player) {
      case Player::WHITE:
        playerStr = "White player";
        break;
      default:
        playerStr = "Black player";
        break;
      }
      return playerStr;
    }

    void EndGame(std::string player) {
      std::cout << std::endl << "Game Over!" << std::endl;
      std::cout << player << " wins!" << std::endl;
    }

    std::string CastState(Cell::State state) {
     std::string result;
      switch(state) {
      case Cell::State::BLACK:
        result = "B";
        break;
      case Cell::State::WHITE:
        result = "W";
        break;
      default:
        result = " ";
        break;
      }
      return std::move(result);
   }

private:
   size_t mWhiteScore;
   size_t mBlackScore;
   Player mLastPlayer;
};


class ConsoleView: public Observer {
public:
   ConsoleView(Model* model, BoardModel* board) : _model(model), _mboard(board) {
      _model->addObserver(this);
   }

   void PrintScore(size_t whiteScore, size_t blackScore) {
     std::cout << "White Player score: " << whiteScore << std::endl;
     std::cout << "Black Player score: " << blackScore << std::endl << std::endl;
   }

   void DrawBoard(const map1& board) {
      size_t boardSize = static_cast<size_t>(sqrt(board.size()));
      for (size_t i = 0; i < boardSize; i++) {
        std::cout << "|";
        for (size_t j = 0; j < boardSize; j++) {
          std::string cellValue = _model->CastState(board.at(pos(i, j)).GetState());
          std::cout << cellValue;
          std::cout << "|";
        }
        std::cout << std::endl;
      }
      std::cout << std::endl;
      std::cout << std::endl;
   }

   void update() {
      if (_model->bIsSurrender) {
        EndGame();
      }
      else {
        PrintScore(_model->GetWhiteScore(), _model->GetBlackScore());
        DrawBoard(_mboard->GetMap());
        std::cout << _model->GetCurrentPlayer() << " move:" << std::endl;
      }
   }

   void EndGame() {
      std::cout << std::endl << "Game Over!" << std::endl;
      std::cout << _model->CastPlayer(_model->GetWinner()) << " wins!" << std::endl;
   }

private:
   Model* _model;
   BoardModel* _mboard;
};


class Controller {
public:
   Controller(Model* model, BoardModel* board) : _model(model), _mboard(board) {}
   
   void start() {
      _mboard->ResetMap();
      while (_model->GetWinner() == Model::Player::NONE) {
        BoardModel::MoveResult moveResult(BoardModel::MoveResult::PROHIBITED);
        std::string line1, line2;
        while (moveResult == BoardModel::MoveResult::PROHIBITED) {
          bool direction = _model->GetDirection();
          _model->up();
          std::cin >> line1;
          std::cin >> line2;
          auto newMove = std::move(_model->GetMove(line1, line2));
          moveResult = _mboard->MakeMove(newMove.first, newMove.second, direction);
        }
        if (moveResult == BoardModel::MoveResult::FF) {
          _model->bIsSurrender = true;
          break;
        }
        if (moveResult == BoardModel::MoveResult::SUCCESSFUL_COMBAT) {
          _model->UpdateScore();
        }
        _model->SwitchPlayer();
      }
      _model->up();
   }

private:
   Model* _model;
   BoardModel* _mboard;
};


int main() {
   Model model;
   BoardModel mboard;
   ConsoleView view(&model, &mboard);
   Controller controller(&model, &mboard);
   controller.start();
   return 0;
}
