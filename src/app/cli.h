#ifndef CLI_H
#define CLI_H

#include "servers.h"
#include "version.h"
#include "Windows.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDate>

namespace ServerControlPanel
{
    class CLI
    {
    public:
        void handleCommandLineArguments();
        void printHelpText(QString errorMessage = QString());
        void execServers(const QString &command,
                         QCommandLineOption &clioption,
                         const QStringList &args,
                         QCommandLineParser &parser);
        void colorPrint(const QString &msg, const QString &colorName = "gray");

        // void colorTest();
    };
} // namespace ServerControlPanel

#endif // CLI_H
