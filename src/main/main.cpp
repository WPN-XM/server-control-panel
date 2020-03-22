#include "main.h"

int main(int argc, char *argv[])
{
    // Q_INIT_RESOURCE(resources);

    /*
     * On Windows an application is either a GUI application or Console application.
     * This application is a rare GUI and CLI application hybrid.
     *
     * The application is compiled with "CONFIG += CONSOLE" to support the native console mode.
     * When the app is executed without command line args, it will start a console in GUI mode.
     *
     * If CLI args are found, the application reacts as a console application.
     * In this case QCoreApplication is used instead of QApplication to
     * avoid unnecessarily initializing resources.
     *
     * Else, it runs as a normal QtGUI application.
     */
    if (argc > 1) { // first arg is the executable itself
        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName(APP_NAME);
        QCoreApplication::setApplicationVersion(APP_VERSION);

        auto *cli = new ServerControlPanel::CLI;
        cli->handleCommandLineArguments();

        return QCoreApplication::exec();
    }

    // HighDpi support
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Initialize Qt application
    QApplication app(argc, argv);

    /*
     * By calling FreeConsole(), we close this console immediately after starting the GUI.
     * That results in a short console flickering.
     *
     * Yes, it's annoying - but feel free to contribute a better solution.
     * Rules: one executable; not embedded exe; not the devenv.com solution.
     * Ok, now its your turn... :)
     *
     * For more information, see:
     * https://github.com/WPN-XM/WPN-XM/issues/39
     */
    FreeConsole();

    // Single Instance Check
    ServerControlPanel::Main::exitIfAlreadyRunning();

    // Executable Placement Check
    ServerControlPanel::Main::exitIfNotInAppFolder();

    // Setup Translator for Localization
    /*QTranslator translator;
  QString locale = QLocale::system().name(); // locale = "de_DE";
  qDebug() << "Translator uses System Locale:" << locale;
  translator.load(QString(":languages/lang_") + locale + QString(".qm"));
  app.installTranslator(&translator);
  app.setLayoutDirection(QObject::tr("LTR") == "RTL" ? Qt::RightToLeft :
  Qt::LeftToRight);*/

    // Application Meta Data and Settings
    QApplication::setApplicationName(APP_NAME);
    QApplication::setApplicationVersion(APP_VERSION);
    QApplication::setOrganizationName("Jens A. Koch");
    QApplication::setOrganizationDomain("https://wpn-xm.org/");
    QApplication::setWindowIcon(QIcon(":/wpnxm.ico"));

    // do not leave application, until Quit is clicked in the tray menu
    QApplication::setQuitOnLastWindowClosed(false);

    /**
     * TODO application style / dark style
     * @brief Set Style: "Fusion Dark"
     */
    // app.setStyle(QStyleFactory::create("Fusion"));
    /*
  QPalette p;
  p = app.palette();
  p.setColor(QPalette::Window, QColor(53,53,53));
  p.setColor(QPalette::Button, QColor(53,53,53));
  p.setColor(QPalette::Highlight, QColor(142,45,197));
  p.setColor(QPalette::ButtonText, QColor(255,255,255));
  app.setPalette(p);*/

    // Initialize Application MainWindow
    // So that we can inject data. But do not show it, yet.
    ServerControlPanel::MainWindow mainWindow;

    // Activate SplashScreen
    ServerControlPanel::SplashScreen splash(QPixmap(), Qt::WindowStaysOnTopHint);
    splash.setProgress(0);
    splash.show();

    splash.setMessage("Loading Settings...", 5);
    Settings::SettingsManager *settings = mainWindow.getSettings();

    splash.setMessage("Loading Plugins...", 10);
    QApplication::addLibraryPath("./plugins");
    auto *plugins = new PluginsNS::Plugins();
    plugins->loadPlugins();
    mainWindow.setPlugins(plugins);

    splash.setMessage("Initial scan of installed applications ..", 15);
    QThread::msleep(300);

    splash.setMessage("Getting System Processes ..", 30);
    Processes::ProcessUtil *processes = Processes::ProcessUtil::getInstance();
    mainWindow.setProcessUtil(processes);

    splash.setMessage("Searching for already running processes and blocked ports ..", 40);
    if (Processes::ProcessUtil::getAlreadyRunningProcesses()) {
        // this fills monitoredProcessesList
        splash.hide();
        // displayShutdownAlreadyRunningProcessesOrContinueDialog
        /*AlreadyRunningProcessesDialog *arpd = new AlreadyRunningProcessesDialog();
        arpd->checkAlreadyRunningServers();*/
        /*ProcessViewerDialog *pvd = new ProcessViewerDialog();
        pvd->setWindowTitle("Process Viewer - Already Running Processes");
        pvd->setChecked_ShowOnlyWpnxmProcesses();
        pvd->setProcessesInstance(processes);
        pvd->exec();*/
        splash.show();
    }

    splash.setProgress(50);
    QThread::msleep(300);

    // Setup the environment: trayicon, actions, servers, process monitoring
    mainWindow.setup();

    // Finally, show the MainWindow
    if (!settings->getBool("global/startminimized")) {
        mainWindow.show();
    }

    // be nice and jump to 100% on the progress bar :P
    splash.setMessage("", 100);

    // Deactivate SplashScreen
    splash.finish(&mainWindow);

    // enter the Qt Event loop here
    return QApplication::exec();
}

namespace ServerControlPanel
{
    /*
     * Single Instance Check
     *
     * Although some people tend to solve this problem by using QtSingleApplication,
     * this approach uses a GUID stored into shared memory.
     *
     * The GUID needs to be "static", because the QSharedMemory instance gets
     * destroyed
     * at the end of the function and so does the shared memory segment.
     */
    void Main::exitIfAlreadyRunning()
    {
        static QSharedMemory shared("004d54f6-7d00-4478-b612-f242f081b023");

        // already running
        if (!shared.create(64, QSharedMemory::ReadWrite)) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(APP_NAME);
            msgBox.setText(QObject::tr("WPN-XM is already running."));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();

            exit(0);
        }
    }

    /*
     * Check Placement of Executable
     *
     * Ensure that "wpn-xm.exe" is placed in the application root folder.
     * Disallow starts outside the wpn-xm installation folder (e.g. from the desktop).
     */
    void Main::exitIfNotInAppFolder()
    {
        QString appDir = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());

        // handle development environment
        // After a build the executable resides in the "build-*-Release|Debug" folder.
        // It is started via QtCreator using the work directory directive, which points to a server stack folder.
        if (appDir.contains("build-")) {
            return;
        }

        QString phpFolder = appDir + "/bin/php";

        if (!QDir(phpFolder).exists()) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(APP_NAME);
            msgBox.setText("The WPN-XM executable must be placed in the root folder of the server stack.");
            msgBox.setDetailedText("[Error] The directory that currently contains the executable is: \n" % appDir);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();

            exit(0);
        }
    }
} // namespace ServerControlPanel
