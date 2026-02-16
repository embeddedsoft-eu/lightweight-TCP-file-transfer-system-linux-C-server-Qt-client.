#include "FileSender.h"

FileSender::FileSender(QObject *parent) : QObject(parent)
{
}

bool FileSender::sendFile(const QString &filePath, 
                          const QString &serverHost, 
                          quint16 serverPort)
{
    QTcpSocket socket(this);
    QFile file(filePath);
    qint64 totalBytes = 0;
    qint64 bytesSent = 0;
    
    // Validate file
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        m_lastError = QString("File does not exist: %1").arg(filePath);
        qDebug() << "Error:" << m_lastError;
        emit finished(false, m_lastError);
        return false;
    }
    
    totalBytes = fileInfo.size();
    
    // Open file
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Cannot open file: %1 - %2")
                     .arg(filePath).arg(file.errorString());
        qDebug() << "Error:" << m_lastError;
        emit finished(false, m_lastError);
        return false;
    }
    
    qDebug() << "Sending file:" << fileInfo.fileName() << "Size:" << totalBytes << "bytes";
    
    // Connect to server
    qDebug() << "Connecting to" << serverHost << ":" << serverPort;
    socket.connectToHost(serverHost, serverPort);
    
    if (!socket.waitForConnected(CONNECT_TIMEOUT)) {
        m_lastError = QString("Connection failed: %1")
                     .arg(socket.errorString());
        qDebug() << "Error:" << m_lastError;
        file.close();
        emit finished(false, m_lastError);
        return false;
    }
    
    qDebug() << "Connected successfully";
    
    // Send filename (terminated with newline)
    QByteArray fileNameData = fileInfo.fileName().toUtf8() + "\n";
    socket.write(fileNameData);
    
    if (!socket.waitForBytesWritten(WRITE_TIMEOUT)) {
        m_lastError = QString("Failed to send filename: %1")
                     .arg(socket.errorString());
        qDebug() << "Error:" << m_lastError;
        file.close();
        socket.disconnectFromHost();
        emit finished(false, m_lastError);
        return false;
    }
    
    // Send file data in chunks
    char buffer[CHUNK_SIZE];
    qint64 bytesRead;
    
    while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0) {
        qint64 bytesWritten = socket.write(buffer, bytesRead);
        
        if (bytesWritten != bytesRead) {
            m_lastError = QString("Write error: expected %1 bytes, wrote %2")
                         .arg(bytesRead).arg(bytesWritten);
            qDebug() << "Error:" << m_lastError;
            file.close();
            socket.disconnectFromHost();
            emit finished(false, m_lastError);
            return false;
        }
        
        if (!socket.waitForBytesWritten(WRITE_TIMEOUT)) {
            m_lastError = QString("Timeout writing data: %1")
                         .arg(socket.errorString());
            qDebug() << "Error:" << m_lastError;
            file.close();
            socket.disconnectFromHost();
            emit finished(false, m_lastError);
            return false;
        }
        
        bytesSent += bytesWritten;
        emit progressChanged(bytesSent, totalBytes);
        qDebug() << "Progress:" << bytesSent << "/" << totalBytes;
    }
    
    // Clean up
    file.close();
    socket.disconnectFromHost();
    
    if (socket.state() != QAbstractSocket::UnconnectedState) {
        socket.waitForDisconnected(3000);
    }
    
    qDebug() << "File sent successfully:" << fileInfo.fileName();
    emit finished(true, QString());
    return true;
}
