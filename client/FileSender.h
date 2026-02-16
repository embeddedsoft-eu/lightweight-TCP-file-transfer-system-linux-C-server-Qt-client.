#ifndef FILESENDER_H
#define FILESENDER_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

/**
 * FileSender class for sending files to the TCP file server
 * 
 * Example usage:
 *   FileSender sender;
 *   sender.sendFile("/path/to/file.txt", "embeddedsoft.eu", 4445);
 */
class FileSender : public QObject
{
    Q_OBJECT

public:
    explicit FileSender(QObject *parent = nullptr);
    
    /**
     * Send a file to the server
     * @param filePath Full path to the file to send
     * @param serverHost Server hostname or IP address
     * @param serverPort Server port number
     * @return true if file was sent successfully, false otherwise
     */
    bool sendFile(const QString &filePath, 
                  const QString &serverHost, 
                  quint16 serverPort);

signals:
    /**
     * Emitted when progress changes
     * @param bytesSent Number of bytes sent so far
     * @param totalBytes Total file size
     */
    void progressChanged(qint64 bytesSent, qint64 totalBytes);
    
    /**
     * Emitted when operation completes
     * @param success true if successful, false otherwise
     * @param errorMessage Error message if not successful
     */
    void finished(bool success, const QString &errorMessage);

private:
    static const int CONNECT_TIMEOUT = 10000;  // 10 seconds
    static const int WRITE_TIMEOUT = 5000;     // 5 seconds
    static const int CHUNK_SIZE = 16384;       // 16KB chunks
    
    QString m_lastError;
};

#endif // FILESENDER_H