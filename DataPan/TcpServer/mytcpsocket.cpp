#include "mytcpsocket.h"
#include <QDebug>
#include <stdio.h>
#include "mytcpserver.h"
#include <QDir>
#include <QFileInfoList>
MyTcpSocket::MyTcpSocket()
{
    connect(this, SIGNAL(readyRead())
            , this, SLOT(recvMsg()));
    connect(this, SIGNAL(disconnected())
            , this, SLOT(clientOffline()));
    m_bUpload = false;
    m_pTimer = new QTimer;
    connect(m_pTimer,SIGNAL(timeout()),this,SLOT(sendFileToClient()));
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::recvMsg()
{
    PDU *respdu = NULL;
    respdu = mkPDU(0);
    respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
    if(!m_bUpload)
    {
        qDebug() << this->bytesAvailable();
        uint uiPDULen = 0;
        this->read((char*)&uiPDULen, sizeof(uint));
        uint uiMsgLen = uiPDULen-sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        this->read((char*)pdu+sizeof(uint), uiPDULen-sizeof(uint));
        qDebug() << pdu->uiMsgType;
        switch (pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData+32, 32);
            bool ret = OpeDB::getInstance().handleRegist(caName, caPwd);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
            if (ret)
            {
                strcpy(respdu->caData, REGIST_OK);
                QDir dir;
                qDebug() << dir.mkdir(QString("./%1").arg(caName));

            }
            else
            {
                strcpy(respdu->caData, REGIST_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData+32, 32);
            bool ret = OpeDB::getInstance().handleLogin(caName, caPwd);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
            if (ret)
            {
                strcpy(respdu->caData, LOGIN_OK);
                m_strName = caName;
            }
            else
            {
                strcpy(respdu->caData, LOGIN_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            qDebug() << "all online request";
            QStringList ret = OpeDB::getInstance().handleAllOnline();
            uint uiMsgLen = ret.size()*32;
            qDebug () << "size=" << ret.size();
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for (int i=0; i<ret.size(); i++)
            {
                memcpy((char*)(respdu->caMsg)+i*32
                       , ret.at(i).toStdString().c_str()
                       , ret.at(i).size());
                qDebug() << ret.at(i).toStdString().c_str();
                printf("%s\n", (char*)(respdu->caMsg)+i*32);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
        {
            int ret = OpeDB::getInstance().handleSearchUsr(pdu->caData);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
            if (-1 == ret)
            {
                strcpy(respdu->caData, SEARCH_USR_NO);
            }
            else if (1 == ret)
            {
                strcpy(respdu->caData, SEARCH_USR_ONLINE);
            }
            else if (0 == ret)
            {
                strcpy(respdu->caData, SEARCH_USR_OFFLINE);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32);
            strncpy(caName, pdu->caData+32, 32);
            int ret = OpeDB::getInstance().handleAddFriend(caPerName, caName);
            PDU *respdu = NULL;
            if (-1 == ret)
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData, UNKNOW_ERROR);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if (0 == ret)
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData, EXISTED_FRIEND);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if (1 == ret)
            {
                MyTcpServer::getInstance().resend(caPerName, pdu);
            }
            else if (2 == ret)
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData, ADD_FRIEND_OFFLINE);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if (3 == ret)
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData, ADD_FRIEND_NO_EXIST);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE:
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32);
            strncpy(caName, pdu->caData+32, 32);
            OpeDB::getInstance().handleAgreeAddFriend(caPerName, caName);

            MyTcpServer::getInstance().resend(caName, pdu);
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData+32, 32);
            MyTcpServer::getInstance().resend(caName, pdu);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            QStringList ret = OpeDB::getInstance().handleFlushFriend(caName);
            uint uiMsgLen = ret.size()*32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            for (int i=0; i<ret.size(); i++)
            {
                memcpy((char*)(respdu->caMsg)+i*32
                       , ret.at(i).toStdString().c_str()
                       , ret.at(i).size());
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char caSelfName[32] = {'\0'};
            char caFriendName[32] = {'\0'};
            strncpy(caSelfName, pdu->caData, 32);
            strncpy(caFriendName, pdu->caData+32, 32);
            OpeDB::getInstance().handleDelFriend(caSelfName, caFriendName);

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            strcpy(respdu->caData, DEL_FRIEND_OK);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            MyTcpServer::getInstance().resend(caFriendName, pdu);

            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char caPerName[32] = {'\0'};
            memcpy(caPerName, pdu->caData+32, 32);
            char caName[32] = {'\0'};
            memcpy(caName, pdu->caData, 32);
            qDebug() << caName << "-->" << caPerName << (char*)(pdu->caMsg);
            MyTcpServer::getInstance().resend(caPerName, pdu);

            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            QStringList onlineFriend = OpeDB::getInstance().handleFlushFriend(caName);
            QString tmp;
            for (int i=0; i<onlineFriend.size(); i++)
            {
                tmp = onlineFriend.at(i);
                MyTcpServer::getInstance().resend(tmp.toStdString().c_str(), pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            QDir dir;
            QString strCurPath = QString("%1").arg((char *)(pdu->caMsg));
            bool ret = dir.exists(strCurPath);
            PDU *respdu = NULL;
            if(ret){
                char caNewDir[32] = {'\0'};
                memcpy(caNewDir,pdu->caData+32,32);
                QString strNewPath = strCurPath + "/" + caNewDir;
                ret = dir.exists(strNewPath);
                if(ret){
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData,FILE_NAME_EXIST);
                }else
                {
                    dir.mkdir(strNewPath);
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData,CREARE_DIR_OK);
                }
            }else
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData,DIR_NO_EXIST);
            }

            write((char*)respdu, respdu->uiPDULen);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen);
            QDir dir(pCurPath);
            QFileInfoList fileInfoList = dir.entryInfoList();
            int iFileCount = fileInfoList.size();
            PDU *respdu = mkPDU(sizeof(FileInfo)*iFileCount);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
            FileInfo *pFileInfo = NULL;
            QString strFileName;
            for (int i=0; i<iFileCount; i++)
            {
                pFileInfo = (FileInfo*)(respdu->caMsg)+i;
                strFileName = fileInfoList[i].fileName();

                memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size());
                if (fileInfoList[i].isDir())
                {
                    pFileInfo->iFileType = 0;
                }
                else if (fileInfoList[i].isFile())
                {
                    pFileInfo->iFileType = 1;
                }
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;

        }
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
        {
            char caName[32] = {'\0'};
            strcpy(caName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);
            qDebug()<<strPath;
            QFileInfo fileInfo(strPath);
            bool ret = false;
            if(fileInfo.isDir())
            {
                QDir dir;
                dir.setPath(strPath);
                ret = dir.removeRecursively();
            }else if(fileInfo.isFile())
            {
                ret = false;
            }
            PDU *respdu = NULL;
            if(ret)
            {
                respdu = mkPDU(strlen(DEL_DIR_OK) +1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(respdu->caData,DEL_DIR_OK,strlen(DEL_DIR_OK));
            }else
            {
                respdu = mkPDU(strlen(DEL_DIR_FAILURED) +1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(respdu->caData,DEL_DIR_FAILURED,strlen(DEL_DIR_FAILURED));
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
                {
                    char caOldName[32] = {'\0'};
                    char caNewName[32] = {'\0'};
                    strncpy(caOldName, pdu->caData, 32);
                    strncpy(caNewName, pdu->caData+32, 32);

                    char *pPath = new char[pdu->uiMsgLen];
                    memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);

                    QString strOldPath = QString("%1/%2").arg(pPath).arg(caOldName);
                    QString strNewPath = QString("%1/%2").arg(pPath).arg(caNewName);

                    qDebug() << strOldPath;
                    qDebug() << strNewPath;

                    QDir dir;
                    bool ret = dir.rename(strOldPath, strNewPath);
                    PDU *respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
                    if (ret)
                    {
                        strcpy(respdu->caData, RENAME_FILE_OK);
                    }
                    else
                    {
                        strcpy(respdu->caData, RENAME_FILE_FAILURED);
                    }

                    write((char*)respdu, respdu->uiPDULen);
                    free(respdu);
                    respdu = NULL;

                    break;
                }
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        {
            char caEnterName[32] = {'\0'};
            strncpy(caEnterName, pdu->caData, 32);

            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);

            QString strPath = QString("%1/%2").arg(pPath).arg(caEnterName);

            qDebug() << strPath;

            QFileInfo fileInfo(strPath);
            PDU *respdu = NULL;
            if (fileInfo.isDir())
            {
                QDir dir(strPath);
                QFileInfoList fileInfoList = dir.entryInfoList();
                int iFileCount = fileInfoList.size();
                respdu = mkPDU(sizeof(FileInfo)*iFileCount);
                respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
                FileInfo *pFileInfo = NULL;
                QString strFileName;
                for (int i=0; i<iFileCount; i++)
                {
                    pFileInfo = (FileInfo*)(respdu->caMsg)+i;
                    strFileName = fileInfoList[i].fileName();

                    memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size());
                    if (fileInfoList[i].isDir())
                    {
                        pFileInfo->iFileType = 0;
                    }
                    else if (fileInfoList[i].isFile())
                    {
                        pFileInfo->iFileType = 1;
                    }
                }
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if (fileInfo.isFile())
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strcpy(respdu->caData, ENTER_DIR_FAILURED);

                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }

            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            qint64 fileSize = 0;
            sscanf(pdu->caData,"%s %lld",caFileName,&fileSize);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            delete [] pPath;
            pPath = NULL;

            m_file.setFileName(strPath);
            if(m_file.open(QIODevice::WriteOnly))
            {
                m_bUpload = true;
                m_iTotal = fileSize;
                m_iRecved = 0;

            }

            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_REQUEST:
        {
            char caName[32] = {'\0'};
            strcpy(caName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);
            qDebug()<<strPath;
            QFileInfo fileInfo(strPath);
            bool ret = false;
            if(fileInfo.isDir())
            {
                ret = false;
            }else if(fileInfo.isFile())
            {
                QDir dir;
                dir.remove(strPath);
            }
            PDU *respdu = NULL;
            if(ret)
            {
                respdu = mkPDU(strlen(DEL_FILE_OK) +1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->caData,DEL_FILE_OK,strlen(DEL_FILE_OK));
            }else
            {
                respdu = mkPDU(strlen(DEL_FILE_FAILURED) +1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->caData,DEL_FILE_FAILURED,strlen(DEL_FILE_FAILURED));
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            strcpy(caFileName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            delete [] pPath;
            pPath = NULL;

            QFileInfo fileInfo(strPath);
            qint64 fileSize = fileInfo.size();
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            sprintf(respdu->caData,"%s %lld",caFileName,fileSize);
            write((char *)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            m_file.setFileName(strPath);
            m_file.open(QIODevice::ReadOnly);

            m_pTimer->start(1000);



        }
        default:
            break;
        }

        free(pdu);
        pdu = NULL;
        }
    else
    {
        QByteArray buff = readAll();
        m_file.write(buff);
        m_iRecved += buff.size();
        if(m_iTotal == m_iRecved)
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData,UPLOAD_FILE_OK);
            write((char *)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(m_iTotal < m_iRecved)
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData,UPLOAD_FILE_FAILURED);
            write((char *)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;


        }

    }
    //    qDebug() << caName << caPwd << pdu->uiMsgType;
}

void MyTcpSocket::clientOffline()
{
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this);
}

void MyTcpSocket::sendFileToClient()
{
    char *pDate = new char[4096];
    qint64 ret = 0;
    while(true)
    {
        ret = m_file.read(pDate,4096);
        if(ret > 0 && ret <= 4096)
        {
            write(pDate,ret);
        }
        else if(0==ret)
        {
            m_file.close();
            break;
        }else if(ret < 0)
        {
            qDebug()<<"发送内容给客户端过程中失败";
            m_file.close();
            break;
        }
    }
    delete [] pDate;
    pDate = NULL;
}
