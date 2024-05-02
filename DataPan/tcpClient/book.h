#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "protocol.h"
#include <QTimer>
#include <QCheckBox>
class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = 0);
    void updateFileList(const PDU *pdu);
    void clearEnterName();
    QString enterDir();
    void setDownloadStatus(bool status);
    qint64 m_iTotal;  //总的大小
    qint64 m_iReccved;  //已收到大小
    bool getDownloadStatus();

    QString getSaveFilePath();
signals:

public slots:
    void createDir();
    void flushFile();
    void delDir();
    void renameFile();
    void enterDir(const QModelIndex &index);
    void returnPre();
    void delRegFile();
    void uploadFile();
    void uploadFileData();
    void downloadFile();
    void shareFile();

private:
    QListWidget *m_pBookListW;
    QPushButton *m_pReturnPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDirPB;
    QPushButton *m_pRenamePB;
    QPushButton *m_pFlushFilePB;
    QPushButton *m_pUploadPB;
    QPushButton *m_pDownLoadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pshareFilePB;
    QString m_strEnterDir;
    QString m_strUploadFilePath;
    QTimer *m_pTimer;
    QString m_strSaveFilePath;
    bool m_bDownload;

};

#endif // BOOK_H
