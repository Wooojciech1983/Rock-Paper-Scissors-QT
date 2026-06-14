#pragma once
#include <condition_variable>
#include <deque>
#include <istream>
#include <mutex>
#include <streambuf>
#include <string>

// A blocking, thread-safe std::istream used to feed the local human's input into
// GameController::ReadPlayerMove() / TournamentManager (which call std::getline)
// without blocking the Qt event loop.
//
// The GUI thread calls PushLine("r") when a move button is clicked (or PushLine("")
// for the "press Enter to continue" prompts). The game-logic worker thread blocks
// inside getline until a line is available. Close() injects EOF so a blocked worker
// unwinds cleanly on shutdown (getline fails -> ReadPlayerMove returns Move::Invalid).
//
// Threading contract: PushLine()/Close() are called only from the GUI thread;
// the istream itself (get area) is touched only by the single worker thread.
class MoveStreamBuf : public std::streambuf {
public:
    void PushLine(const std::string& line);
    void Close();

protected:
    int_type underflow() override;

private:
    std::deque<std::string> mQueue;     // pending lines, each already '\n'-terminated
    std::string mCurrent;               // backing store for the active get area
    bool mClosed = false;
    std::mutex mMutex;
    std::condition_variable mCv;
};

class MoveStream : public std::istream {
public:
    MoveStream() : std::istream(&mBuf) {}

    void PushLine(const std::string& line) { mBuf.PushLine(line); }
    void Close() { mBuf.Close(); }

private:
    MoveStreamBuf mBuf;
};
