#pragma once
#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <optional>

// Wire protocol for the host/client link: one compact JSON object per message,
// terminated by '\n'. The host is authoritative — it runs the GameController and
// broadcasts already-formatted view events; clients only render and reply moves.
//
// Direction legend:  C→H client to host,  H→C host to client.
namespace Protocol {

// Top-level "type" tags.
namespace Type {
inline constexpr char Hello[]         = "hello";          // C→H {name}
inline constexpr char Move[]          = "move";           // C→H {move:"r"|"p"|"s"}
inline constexpr char Welcome[]       = "welcome";        // H→C {id, roster[]}
inline constexpr char Lobby[]         = "lobby";          // H→C {roster[]}
inline constexpr char GameStarted[]   = "game_started";   // H→C {}
inline constexpr char YourTurn[]      = "your_turn";      // H→C {}
inline constexpr char Log[]           = "log";            // H→C {text}
inline constexpr char Scores[]        = "scores";         // H→C {names[], scores[]}
inline constexpr char RoundFinished[] = "round_finished"; // H→C {}
inline constexpr char GameFinished[]  = "game_finished";  // H→C {lines[]}
} // namespace Type

// Framing.
QByteArray encode(const QJsonObject& msg);                  // -> compact JSON + '\n'
std::optional<QJsonObject> decode(const QByteArray& line);  // parse one (un-framed) message
QString typeOf(const QJsonObject& msg);                     // value of "type", or empty

// Builders — C→H
QJsonObject hello(const QString& name);
QJsonObject move(const QString& move);

// Builders — H→C
QJsonObject welcome(int id, const QStringList& roster);
QJsonObject lobby(const QStringList& roster);
QJsonObject gameStarted();
QJsonObject yourTurn();
QJsonObject log(const QString& text);
QJsonObject scores(const QStringList& names, const QStringList& scores);
QJsonObject roundFinished();
QJsonObject gameFinished(const QStringList& lines);

// Buffers raw TCP bytes and yields complete '\n'-delimited messages as they
// arrive (TCP gives no message boundaries, so reads may split or coalesce).
class FrameReader {
public:
    void append(const QByteArray& bytes);
    // Returns the next fully-received, successfully-decoded message, or nullopt
    // when the buffer holds no further complete line. Malformed lines are skipped.
    std::optional<QJsonObject> next();

private:
    QByteArray mBuffer;
};

} // namespace Protocol
