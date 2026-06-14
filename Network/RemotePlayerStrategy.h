#pragma once
#include <IBotStrategy.h>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>

// Host-side stand-in for a remote human, plugged into GameController as an
// IBotStrategy alongside the AI bots. PickMove() runs on the game worker thread
// and blocks until the NetworkServer (main thread) supplies the move that arrived
// over TCP. At the start of each turn it fires the "turn requested" callback the
// server installs; the server uses it to post a "your_turn" to the right client
// on its own thread. On disconnect the player forfeits (Move::Invalid), which
// also unblocks any in-flight PickMove so the round can resolve.
//
// Deliberately NOT a QObject: GameController owns it via unique_ptr and destroys
// it on the worker thread, so keeping it a plain object avoids QObject cross-
// thread-affinity hazards. The server holds a raw pointer for SupplyMove/Disconnect.
class RemotePlayerStrategy : public IBotStrategy {
public:
    Move PickMove() override;            // worker thread: blocks until a move/disconnect
    void NotifyResult(int) override {}   // remote client learns the result via view events

    // Installed once by the server before play starts; invoked on the worker thread
    // at the start of each PickMove. Must be cheap and thread-safe (it marshals).
    void SetOnTurnRequested(std::function<void()> cb) { mOnTurnRequested = std::move(cb); }

    // Called on the main (network) thread.
    void SupplyMove(Move move);          // deliver the client's choice; unblock PickMove
    void Disconnect();                   // permanent forfeit; unblock PickMove with Invalid

private:
    std::function<void()>   mOnTurnRequested;
    std::mutex              mMtx;
    std::condition_variable mCv;
    std::optional<Move>     mMove;
    bool                    mDisconnected = false;
};
