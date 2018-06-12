#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    my_serialPort(new QSerialPort(this)),
    my_server(new QTcpServer(this))
{    
    ui->setupUi(this);
    timer = new QTimer(this);
    MDtimer_2000ms = new QTimer(this);
    MDtimer_100ms = new QTimer(this);
    if(checkBox_Serial_Flag == true)
    {
        foreach( const QSerialPortInfo &Info,QSerialPortInfo::availablePorts())//读取串口信息
         {
            QSerialPort serial;
            serial.setPort(Info);
            if( serial.open( QIODevice::ReadWrite) )//如果串口是可以读写方式打开的
            {
                ui->comboBox_Serial->addItem( Info.portName() + ' '+ ' ' + Info.description() );//在comboBox那添加串口号
                serial.close();//然后自动关闭等待人为开启（通过那个打开串口的PushButton）
            }
          }
         qDebug()<<"点击了串口复选框" ;
         connect(my_serialPort,SIGNAL(readyRead()),this,SLOT(readMyCom()));
    }

    if(checkBox_Serial_Flag == false)
    {
        ui->comboBox_Serial->clear();

    }

    if(checkBox_Socket_Flag == true)
    {
        QList<QHostAddress> addList = QNetworkInterface::allAddresses();

        foreach(QHostAddress address,addList)
        {
            //排除IPV6，排除回环地址
            if(address.protocol() == QAbstractSocket::IPv4Protocol
                    && address != QHostAddress(QHostAddress::LocalHost))
            {
                //输出，转换为字符串格式
                qDebug() << address.toString();
                ui->comboBox_TcpServer->addItem(address.toString());

            }
        }
    }

    if(checkBox_Socket_Flag == false)
    {
        ui->comboBox_TcpServer->clear();
        ui->comboBox_TcpClient->clear();

    }

   connect(timer,SIGNAL(timeout()),this,SLOT(TimerUpdate_COM()));
   connect(MDtimer_2000ms,SIGNAL(timeout()),this,SLOT(MD_BeforeUpdat_timer()));
   connect(MDtimer_100ms,SIGNAL(timeout()),this,SLOT(MD_Update()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

/************************************************************************************************************************************
                                                              超时候处理数据
*************************************************************************************************************************************/
void MainWindow::TimerUpdate_COM()
{
    char *readdate_tohex;
    unsigned int readdate_lenth;
//    bool isHexRead = ui->isHexRead->isChecked();
    timer->stop();
    readdate_tohex = requestData.data();
    readdate_lenth = requestData.length();

     if(requestData != NULL)
     {
          QString str = ui->textEdit_Read->toPlainText();
          str += tr(requestData);
          ui->textEdit_Read->clear();
          ui->textEdit_Read->append(str);
     }
    myData_B2.clear();
}

/************************************************************************************************************************************
                                                             串口接收处理
*************************************************************************************************************************************/
void MainWindow::readMyCom()//读取缓冲的数据
{

     timer->start(400);
     if(checkBox_Serial_Flag == true)
     {
        requestData = myData_B2.append(my_serialPort->readAll());
     }

     if(checkBox_Socket_Flag== true)
     {
        requestData = myData_B2.append(my_socket->readAll());
     }
      qDebug()<<requestData.toHex()<<"转换结束";

}

/************************************************************************************************************************************
                                                             打开升级文件控件处理逻辑
*************************************************************************************************************************************/
void MainWindow::on_pushButton_OpenFile_clicked()
{
    QFileInfo open_update_fileInfo;
    open_update_name = QFileDialog::getOpenFileName(this,tr("打开升级文件"),tr("C:"),tr("Bin Files(*.bin)"),0);
    if(!open_update_name.isNull())
    {
        open_update_fileInfo.setFile(open_update_name);
        qDebug() <<"点击了打开";
        qDebug() <<"file path is \""<<open_update_fileInfo.path()<<"\"";
        qDebug() <<"file name is"<<open_update_fileInfo.fileName();
        qDebug() <<"fileBaseName is"<<open_update_fileInfo.baseName();
        qDebug() <<"fileExtension is "<<open_update_fileInfo.suffix();
    }
    else
    {
        qDebug()<<"点击了取消";
    }
}

/************************************************************************************************************************************
                                                             MD柜升级槽函数处理逻辑

*************************************************************************************************************************************/
void MainWindow::on_pushButton_SendFile_clicked()
{
    if(checkBox_Serial_Flag == true)
    {
        if(my_serialPort->isOpen())
        {
            ui->textEdit_Read->clear();
            ui->textEdit_Read->append(tr("MD升级前准备"));
            MDtimer_2000ms ->start(2000); //16个字节延时发送时间修改
        }
        else
        {
            QMessageBox::critical(this,tr("警告"),"请先打开串口");
        }

    }
    else if(checkBox_Socket_Flag ==true)
    {
       if(my_server->isListening())
       {
           ui->textEdit_Read->clear();
           ui->textEdit_Read->append(tr("MD升级前准备"));
           MDtimer_2000ms ->start(2000); //16个字节延时发送时间修改
       }

       else
       {
           QMessageBox::critical(this,tr("警告"),"请先打开TCP通信");
       }

    }
    else
    {
           QMessageBox::critical(this,tr("警告"),"请先选择通信方式");
    }
}

/************************************************************************************************************************************
                                                             MD升级槽函数处理逻辑
串口延时发送还是单线程。主要是建立一个定时器来进行延时发送。
*************************************************************************************************************************************/
void MainWindow::MD_BeforeUpdat_timer()
{
    QFile file(open_update_name);
    if(MD_BeforeUpdate_Flag==0)
    {
        MD_BeforeUpdate(1);
        ui->textEdit_Read->append(tr("MD升级前第一包已经发送完毕"));
    }

    if(MD_BeforeUpdate_Flag==1)
    {
        MD_BeforeUpdate(2);
       ui->textEdit_Read->append(tr("MD升级前第二包已经发送完毕"));

    }

    if(MD_BeforeUpdate_Flag==2)
    {
        MD_BeforeUpdate_Flag = 0;
        ui->textEdit_Read->append("MD现在准备升级");
        MDtimer_2000ms->stop();//升级前的延时停止
        timer->stop();//超时检测串口也关闭掉
        MDtimer_100ms->start(200);
    }
    MD_BeforeUpdate_Flag++;

}

/************************************************************************************************************************************
                                                             MD升级槽函数处理逻辑
串口延时发送还是单线程。主要是建立一个定时器来进行延时发送。
*************************************************************************************************************************************/
void MainWindow::MD_BeforeUpdate(uint8_t freq)
{
    QFile file(open_update_name);
    QByteArray to_bin;
    QByteArray to_binCount;
    char *Update;
    qDebug() << "打开了发送升级文件!";

    if(file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Open file !";
        qDebug()<<"文件大小："<<file.size();
        file.seek(0);//设置当前文件位置
        qDebug()<<"第一次发送的当前位置："<<file.pos();
        to_bin = file.read(16);
        if(ui->comboBox_ChooseUpdate->currentText() == "全部药盒板")
        {
            Node_str = QString("Qpdate=1234,Node=FFFF,Boot=%1,%2,").arg(freq).arg(to_bin.length());
        }
        else if(ui->comboBox_ChooseUpdate->currentText() == "全部层板")
        {
            Node_str = QString("Qpdate=1234,Node=FF00,Boot=%1,%2,").arg(freq).arg(to_bin.length());
        }
        else if(ui->comboBox_ChooseUpdate->currentText() == "指定板")
        {
            if(ui->lineEdit_ChooseUpdate->text().isEmpty())
            {
               QMessageBox::critical(this,tr("警告"),"设定值不能为空!!!");
            }
            else
            {
                 Node_str = QString("Qpdate=1234,Node=%1,Boot=%2,%3,").arg(ui->lineEdit_ChooseUpdate->text()).arg(freq).arg(to_bin.length());
            }
        }
        to_binCount =Node_str.toLatin1() + to_bin;
        Update = to_binCount.data();
        qDebug()<<"第一次发送的当前大小："<<to_bin.length();
        qDebug()<<"第一次发送的当前大小1："<<to_binCount.length();
        qDebug()<<"内容："<<Update;
        if(checkBox_Serial_Flag == true)
        {
             my_serialPort->write(Update,to_binCount.length());
        }

        if(checkBox_Socket_Flag == true)
        {
             my_socket->write(Update,to_binCount.length());
        }

        qDebug()<<"第一次发送的当前位置："<<file.pos();

    }
     file.close();
}

/************************************************************************************************************************************
                                                             MD升级槽函数处理逻辑
串口延时发送还是单线程。主要是建立一个定时器来进行延时发送。
*************************************************************************************************************************************/
void MainWindow::MD_Update()
{
    QFile file(open_update_name);
    QByteArray to_bin;
    QByteArray to_binCount;
    char *Update;
    qDebug() << "打开了发送升级文件!";
    if(file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Open file !";
        qDebug()<<"文件大小："<<file.size();
        file.seek(16 + (512*MD_Update_Flag));//设置当前文件位置
        qDebug()<<"当前位置："<<file.pos();
        to_bin =  file.read(512) ;
        if(ui->comboBox_ChooseUpdate->currentText() == "全部药盒板")
        {
            Node_str = QString("Qpdate=1234,Node=FFFF,Boot=%1,%2,").arg((MD_Update_Flag + 3)).arg(to_bin.length());

        }
        else if(ui->comboBox_ChooseUpdate->currentText() == "全部层板")
        {
            Node_str = QString("Qpdate=1234,Node=FF00,Boot=%1,%2,").arg((MD_Update_Flag + 3)).arg(to_bin.length());
        }
        else if(ui->comboBox_ChooseUpdate->currentText() == "指定板")
        {
            if(ui->lineEdit_ChooseUpdate->text().isEmpty())
            {
               QMessageBox::critical(this,tr("警告"),"设定值不能为空!!!");
            }
            else
            {
                 Node_str = QString("Qpdate=1234,Node=%1,Boot=%2,%3,").arg(ui->lineEdit_ChooseUpdate->text()).arg((MD_Update_Flag + 3)).arg(to_bin.length());
            }
        }
        to_binCount =Node_str.toLatin1() + to_bin;
        Update = to_binCount.data();
        qDebug()<<"当前大小："<<to_bin.length();
        qDebug()<<"当前包："<<MD_Update_Flag + 3;
        qDebug()<<"当前大小1："<<to_binCount.length();
        if(checkBox_Serial_Flag == true) //串口
        {
            my_serialPort->write(Update,to_binCount.length());
        }

        if(checkBox_Socket_Flag == true) //TCP
        {
             my_socket->write(Update,to_binCount.length());
        }
        qDebug()<<"当前位置："<<file.pos();
        MD_Update_Flag++;
        ui->textEdit_Read->append(tr("MD升级第%1包已经发送完毕,长度为:%2").arg(MD_Update_Flag).arg(to_bin.length()));
        if((to_bin.length() < 512) ||(file.atEnd()))
        {
            MD_Update_Flag = 0;
            MD_BeforeUpdate_Flag = 0;
            MDtimer_100ms->stop();//超时检测串口也关闭掉
            timer->start();
            ui->textEdit_Read->append(tr("MD升级最后一包已经发送完毕,长度为:%1").arg(to_bin.length()));
        }

    }
     file.close();
}

/************************************************************************************************************************************
                                                             打开串口控件处理逻辑
*************************************************************************************************************************************/
void MainWindow::on_pushButton_SerialOpen_toggled(bool checked)
{
    if(checkBox_Serial_Flag == true)
    {
        if(checked == true)
        {
             qDebug() <<"PushButtom_comopen_on";
            //对串口进行一些初始化
             ui->pushButton_SerialOpen->setText("关闭串口");
             my_serialPort->setPortName( ui->comboBox_Serial->currentText().mid(0,5) );//mid()截取QString指定位置的的值
             qDebug() << ui->comboBox_Serial->currentText();
             my_serialPort->setBaudRate(  ui->comboBox_baudRate->currentText().toInt() );//波特率
             my_serialPort->setDataBits( QSerialPort::Data8 );//数据字节，8字节
             my_serialPort->setParity( QSerialPort::NoParity );//校验，无
             my_serialPort->setFlowControl( QSerialPort::NoFlowControl );//数据流控制,无
             my_serialPort->setStopBits( QSerialPort::OneStop );//一位停止位

            if( my_serialPort->open( QIODevice::ReadWrite ))
            {
                 ui->textEdit_Read->append(tr("打开串口成功"));
                 qDebug() << "serial open successful";
            }
             else
             {
                 QMessageBox::critical(this,tr("警告"),"打开失败");
             }
        }
        if(checked == false)
        {
             ui->pushButton_SerialOpen->setText("打开串口");
             ui->textEdit_Read->append(tr("关闭串口成功"));
             qDebug() <<"PushButtom_comopen_off";
             my_serialPort->close();
        }
    }

    if(checkBox_Serial_Flag == false)
    {
         QMessageBox::critical(this,tr("警告"),"请先选择通信方式");
    }
}

/************************************************************************************************************************************
                                                             Socket打开控件处理逻辑
*************************************************************************************************************************************/
void MainWindow::on_pushButton_SocketOpen_toggled(bool checked)
{
    if(checkBox_Socket_Flag == true)
    {
        if(checked == true)
        {
            ui->pushButton_SocketOpen->setText("关闭连接");
            ui->textEdit_Read->append(tr("打开TCP成功"));
            my_server->listen(QHostAddress::Any,ui->lineEdit_SocketPort->text().toInt());
            connect(my_server, SIGNAL(newConnection()),this, SLOT(acceptConnection()));
        }

        if(checked == false)
        {
            ui->pushButton_SocketOpen->setText("打开连接");
            ui->textEdit_Read->append(tr("关闭TCP成功"));
            my_socket->disconnectFromHost();
            my_server->disconnect();
            my_server->close();
            ui->comboBox_TcpClient ->clear();
//            ui->comboBox_serverName->clear();
            disconnect(my_server, SIGNAL(newConnection()),this, SLOT(acceptConnection()));
            disconnect(my_socket,SIGNAL(readyRead()),this,SLOT( readMyCom()));
        }
    }

    if(checkBox_Socket_Flag == false)
    {
          ui->comboBox_TcpServer->clear();
          QMessageBox::critical(this,tr("警告"),"请先选择通信方式");
    }
}

void MainWindow::acceptConnection()
{
    my_socket = my_server->nextPendingConnection();
//    ui->comboBox_clientName->addItem(my_socket->peerAddress().toString() + '--' + my_socket->peerPort());
    ui->comboBox_TcpClient->addItem(my_socket->peerAddress().toString().mid(7) + '::');
    qDebug()<<"Client IP"<<my_socket->peerAddress().toIPv4Address();
    connect(my_socket,SIGNAL(readyRead()),this,SLOT( readMyCom()));
}

/************************************************************************************************************************************
                                                              串口复选框判断
*************************************************************************************************************************************/
void MainWindow::on_checkBox_Serial_clicked(bool checked)
{
    if(checked == true)
    {
        foreach( const QSerialPortInfo &Info,QSerialPortInfo::availablePorts())//读取串口信息
         {
            QSerialPort serial;
            serial.setPort(Info);
            if( serial.open( QIODevice::ReadWrite) )//如果串口是可以读写方式打开的
            {
                ui->comboBox_Serial->addItem( Info.portName() + ' '+ ' ' + Info.description() );//在comboBox那添加串口号
//                ui->comboBox_portName->addItem( Info.portName() );//在comboBox那添加串口号
                serial.close();//然后自动关闭等待人为开启（通过那个打开串口的PushButton）
            }
          }

        connect(my_serialPort,SIGNAL(readyRead()),this,SLOT( readMyCom()));
        if(checkBox_Socket_Flag == true)
        {
            ui->checkBox_Socket->setChecked(false);
            ui->comboBox_TcpClient ->clear();
            ui->comboBox_TcpServer->clear();

        }
        qDebug()<<"点击了串口复选框" ;

    }
    if(checked == false)
    {
        disconnect(my_serialPort,SIGNAL(readyRead()),this,SLOT( readMyCom()));
        ui->comboBox_Serial->clear();
    }
}

/************************************************************************************************************************************
                                                              Socket复选框判断
*************************************************************************************************************************************/
void MainWindow::on_checkBox_Socket_clicked(bool checked)
{
    if(checked == true)
    {
        QList<QHostAddress> addList = QNetworkInterface::allAddresses();
        foreach(QHostAddress address,addList)
        {
            //排除IPV6，排除回环地址
            if(address.protocol() == QAbstractSocket::IPv4Protocol
                    && address != QHostAddress(QHostAddress::LocalHost))
            {
                //输出，转换为字符串格式
                qDebug() << address.toString();
                ui->comboBox_TcpServer->addItem(address.toString());

            }
        }
        if(checkBox_Serial_Flag == true)
        {
            ui->checkBox_Serial->setChecked(false);
            ui->comboBox_Serial->clear();

        }

    }
    if(checked == false)
    {
        ui->comboBox_TcpClient ->clear();
        ui->comboBox_TcpServer->clear();
        disconnect(my_server, SIGNAL(newConnection()),this, SLOT(acceptConnection()));
        disconnect(my_socket,SIGNAL(readyRead()),this,SLOT( readMyCom()));
    }
}
/************************************************************************************************************************************
                                                             清除消息框
*************************************************************************************************************************************/
void MainWindow::on_pushButton_Clear_clicked()
{
    ui->textEdit_Read->clear();
}
/************************************************************************************************************************************
                                                             读取版本号
*************************************************************************************************************************************/
void MainWindow::on_pushButton_Version_clicked()
{
    Node_str = QString("Read=1234,Node=%1,Version=?").arg(ui->lineEdit_ChooseUpdate->text());
    if(checkBox_Serial_Flag == true)
    {
        if(my_serialPort->isOpen())
        {
            my_serialPort->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
             QMessageBox::critical(NULL,tr("警告"),"请先打开串口");
        }
    }

    else if(checkBox_Socket_Flag == true)
    {
        if(my_server->isListening())
        {
            my_socket->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
               QMessageBox::critical(this,tr("警告"),"请先打开TCP通信");
        }
    }

    else
    {
         QMessageBox::critical(this,tr("警告"),"请先选择通信方式");
    }
}
/************************************************************************************************************************************
                                                           校准准备
*************************************************************************************************************************************/
void MainWindow::on_pushButton_Calib_clicked()
{
    Node_str = QString("Calibration=5905,Node=%1,Count=0").arg(ui->lineEdit_ChooseUpdate->text());
    if(checkBox_Serial_Flag == true)
    {
        if(my_serialPort->isOpen())
        {
            my_serialPort->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
             QMessageBox::critical(NULL,tr("警告"),"请先打开串口");
        }
    }

    else if(checkBox_Socket_Flag == true)
    {
        if(my_server->isListening())
        {
            my_socket->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
               QMessageBox::critical(this,tr("警告"),"请先打开TCP通信");
        }
    }

    else
    {
         QMessageBox::critical(this,tr("警告"),"请先选择通信方式");
    }
}
/************************************************************************************************************************************
                                                             校准完成
*************************************************************************************************************************************/
void MainWindow::on_pushButton_Calibcom_clicked()
{

    Node_str = QString("Calibration=5905,Node=%1,Count=%2").arg(ui->lineEdit_ChooseUpdate->text()).arg(ui->lineEdit_CalibNum->text());
    if(checkBox_Serial_Flag == true)
    {
        if(my_serialPort->isOpen())
        {
            my_serialPort->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
             QMessageBox::critical(NULL,tr("警告"),"请先打开串口");
        }
    }

    else if(checkBox_Socket_Flag == true)
    {
        if(my_server->isListening())
        {
            my_socket->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
               QMessageBox::critical(this,tr("警告"),"请先打开TCP通信");
        }
    }

    else
    {
         QMessageBox::critical(this,tr("警告"),"请先选择通信方式");
    }
}
/************************************************************************************************************************************
                                                           开灯
*************************************************************************************************************************************/

void MainWindow::on_pushButton_LEDOn_clicked()
{
    if(ui->comboBox_ChooseUpdate->currentText() == "全部药盒板")
    {
        Node_str = QString("Write=1234,Node=FFFF,Led=On");

    }
    else if(ui->comboBox_ChooseUpdate->currentText() == "全部层板")
    {
        Node_str = QString("Write=1234,Node=FF00,Led=On");
    }
    else if(ui->comboBox_ChooseUpdate->currentText() == "指定板")
    {
        if(ui->lineEdit_ChooseUpdate->text().isEmpty())
        {
           QMessageBox::critical(this,tr("警告"),"设定值不能为空!!!");
        }
        else
        {
             Node_str = QString("Write=1234,Node=%1,Led=On").arg(ui->lineEdit_ChooseUpdate->text());
        }
    }
    if(checkBox_Serial_Flag == true)
    {
        if(my_serialPort->isOpen())
        {
            my_serialPort->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
             QMessageBox::critical(NULL,tr("警告"),"请先打开串口");
        }
    }

    else if(checkBox_Socket_Flag == true)
    {
        if(my_server->isListening())
        {
            my_socket->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
               QMessageBox::critical(this,tr("警告"),"请先打开TCP通信");
        }
    }

    else
    {
         QMessageBox::critical(this,tr("警告"),"请先选择通信方式");
    }
}
/************************************************************************************************************************************
                                                          关灯
*************************************************************************************************************************************/
void MainWindow::on_pushButton_LEDOff_clicked()
{
    if(ui->comboBox_ChooseUpdate->currentText() == "全部药盒板")
    {
        Node_str = QString("Write=1234,Node=FFFF,Led=Off");

    }
    else if(ui->comboBox_ChooseUpdate->currentText() == "全部层板")
    {
        Node_str = QString("Write=1234,Node=FF00,Led=Off");
    }
    else if(ui->comboBox_ChooseUpdate->currentText() == "指定板")
    {
        if(ui->lineEdit_ChooseUpdate->text().isEmpty())
        {
           QMessageBox::critical(this,tr("警告"),"设定值不能为空!!!");
        }
        else
        {
             Node_str = QString("Write=1234,Node=%1,Led=Off").arg(ui->lineEdit_ChooseUpdate->text());
        }
    }
    if(checkBox_Serial_Flag == true)
    {
        if(my_serialPort->isOpen())
        {
            my_serialPort->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
             QMessageBox::critical(NULL,tr("警告"),"请先打开串口");
        }
    }

    else if(checkBox_Socket_Flag == true)
    {
        if(my_server->isListening())
        {
            my_socket->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
               QMessageBox::critical(this,tr("警告"),"请先打开TCP通信");
        }
    }

    else
    {
         QMessageBox::critical(this,tr("警告"),"请先选择通信方式");
    }
}
/************************************************************************************************************************************
                                                       温湿度上报时间
*************************************************************************************************************************************/
void MainWindow::on_pushButton_ReportTime_clicked()
{
    Node_str = QString("Write=1234,Node=%1,SetTime=%2").arg(ui->lineEdit_ChooseUpdate->text()).arg(ui->lineEdit_ReportTime->text());
    if(checkBox_Serial_Flag == true)
    {
        if(my_serialPort->isOpen())
        {
            my_serialPort->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
             QMessageBox::critical(NULL,tr("警告"),"请先打开串口");
        }
    }

    else if(checkBox_Socket_Flag == true)
    {
        if(my_server->isListening())
        {
            my_socket->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
               QMessageBox::critical(this,tr("警告"),"请先打开TCP通信");
        }
    }

    else
    {
         QMessageBox::critical(this,tr("警告"),"请先选择通信方式");
    }
}
/************************************************************************************************************************************
                                                       读取节点
*************************************************************************************************************************************/
void MainWindow::on_pushButton_ReadNode_clicked()
{
    Node_str = QString("Read=1234,Node=%1").arg(ui->lineEdit_ChooseUpdate->text());
    if(checkBox_Serial_Flag == true)
    {
        if(my_serialPort->isOpen())
        {
            my_serialPort->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
             QMessageBox::critical(NULL,tr("警告"),"请先打开串口");
        }
    }

    else if(checkBox_Socket_Flag == true)
    {
        if(my_server->isListening())
        {
            my_socket->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
               QMessageBox::critical(this,tr("警告"),"请先打开TCP通信");
        }
    }

    else
    {
         QMessageBox::critical(this,tr("警告"),"请先选择通信方式");
    }
}
/************************************************************************************************************************************
                                                       开锁
*************************************************************************************************************************************/
void MainWindow::on_pushButton_Unlock_clicked()
{
    if(ui->comboBox_ChooseUpdate->currentText() == "全部药盒板")
    {
        Node_str = QString("Write=1234,Node=FFFF,Lock=Unlock");

    }
    else if(ui->comboBox_ChooseUpdate->currentText() == "全部层板")
    {
        Node_str = QString("Write=1234,Node=FF00,Lock=Unlock");
    }
    else if(ui->comboBox_ChooseUpdate->currentText() == "指定板")
    {
        if(ui->lineEdit_ChooseUpdate->text().isEmpty())
        {
           QMessageBox::critical(this,tr("警告"),"设定值不能为空!!!");
        }
        else
        {
             Node_str = QString("Write=1234,Node=%1,Lock=Unlock").arg(ui->lineEdit_ChooseUpdate->text());
        }
    }
    if(checkBox_Serial_Flag == true)
    {
        if(my_serialPort->isOpen())
        {
            my_serialPort->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
             QMessageBox::critical(NULL,tr("警告"),"请先打开串口");
        }
    }

    else if(checkBox_Socket_Flag == true)
    {
        if(my_server->isListening())
        {
            my_socket->write(Node_str.toLocal8Bit());
            ui->textEdit_Read->append(Node_str.toLocal8Bit() + "\r\n");
        }
        else
        {
               QMessageBox::critical(this,tr("警告"),"请先打开TCP通信");
        }
    }

    else
    {
         QMessageBox::critical(this,tr("警告"),"请先选择通信方式");
    }
}
