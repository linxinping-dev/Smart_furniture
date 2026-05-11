#include <QGuiApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QFile>
#include <QTextStream>

#include "HomeState.h"
#include "TcpClient.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    HomeState homeState;
    TcpClient tcpClient;
    tcpClient.setHomeState(&homeState);
    tcpClient.setServerAddress(QStringLiteral("172.20.10.4"), 8080);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("homeState"), &homeState);
    engine.rootContext()->setContextProperty(QStringLiteral("tcpClient"), &tcpClient);

    // qml.qrc 中的资源路径是 qrc:/qml/main.qml
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));

    QStringList qmlErrors;
    // Collect QML load warnings/errors so we can diagnose missing modules/plugins.
    QObject::connect(
        &engine, &QQmlApplicationEngine::warnings, &app,
        [&](const QList<QQmlError>& warnings) {
            for (const auto& w : warnings) {
                const QString line = QStringLiteral("%1: %2")
                                         .arg(w.url().toString())
                                         .arg(w.description());
                qmlErrors.push_back(line);
                qWarning().noquote() << "QML Warning:" << w.url().toString() << w.description();
            }
        });

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url, &qmlErrors](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl) {
                // Some QML issues happen in a windows-subsystem app and might not show console output.
                // Persist the error to a file so you can inspect it.
                QFile f(QStringLiteral("qml_load_error.txt"));
                if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                    QTextStream ts(&f);
                    ts << "Failed to load QML:\n";
                    ts << "url=" << url.toString() << "\n";
                    if (!qmlErrors.isEmpty()) {
                        ts << "\nWarnings/Errors:\n";
                        for (const auto& line : qmlErrors) {
                            ts << "- " << line << "\n";
                        }
                    }
                }
                qCritical().noquote() << "Failed to load QML. See qml_load_error.txt.";
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "QML root object is empty after load:" << url.toString();
    }

    return app.exec();
}

