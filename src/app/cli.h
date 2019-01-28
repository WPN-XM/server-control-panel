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
                         QStringList args,
                         QCommandLineParser &parser);
        void colorTest();
        void colorPrint(QString msg, QString colorName = "gray");
    };
} // namespace ServerControlPanel

#endif // CLI_H
