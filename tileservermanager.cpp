#include "tileservermanager.h"

TileserverManager::TileserverManager(QObject *parent)
    : QObject{parent},
    m_process(new QProcess(this))
{
  connect(m_process, &QProcess::readyReadStandardOutput, this,
          &TileserverManager::handleReadyReadStandardOutput);
  connect(m_process, &QProcess::readyReadStandardError, this,
          &TileserverManager::handleReadyReadStandardError);
  connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
          &TileserverManager::handleProcessFinished);


}
TileserverManager::~TileserverManager()
{
    stopTileserver(8082);
}


QString TileserverManager::runDockerCommand(const QStringList &args, bool wait)
{
    QProcess proc;
    proc.start(dockerProgram, args);
    if (wait) proc.waitForFinished();
    return QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
}

bool TileserverManager::isContainerRunning(const QString &name)
{
    QString state = runDockerCommand({"/usr/bin/docker", "inspect", "-f", "{{.State.Running}}", name});
    return state == "true";
}
QString TileserverManager::getContainerNameByPort(int port)
{
    QString output = runDockerCommand({"/usr/bin/docker", "ps", "--format", "{{.ID}} {{.Ports}}"});
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : std::as_const(lines)) {
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() >= 2 && parts[1].contains(QString("0.0.0.0:%1->8082").arg(port))) {
            QString id = parts[0];
            QString name = runDockerCommand({"/usr/bin/docker", "inspect", "-f", "{{.Name}}", id});
            return name.startsWith('/') ? name.mid(1) : name;
        }
    }
    return QString();
}

bool TileserverManager::stopContainer(const QString &idOrName)
{
    QString result = runDockerCommand({"/usr/bin/docker", "stop", idOrName});
    if (!result.isEmpty()) {
        qInfo() << "Stopped container:" << result;
        return true;
    }
    return false;
}
bool TileserverManager::startContainer(const QString &name)
{
    QString result = runDockerCommand({"/usr/bin/docker", "start", name});
    if (!result.isEmpty()) {
        qInfo() << QString("Started container '%1'. Access at: http://localhost:8082").arg(name);
        return true;
    }
    return false;
}

bool TileserverManager::createContainer(const QString &name, const QString &dataPath, int port)
{
    QStringList args;
    args << "/usr/bin/docker"
         << "run"
         << "--name" << name
         << "-p" << QString("%1:8080").arg(port)
         << "-v" << QString("%1:/data").arg(dataPath)
         << "-d"
         << "maptiler/tileserver-gl";

    m_process->start(dockerProgram, args);
    if (!m_process->waitForStarted()) {
        QByteArray errorOutput = m_process->readAllStandardError();
        qCritical() << "Failed to start new container!" << errorOutput;
        return false;
    }
    qInfo() << QString("Created and started new container '%1' at http://localhost:%2").arg(name).arg(port);
    return true;
}

bool TileserverManager::startTileserver(const QString &dataPath, int port)
{
    // Check if container exists
    QString existingContainer = getContainerNameByPort(port);
    if (!existingContainer.isEmpty() && existingContainer != containerName) {
        stopContainer(existingContainer);
        runDockerCommand({"/usr/bin/docker", "rename", existingContainer, containerName});
    }

    // Start existing container
    if (isContainerRunning(containerName)) {
        qInfo() << QString("Tileserver already running at http://localhost:%1").arg(port);
        return true;
    }

    // Start or create container
    if (runDockerCommand({"/usr/bin/docker", "ps", "-a", "-q", "--filter", "name=" + containerName}).isEmpty()) {
        return createContainer(containerName, dataPath, port);
    }

    return startContainer(containerName);
}

bool TileserverManager::stopTileserver(int port)
{
    QString containerName = "tileserver";

    if (isContainerRunning(containerName)) {
        QProcess stopProcess;
        stopProcess.start(dockerProgram, {"/usr/bin/docker", "stop", containerName});
        stopProcess.waitForFinished();
        qInfo() << QString("Stopped tileserver container '%1' using port %2").arg(containerName).arg(port);
        return true;
    }

    qInfo() << QString("No running tileserver container found at port %1").arg(port);
    return false;
}
bool TileserverManager::isRunning() const
{
     return m_process->state() == QProcess::Running;
}



void TileserverManager::handleReadyReadStandardOutput()
{
    QString output = m_process->readAllStandardOutput();
    emit outputReceived(output);
    qDebug() << "Tileserver Docker Output:" << output;
}

void TileserverManager::handleReadyReadStandardError()
{
    QString error = m_process->readAllStandardError();
    emit errorReceived(error);
    qWarning() << "Tileserver Docker Error:" << error;
}

void TileserverManager::handleProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(status);
    emit processFinished(exitCode);
    qInfo() << "Tileserver Docker process finished with exit code:" << exitCode;
}







