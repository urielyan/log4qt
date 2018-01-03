#ifndef LOG4QTDEFS_H
#define LOG4QTDEFS_H

#include <QtCore/QtGlobal>
#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QThread>
#include <functional>
#include <vector>
#include <cstdarg>

#if defined(LOG4QT_LIBRARY)
#  define LOG4QTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LOG4QTSHARED_EXPORT Q_DECL_IMPORT
#endif

namespace log4qt {
namespace impl {
struct LogMessage;

// patterns for formating log message
static const QString PatternDateTime = QStringLiteral("%{datetime}");
static const QString PatternType = QStringLiteral("%{type}");
static const QString PatternLevel = QStringLiteral("%{level}");
static const QString PatternPID = QStringLiteral("%{pid}");
static const QString PatternThreadId = QStringLiteral("%{threadid}");
static const QString PatternThreadPtr = QStringLiteral("%{qthreadptr}");
static const QString PatternFile = QStringLiteral("%{file}");
static const QString PatternLine = QStringLiteral("%{line}");
static const QString PatternFunction = QStringLiteral("%{function}");
static const QString PatternMessage = QStringLiteral("%{message}");
static const QString DefaultPattern = QStringLiteral("%1 %2(%3) %4:%5 %6:%7 %8 - %9")
                                      .arg(PatternDateTime)
                                      .arg(PatternType)
                                      .arg(PatternLevel)
                                      .arg(PatternPID)
                                      .arg(PatternThreadId)
                                      .arg(PatternFile)
                                      .arg(PatternLine)
                                      .arg(PatternFunction)
                                      .arg(PatternMessage);
static const QString PlaceHolderDateTime = QStringLiteral("%{D}");
static const QString PlaceHolderType = QStringLiteral("%{T}");
static const QString PlaceHolderLevel = QStringLiteral("%{V}");
static const QString PlaceHolderPID = QStringLiteral("%{P}");
static const QString PlaceHolderThreadId = QStringLiteral("%{I}");
static const QString PlaceHolderThreadPtr = QStringLiteral("%{H}");
static const QString PlaceHolderFile = QStringLiteral("%{F}");
static const QString PlaceHolderLine = QStringLiteral("%{L}");
static const QString PlaceHolderFunction = QStringLiteral("%{N}");
static const QString PlaceHolderMessage = QStringLiteral("%{M}");
// parse the message pattern, refactor to internal syntax and return it
LOG4QTSHARED_EXPORT QString& parsePattern(QString& pattern);
// combine LogMessage into return text base on parsed pattern
LOG4QTSHARED_EXPORT QString processMessage(const QString& parsedPattern, const QSharedPointer<LogMessage>& message);

// log type, calculated by LogLevel
enum LogType
{
    Debug = 0,
    Infomation = 25,
    Warning = 50,
    Critical = 75,
    Fatal = 100
};
static const int DefaultFilter = LogType::Infomation;
// get LogType by input log level
LOG4QTSHARED_EXPORT LogType getLogType(int level);
// get printable description string for LogType
LOG4QTSHARED_EXPORT QString getLogTypeString(LogType type);

// log message
struct LogMessage
{
    QDateTime dateTime;
    LogType type;
    int level;
    qint64 pid;
    Qt::HANDLE threadid;
    QThread* threadptr;
    QString file;
    int line;
    QString function;
    QSharedPointer<QString> message;
    LogMessage(int level, QString file, int line, QString function);
};

class LogStream;
struct ILogProcessorBase;
// interface for log engine
namespace LogEngine
{
    // generate LogStream just like QMessageLogger
    LOG4QTSHARED_EXPORT LogStream log(int logLevel, const QString& file, int line, const QString& function);

    // register log processor, pushing log messages using connectionType
    // WARNING
    //     When processor and Logger are in the same thread,
    //     Qt::BlockingQueuedConnection will be replaced by Qt::DirectConnection to prevent deadlock.
    LOG4QTSHARED_EXPORT bool registerProcessor(ILogProcessorBase* processor, Qt::ConnectionType connectionType);

    // unregister log processor
    LOG4QTSHARED_EXPORT void unRegisterProcessor(ILogProcessorBase* processor);
} // namespace LogEngine

// Interface for log processor
struct ILogProcessorBase
{
    virtual ~ILogProcessorBase() { LogEngine::unRegisterProcessor(this); }

    // callback for pushing log message
    // must explicitly declared as Q_SLOT
    virtual void log(const QSharedPointer<LogMessage> message) = 0;
};
template<class T, class U>
struct ILogProcessorBaseWrapper : public ILogProcessorBase, public T
{
    explicit ILogProcessorBaseWrapper(U* parent = 0) : T(parent) {}
    virtual ~ILogProcessorBaseWrapper() {}
};
using ILogProcessor = ILogProcessorBaseWrapper<QObject, QObject>;
template<typename T>
using ILogProcessorWidget = ILogProcessorBaseWrapper<T, QWidget>;

// log output stream, has the same feature as QDebug
class LOG4QTSHARED_EXPORT LogStream : public QDebug
{
public:
    LogStream(QSharedPointer<LogMessage> msg, std::function<void(QSharedPointer<LogMessage>)> onDel);
    ~LogStream();

    // C++ style output interface, equivalent to "QDebug() << ..."
    // will call noquote() when Qt version >= 5.4.0
    LogStream& log();

    // C style output interface, equivalent to "qDebug(format, ...)"
    void log(const char* format, ...) Q_ATTRIBUTE_FORMAT_PRINTF(2, 3);

private:
    // log message
    QSharedPointer<LogMessage> message;
    // callback on destruction, pushing message to LogEngine
    std::function<void(QSharedPointer<LogMessage>)> onDelete;
};

} // namesapce impl
} // namespace log4qt

Q_DECLARE_METATYPE(QSharedPointer<log4qt::impl::LogMessage>)

#endif // LOG4QTDEFS_H