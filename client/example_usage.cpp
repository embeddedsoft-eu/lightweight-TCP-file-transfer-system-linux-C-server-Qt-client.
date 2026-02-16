/**
 * Example usage of FileSender class
 */

#include <QCoreApplication>
#include <QDebug>
#include "FileSender.h"


class FileTransferExample : public QObject
{
    Q_OBJECT

public:
    FileTransferExample(QCoreApplication *app) : m_app(app) {}

    void startTransfer() {
        m_sender = new FileSender(this);
        
        // Connect signals
        connect(m_sender, &FileSender::progressChanged, this, &FileTransferExample::onProgress);
        connect(m_sender, &FileSender::finished, this, &FileTransferExample::onFinished);
        
        // Start transfer
        m_sender->sendFile("test_file.txt", "embeddedsoft.eu",  4445);
    }

private slots:
    void onProgress(qint64 sent, qint64 total) {
        int percent = (total > 0) ? (sent * 100 / total) : 0;
        qDebug() << "Progress:" << percent << "%";
    }
    
    void onFinished(bool success, const QString &error) {
        if (success) {
            qDebug() << "Transfer completed successfully!";
        }
        else {
            qDebug() << "Transfer failed:" << error;
        }
        m_app->quit();
    }

private:
    QCoreApplication *m_app;
    FileSender *m_sender;
};


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    FileTransferExample example(&app);
    example.startTransfer();
    
    return app.exec();
}

#include "example_usage.moc"
