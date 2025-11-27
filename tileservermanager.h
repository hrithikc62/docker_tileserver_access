#ifndef TILESERVERMANAGER_H
#define TILESERVERMANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QDebug>
#include <QTcpServer>
#include <QThread>

class TileserverManager : public QObject
{
    Q_OBJECT
public:
    explicit TileserverManager(QObject *parent = nullptr);
    ~TileserverManager();


    bool startTileserver(const QString &datapath,int port);
    bool stopTileserver(int port);

    bool isRunning() const;


signals:

    void outputReceived(QString const &output);
    void errorReceived(const QString &error);
    void processFinished(int exitCode);

private slots:

    void handleReadyReadStandardOutput();
    void handleReadyReadStandardError();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    QString runDockerCommand(const QStringList &args,bool wait =true);
    bool isContainerRunning(const QString &name);
    QString getContainerNameByPort(int port);
    bool stopContainer(const QString &idOrName);
    bool startContainer(const QString &name);
    bool createContainer(const QString &name,const QString &datapath,int port);


    const QString containerName = "tileserver";
    const QString dockerProgram = "wsl";
    QProcess *m_process=nullptr;
};

#endif // TILESERVERMANAGER_H
