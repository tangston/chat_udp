#include "widget.h"
#include "ui_widget.h"
const quint16 PORT = 45454;
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    //--------------------------------------------------------------------------ui
    ui->setupUi(this);
    localIpStr = getIp();
    configFlag = false;        //初始化连接参数 为未连接
    ui->getEdit->ensureCursorVisible();
    ui->sendEdit->setFocus();      //程序启动时，焦点停在发送对话框
    ui->ipEdit->setText(tr("127.0.0.1"));   //设置默认的远程端Ip
    ui->portEdit->setText("45454");           //设置默认端口号
    ui->configButton->setShortcut(tr("Alt+F"));
 //--------------------------------------------------------------------------ui

    localHostAddr = new QHostAddress("127.0.0.1");//用 QHostAddress的格式做了个地址存储的东西
    udpSocket1 = new QUdpSocket(this);//初始化套接字和一系列其他东西，这是要研究的
    bool bindFlag = udpSocket1->bind(*localHostAddr, PORT, QUdpSocket::DefaultForPlatform | QUdpSocket::ReuseAddressHint);

    if(!bindFlag)
    {
        //printf("error=%d\n",WSAGetLastError());
        QMessageBox box;
        box.setText(tr("error on binding socket"));
        box.exec();
    }
    else
    {
        //udpSocket1->readyRead();
        connect(udpSocket1, SIGNAL(readyRead()), this, SLOT(receive()));//第一个参数是QObject发送信号的对象，第二个是信号，比如什么东西触发了，第三个是接收对象，第四个是槽函数
        connect(ui->udpSendButton,SIGNAL(clicked()),this, SLOT(send()));//相当于把两个函数绑定在一起了，里面写的函数都是callback（）回调函数
    }
    this->setWindowTitle(tr("Udp chat UI based on Qt "));
}
QString Widget::getIp()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)   //我们使用IPv4地址
        {
            if(address.toString().contains("127.0."))
                continue;
            qDebug()<<"MyIp:"<<address.toString();
            return address.toString();
        }
    }
    return 0;

}

void Widget::send()
{
    autoScroll();
    QString sendStr = ui->sendEdit->toPlainText();
    QByteArray sendByteArray = sendStr.toLatin1();
    QMessageBox box;
    if(sendStr.length()==0)
    {
        //box.setText(tr("please type your message"));
       // box.exec();
    }
    else if(configFlag)
    {
        QString port=ui->portEdit->displayText();
        udpSocket1->writeDatagram(sendByteArray, sendByteArray.length(), *remoteHostAddr, PORT);

        //本地发送信息再信息交互窗口的显示
        QDateTime time;
        QString timeStr = time.currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
        ui->getEdit->setTextColor(QColor("red"));
        ui->getEdit->insertPlainText("My" + localIpStr + ": " + timeStr + "\n");
        ui->getEdit->setTextColor(QColor("black"));
        ui->getEdit->insertPlainText( sendStr +"\n");
        ui->sendEdit->clear();          //点击发送后，发送编辑框内清零
        ui->sendEdit->setFocus();      //焦点停留在发送编辑框
    }
    else if(!configFlag)
    {
       // box.setText("please click send botton");
      //  box.exec();
    }
}
void Widget::autoScroll()//自动滚屏
{
    QTextCursor cursor = ui->getEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->getEdit->setTextCursor(cursor);
}

void Widget::receive()
{
    while(udpSocket1->hasPendingDatagrams())
    {
        QTextCodec *tc=QTextCodec::codecForName("UTF-8"); //UTF-8
        QDateTime time;
        QString timeStr = time.currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");

        QByteArray data;
        data.resize(udpSocket1->pendingDatagramSize());
        udpSocket1->readDatagram(data.data(), data.size());
        //        QString dataStr =  QString::fromUtf8(data.data());   //这样写也是正确的
        QString dataStr = tc->toUnicode(data);
        ui->getEdit->setTextColor(QColor("red"));
        ui->getEdit->insertPlainText("remote"  + remoteIpStr+": "+ timeStr +"\n" );
        ui->getEdit->setTextColor(QColor("black"));
        ui->getEdit->insertPlainText(dataStr  + "\n" );
        autoScroll();

    }

}


void Widget::on_clearButton_clicked()
{
    ui->getEdit->clear();
}


void Widget::on_configButton_clicked()
{
    remoteIpStr = ui->ipEdit->displayText();
    QString port = ui->portEdit->displayText();
    qDebug()<<"romote Ip:"<<remoteIpStr<<"port num:"<<port;
    remoteHostAddr = new QHostAddress(remoteIpStr);

    QMessageBox box;
    if(remoteIpStr.length()==0 || port.length()==0 || port.toInt()<1024)
    {
        configFlag = false;
       // box.setText("please set correct ip address and port number");
       // box.exec();
    }
    else
    {
        configFlag = true;
       // box.setText("Ip you set:" + remoteIpStr+"port number:"+port);
       // box.exec();
        //this->send();
    }

}

void Widget::on_exitButton_clicked()
{
    this->close();
}
Widget::~Widget()
{
    delete ui;
    udpSocket1->disconnectFromHost();
}

void Widget::on_tcpSendButton_clicked()
{

}

void Widget::on_udpSendButton_clicked()
{
    this->send();
}
