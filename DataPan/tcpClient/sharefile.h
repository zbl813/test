#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QListWidget>
#include <QCheckBox>
class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = 0);
    static ShareFile &getInstance();
    void test();
    void updateFriend(QListWidget *pFriendList);
signals:

public slots:


private:
    QPushButton *m_pSelectAllPB;
    QPushButton *m_pCancelSelectPB;

    QPushButton *m_pOKPB;
    QPushButton *m_pCancelPB;

    QScrollArea *m_pSA;
    QWidget *m_pFriendW;
    QButtonGroup *m_pButtonGroup;
    QVBoxLayout *m_pFriendWVBL;
};

#endif // SHAREFILE_H
