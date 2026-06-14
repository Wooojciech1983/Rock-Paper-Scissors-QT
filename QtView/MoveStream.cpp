#include "MoveStream.h"

void MoveStreamBuf::PushLine(const std::string& line)
{
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mQueue.push_back(line + '\n');
    }
    mCv.notify_one();
}

void MoveStreamBuf::Close()
{
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mClosed = true;
    }
    mCv.notify_all();
}

MoveStreamBuf::int_type MoveStreamBuf::underflow()
{
    std::unique_lock<std::mutex> lock(mMutex);
    mCv.wait(lock, [this] { return !mQueue.empty() || mClosed; });

    if (mQueue.empty())                       // closed with no remaining input
        return traits_type::eof();

    mCurrent = std::move(mQueue.front());
    mQueue.pop_front();

    char* const base = mCurrent.data();
    setg(base, base, base + mCurrent.size());
    return traits_type::to_int_type(*gptr());
}
