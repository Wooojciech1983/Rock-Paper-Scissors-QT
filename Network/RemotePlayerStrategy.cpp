#include "RemotePlayerStrategy.h"

Move RemotePlayerStrategy::PickMove()
{
    {
        std::lock_guard<std::mutex> lk(mMtx);
        if (mDisconnected) return Move::Invalid;
        mMove.reset();                 // discard anything received before our turn
    }

    if (mOnTurnRequested)
        mOnTurnRequested();            // ask the server to prompt this client

    std::unique_lock<std::mutex> lk(mMtx);
    mCv.wait(lk, [this] { return mMove.has_value() || mDisconnected; });
    return mMove.value_or(Move::Invalid);
}

void RemotePlayerStrategy::SupplyMove(Move move)
{
    {
        std::lock_guard<std::mutex> lk(mMtx);
        if (mDisconnected) return;
        mMove = move;
    }
    mCv.notify_one();
}

void RemotePlayerStrategy::Disconnect()
{
    {
        std::lock_guard<std::mutex> lk(mMtx);
        mDisconnected = true;
    }
    mCv.notify_one();
}
