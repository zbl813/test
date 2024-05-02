#include "tcpclient.h"
#include <QApplication>
//#include "book.h"
#include "sharefile.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont font("Times",24,QFont::Bold);
    a.setFont(font);

//    Book w;
//    w.show();

//    TcpClient w;
//    w.show();
    TcpClient::getInstance().show();
//    ShareFile W;
//    W.show();
    return a.exec();
}
