#include "Protocol.h"
#include <QJsonArray>
#include <QJsonDocument>

namespace Protocol {

namespace {
QJsonArray toArray(const QStringList& items)
{
    QJsonArray arr;
    for (const QString& s : items) arr.append(s);
    return arr;
}
} // namespace

QByteArray encode(const QJsonObject& msg)
{
    return QJsonDocument(msg).toJson(QJsonDocument::Compact) + '\n';
}

std::optional<QJsonObject> decode(const QByteArray& line)
{
    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(line, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return std::nullopt;
    return doc.object();
}

QString typeOf(const QJsonObject& msg)
{
    return msg.value(QStringLiteral("type")).toString();
}

QJsonObject hello(const QString& name)
{
    return { { QStringLiteral("type"), QString::fromLatin1(Type::Hello) },
             { QStringLiteral("name"), name } };
}

QJsonObject move(const QString& move)
{
    return { { QStringLiteral("type"), QString::fromLatin1(Type::Move) },
             { QStringLiteral("move"), move } };
}

QJsonObject welcome(int id, const QStringList& roster)
{
    return { { QStringLiteral("type"), QString::fromLatin1(Type::Welcome) },
             { QStringLiteral("id"), id },
             { QStringLiteral("roster"), toArray(roster) } };
}

QJsonObject lobby(const QStringList& roster)
{
    return { { QStringLiteral("type"), QString::fromLatin1(Type::Lobby) },
             { QStringLiteral("roster"), toArray(roster) } };
}

QJsonObject gameStarted()
{
    return { { QStringLiteral("type"), QString::fromLatin1(Type::GameStarted) } };
}

QJsonObject yourTurn()
{
    return { { QStringLiteral("type"), QString::fromLatin1(Type::YourTurn) } };
}

QJsonObject log(const QString& text)
{
    return { { QStringLiteral("type"), QString::fromLatin1(Type::Log) },
             { QStringLiteral("text"), text } };
}

QJsonObject scores(const QStringList& names, const QStringList& scores)
{
    return { { QStringLiteral("type"), QString::fromLatin1(Type::Scores) },
             { QStringLiteral("names"), toArray(names) },
             { QStringLiteral("scores"), toArray(scores) } };
}

QJsonObject roundFinished()
{
    return { { QStringLiteral("type"), QString::fromLatin1(Type::RoundFinished) } };
}

QJsonObject gameFinished(const QStringList& lines)
{
    return { { QStringLiteral("type"), QString::fromLatin1(Type::GameFinished) },
             { QStringLiteral("lines"), toArray(lines) } };
}

void FrameReader::append(const QByteArray& bytes)
{
    mBuffer += bytes;
}

std::optional<QJsonObject> FrameReader::next()
{
    for (;;) {
        const int nl = mBuffer.indexOf('\n');
        if (nl < 0)
            return std::nullopt;                 // no complete line yet

        const QByteArray line = mBuffer.left(nl);
        mBuffer.remove(0, nl + 1);

        if (line.trimmed().isEmpty())
            continue;                            // tolerate blank lines

        if (auto msg = decode(line))
            return msg;
        // Malformed line: drop it and try the next one.
    }
}

} // namespace Protocol
