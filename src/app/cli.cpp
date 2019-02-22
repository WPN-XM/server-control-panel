#include "cli.h"
#include "windowsapi.h"

namespace ServerControlPanel
{

    void CLI::handleCommandLineArguments()
    {
        // lets support multiple cli options, each with different options
        // this should handle, e.g. "wpn-xm.exe --service nginx start"

        QCommandLineParser parser;

        /**
         * Definition of Command Line Arguments
         */

        // -h, --help, -?
        QCommandLineOption helpOption(QStringList() << "h"
                                                    << "help"
                                                    << "?",
                                      "Prints this help message.");
        parser.addOption(helpOption);

        // -v, --version
        QCommandLineOption versionOption(QStringList() << "v"
                                                       << "version",
                                         "Display the version.");
        parser.addOption(versionOption);

        // -s, --service
        // TODO: install / uninstall server as service from CLI (part1)
        // QCommandLineOption serviceOption(QStringList() << "s" << "service",
        // "Install/Uninstall server as service.", "[server] [command]");
        // parser.addOption(serviceOption);

        // -srv, --server
        QCommandLineOption serverOption(QStringList() << "srv"
                                                      << "server",
                                        "Execute a command on server.",
                                        "[server] [command]");
        parser.addOption(serverOption);

        // --start
        QCommandLineOption startOption("start", "Starts a server.", "[server/s]");
        parser.addOption(startOption);

        // --stop
        QCommandLineOption stopOption("stop", "Stops a server.", "[server/s]");
        parser.addOption(stopOption);

        // --restart
        QCommandLineOption restartOption("stop", "Restarts a server.", "[server/s]");
        parser.addOption(restartOption);

        /**
         * Handling of Command Line Arguments
         */

        // find out the positional arguments.
        parser.parse(QCoreApplication::arguments());
        const QStringList args = parser.positionalArguments();
        const QString command  = args.isEmpty() ? QString() : args.first();

        // -h, --help, -?
        if (parser.isSet(helpOption)) {
            printHelpText();
        }

        // -v, --version
        if (parser.isSet(versionOption)) {
            colorPrint("WPN-XM Server Stack - Server Control Panel " APP_VERSION "\n", "brightwhite");
            exit(0);
        }

        // -s, --service <server> <command>, where <command> is on|off
        // if (parser.isSet(serviceOption)) {
        // TODO: install / uninstall server as service from CLI (part2)
        // https://github.com/WPN-XM/WPN-XM/issues/39
        //}

        // -d, --server <server> <command>, where <command> is start|stop|restart
        if (parser.isSet(serverOption)) {

            // at this point we already have "--server <server>", but not <command>//

            // server is the value
            QString server = parser.value(serverOption);

            if (server.isEmpty()) {
                printHelpText(QString("Error: no <server> specified."));
            }

            Servers::Servers *servers = new Servers::Servers();

            // check if server is whitelisted
            if (!servers->getListOfServerNames().contains(server)) {
                printHelpText(QString("Error: \"%1\" is not a valid <server>.").arg(server.toLocal8Bit().constData()));
            }

            if (command.isEmpty()) {
                printHelpText(QString("Error: no <command> specified."));
            }

            QStringList availableCommands = QStringList() << "start"
                                                          << "stop"
                                                          << "restart";
            if (!availableCommands.contains(command)) {
                printHelpText(
                    QString("Error: \"%1\" is not a valid <command>.").arg(command.toLocal8Bit().constData()));
            }

            QString methodName = command + servers->getCamelCasedServerName(server);

            if (QMetaObject::invokeMethod(servers, methodName.toLocal8Bit().constData())) {
                exit(0);
            }

            printHelpText(QString("Command not handled, yet! (server = %1) (command = %2) \n")
                              .arg(server.toLocal8Bit().constData(), command.toLocal8Bit().constData()));
            exit(0);
        }

        // --start <servers>
        if (parser.isSet(startOption)) {
            execServers("start", startOption, args, parser);
        }

        // --stop <servers>
        if (parser.isSet(stopOption)) {
            execServers("stop", stopOption, args, parser);
        }

        // --restart <servers>
        if (parser.isSet(restartOption)) {
            execServers("stop", restartOption, args, parser);
        }

        // if(parser.unknownOptionNames().count() > 1) {
        printHelpText(QString("Error: Unknown option."));
        //}
    }

    /**
     * @brief execServers - executes "command" on multiple Servers
     * @param command "start", "stop", "restart"
     * @param clioption
     * @param args
     */
    [[noreturn]] void CLI::execServers(const QString &command,
                                       QCommandLineOption &clioption,
                                       QStringList args,
                                       QCommandLineParser &parser)
    {
        // the value of the key "--start|--stop|--restart" is the first server
        QString server = parser.value(clioption);

        if (server.isEmpty()) {
            printHelpText(QString("Error: no <server> specified."));
        }

        // add first server to the list
        QStringList serversList;
        serversList << server;

        // add the others args
        if (!args.isEmpty()) {
            serversList << args;
        }

        Servers::Servers *servers = new Servers::Servers();

        for (int i = 0; i < serversList.size(); ++i) {
            QString server = serversList.at(i);

            // check if whitelisted
            if (!servers->getListOfServerNames().contains(server)) {
                printHelpText(QString("Error: \"%1\" is not a valid <server>.").arg(server.toLocal8Bit().constData()));
            }

            QString methodName = command + servers->getCamelCasedServerName(server);

            QMetaObject::invokeMethod(servers, methodName.toLocal8Bit().constData());
        }

        exit(0);
    }

    [[noreturn]] void CLI::printHelpText(QString errorMessage)
    {
        colorPrint("WPN-XM Server Stack - Server Control Panel " APP_VERSION "\n", "brightwhite");

        QString year = QDate::currentDate().toString("yyyy");
        colorPrint("Copyright (c) " + year + " Jens A. Koch. All rights reserved.\n\n");

        if (!errorMessage.isEmpty()) {
            colorPrint(errorMessage.append("\n\n").toLocal8Bit().constData(), "red");
        }

        colorPrint("Usage: ", "green");
        QString usage = QCoreApplication::arguments().at(0) + " [option] [args] \n\n";
        colorPrint(usage);

        colorPrint("Options: \n", "green");
        QString options =
            "  -v, --version                        Prints the version. \n"
            "  -h, --help                           Prints this help message. \n"
            "  -d, --server <server> <command>      Executes <command> on <server>. "
            "\n"
            "      --start <servers>                Starts one or more <servers>. \n"
            "      --stop <servers>                 Stops one or more <servers>. \n"
            "      --restart <servers>              Restarts one or more <servers>. "
            "\n\n";
        colorPrint(options);

        colorPrint("Arguments: \n", "green");
        QString arguments =
            "  <server>:  The name of a server, e.g. nginx, mariadb, memcached, "
            "mongodb. \n"
            "  <command>: The command to execute, e.g. start, stop, restart. \n\n";
        colorPrint(arguments);

        colorPrint("Examples: \n", "green");
        QString example = "  " + QCoreApplication::arguments().at(0) +
                          " --server nginx start \n"
                          "  " +
                          QCoreApplication::arguments().at(0) + " --start nginx php mariadb \n\n";
        colorPrint(example);

        colorPrint("Info: \n", "green");
        QString info = "  Ports specified in \"wpn-xm.ini\" will be used. \n";
        colorPrint(info);

        exit(0);
    }

    // sorry, i don't like color numbers
    void CLI::colorPrint(QString msg, QString colorName)
    {
        int prefix = 0;
        int color  = 0;

        if (colorName == "black") {
            prefix = 0;
            color  = 0;
        }
        if (colorName == "darkred") {
            prefix = 0;
            color  = 31;
        }
        if (colorName == "green") {
            prefix = 0;
            color  = 32;
        }
        if (colorName == "yellow") {
            prefix = 0;
            color  = 33;
        }
        if (colorName == "blue") {
            prefix = 0;
            color  = 34;
        }
        if (colorName == "magenta") {
            prefix = 0;
            color  = 35;
        }
        if (colorName == "cyan") {
            prefix = 0;
            color  = 36;
        }
        if (colorName == "white") {
            prefix = 0;
            color  = 37;
        }
        if (colorName == "gray") {
            prefix = 0;
            color  = 37;
        } // white = gray

        if (colorName == "red") {
            prefix = 1;
            color  = 31;
        }
        if (colorName == "lightgreen") {
            prefix = 1;
            color  = 32;
        }
        if (colorName == "lightyellow") {
            prefix = 1;
            color  = 33;
        }
        if (colorName == "lightblue") {
            prefix = 1;
            color  = 34;
        }
        if (colorName == "lightmagenta") {
            prefix = 1;
            color  = 35;
        }
        if (colorName == "lightcyan") {
            prefix = 1;
            color  = 36;
        }
        if (colorName == "brightwhite") {
            prefix = 1;
            color  = 37;
        }

        WindowsAPI::Console::printColoredMsg(prefix, color, msg.toLocal8Bit().constData());
    }

    void CLI::colorTest()
    {
        printf("%s/n", WindowsAPI::Console::printColoredMsg(1, 31, "1-31"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(0, 31, "0-31"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(1, 32, "1-33"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(0, 32, "0-33"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(1, 33, "1-33"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(0, 33, "0-33"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(1, 34, "1-34"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(0, 34, "0-34"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(1, 35, "1-35"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(0, 35, "0-35"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(1, 36, "1-35"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(0, 36, "0-36"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(1, 37, "1-37"));
        printf("%s/n", WindowsAPI::Console::printColoredMsg(0, 37, "0-37"));
    }
} // namespace ServerControlPanel
