#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),//构造函数
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setRange(0,100-1);
    ui->progressBar->setValue(0);
    foreach( const QSerialPortInfo &Info,QSerialPortInfo::availablePorts())//读取串口信息
    {
//        qDebug() << "portName    :"  << Info.portName();//调试时可以看的串口信息
//        qDebug() << "Description   :" << Info.description();
//        qDebug() << "Manufacturer:" << Info.manufacturer();
        QSerialPort serial;
        serial.setPort(Info);
        if( serial.open( QIODevice::ReadWrite) )//如果串口是可以读写方式打开的
        {
            ui->comboBox_ck->addItem( Info.portName() );//在comboBox那添加串口号
            serial.close();//然后自动关闭等待人为开启（通过那个打开串口的PushButton）
        }
    }
}

MainWindow::~MainWindow()//析构函数
{
    delete ui;
}


void MainWindow::on_OPen_ChuanKou_Button_clicked()
{
    if(ui->OPen_ChuanKou_Button->text()==tr("打开串口"))
    {
        my_serialPort = new QSerialPort(this);
        my_serialPort->setPortName( ui->comboBox_ck->currentText() );
        my_serialPort->open( QIODevice::ReadWrite );
        my_serialPort->setPortName(ui->comboBox_ck->currentText());
        //my_serialPort->setBaudRate(  QSerialPort::Baud115200 );//波特率115200
        my_serialPort->setBaudRate(  ui->comboBox_dk->currentText().toInt() );//波特率
        my_serialPort->setDataBits( QSerialPort::Data8 );//数据字节，8字节
        my_serialPort->setParity( QSerialPort::NoParity );//校验，无
        my_serialPort->setFlowControl( QSerialPort::NoFlowControl );//数据流控制,无
        my_serialPort->setStopBits( QSerialPort::OneStop );//一位停止位
        ui->OPen_ChuanKou_Button->setText(tr("关闭串口"));
        timer = new QTimer(this);
        connect( timer, SIGNAL( timeout() ), this, SLOT( readMyCom() ) );
        timer->start(100);//100ms读取一次串口接收数据
    }
    else
    {
        my_serialPort->clear();
        my_serialPort->close();
        my_serialPort->deleteLater();
        ui->OPen_ChuanKou_Button->setText(tr("打开串口"));
    }
}
void MainWindow::readMyCom()
{
    if(ui->OPen_ChuanKou_Button->text()==tr("关闭串口"))
    {
#if 0
        isHexRead=ui->HEXcheckBox->isChecked();
        if(!isHexRead)
        {
            QByteArray buf;
            buf = my_serialPort->readAll();
            if(buf!=NULL)
            {
                QString str = ui->textEdit_read->toPlainText();
                str+=tr(buf);//累加接收的数据
                ui->RxtextBrowser->clear();
                ui->RxtextBrowser->append(str);//附加str字符串值
            }
        }

        if(isHexRead)
        {
            QByteArray temp = my_serialPort->readAll();
            QDataStream out(&temp,QIODevice::ReadWrite);    //将字节数组读入
            while(!out.atEnd())
            {
                qint8 outChar = 0;
                out>>outChar;   //每字节填充一次，直到结束
                //十六进制的转换
                QString str = QString("%1").arg(outChar & 0xFF,2,16,QLatin1Char('0'));
                ui->RxtextBrowser->insertPlainText(str.toUpper());//大写
                ui->RxtextBrowser->insertPlainText(" ");//每发送两个字符后添加一个空格
                ui->RxtextBrowser->moveCursor(QTextCursor::End);
            }
        }
#else
        requestData = my_serialPort->readAll();//用requestData存储从串口那读取的数据

        if(requestData!=NULL)//如果读取到有数据，就显示出来。
        {
            //ui->RxtextBrowser->append(requestData);
            ui->RxtextBrowser->insertPlainText(requestData);
            //for(int i=0;i<requestData.size();i++)
            {
                if ( requestData.at(0) ==  'a'  &&  requestData.at(1)  ==  'f')
                    countNum(++num);
            }
        }
        requestData.clear();    //清除缓冲区
#endif
    }
}

void MainWindow::on_Clear_Button_clicked()
{
    ui->RxtextBrowser->clear();
    ui->SendtextEdit->clear();
    ui->textEdit->clear();
}

void MainWindow::on_pushButton_clicked()
{
    if(ui->OPen_ChuanKou_Button->text()==tr("关闭串口"))
    {
        my_serialPort->write(ui->SendtextEdit->toPlainText().toLatin1());
    }
    else
    {
        //QMessageBox::about(this,tr("提示信息"),tr("更新成功"));
        QMessageBox::information(this,"提示","请打开串口");
    }
}

void MainWindow::on_Open_Button_clicked()
{

    if(ui->Open_Button->text()==tr("Open"))
    {

        fileName=QFileDialog::getOpenFileName(this,
                                              QString::fromLocal8Bit("bin file"),
                                              qApp->applicationDirPath(),
                                              QString::fromLocal8Bit("bin File(*.bin)"));//新建文件打开窗口
        if (fileName.isEmpty())//如果未选择文件便确认，即返回
            return;
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly))
            ui->textBrowser->setText(file.errorString());//文件打开错误显示错误信息
        arry=file.readAll();//读取文件
        QString s = QString::fromUtf8(arry.constData(),arry.length());
        //ui->textBrowser->append(QString::fromLocal8Bit("打开文件："));
        ui->textBrowser->append(fileName);
        file.close();
        length=arry.size();//计算长度
        //ui->textBrowser->setText(s);//显示文件内容
//        ui->textEdit->append(s);//显示出数据
        /*
        QDataStream out(&arry,QIODevice::ReadWrite);    //将字节数组读入
        while(!out.atEnd())
        {
            quint64 outChar = 0;
            out>>outChar;   //每字节填充一次，直到结束
            //十六进制的转换
            QString str = QString("%1").arg(outChar & 0xFF,2,16,QLatin1Char('0'));
            ui->textEdit->insertPlainText(str.toUpper());//大写
            ui->textEdit->insertPlainText(" ");//每发送两个字符后添加一个空格
            ui->textEdit->moveCursor(QTextCursor::End);
        }
        */

        Node_str = QString("total lenght %1").arg(arry.length());
        ui->file_lenght->append(Node_str);
        ui->Open_Button->setText(tr("Canner"));
    }
    else if(ui->Open_Button->text()==tr("Canner"))
    {
        ui->textBrowser->clear();
        ui->textEdit->clear();
        ui->file_lenght->clear();
        ui->Open_Button->setText(tr("Open"));
    }

}


void MainWindow::on_Update_Button_clicked()//开始发送加载文件
{
    if(ui->Update_Button->text()==tr("Update"))//点击升级时应该先发一包数据用来APP复位，在发一包用来IAP记录length和crc校验信息，第三包进行升级app内容发布
    {
        timer1 = new QTimer(this);
        connect( timer1, SIGNAL( timeout() ), this, SLOT( stepOne() ) );
        timer1->start(1000);
        ui->Update_Button->setText(tr("Canner"));
    }
    else
    {
        timer1->stop();
        ui->Update_Button->setText(tr("Update"));
        ui->progressBar->setValue(0);//初始值
        ui->progressBar->setRange(0,100-1);//最大值
    }

}
void MainWindow::on_Open2_clicked()
{
    if(ui->Open2->text() == "start")
    {//开启定时器
        Update1 = new QTimer(this);
        connect( Update1, SIGNAL( timeout() ), this, SLOT( UpdateStart() ) );
        Update1->start(1000);
        ui->Open2->setText("stop");
    }
    else
    {
        ui->Open2->setText("start");
    }
}

void MainWindow::UpdateStart()
{
    static uint8_t start = 0;
    start++;
    Update_app_reset(start);
    if(start == 2)
    {
        start = 0;
        Update1->stop();//发送完成关闭定时器
    }
}

void MainWindow::Update_app_reset(char pre)
{
    QFile file(fileName);
    QByteArray to_bin;
    QByteArray to_binCount;
    char *Update;
    if(file.open(QIODevice::ReadOnly))
    {
        file.seek(0);//设置当前文件位置
        to_bin = file.read(16);
        if(ui->UpdatecomboBox->currentText() == tr("Update ID"))
        {
            if(!(ui->UpdatelineEdit->text().isEmpty()))
            {
                Node_str = QString("Qpdate=1134,Node=%1,Boot=%2,%3,").arg(ui->UpdatelineEdit->text()).arg(pre).arg(to_bin.length());
            }
            else
            {
                QMessageBox::critical(this,tr("Wanrring"),"Update ID not is Empty!!!");
            }
        }
        else if(ui->UpdatecomboBox->currentText() == tr("ALL"))
        {
            Node_str = QString("Qpdate=1134,Node=ffff,Boot=%2,%3,").arg(ui->UpdatelineEdit->text()).arg(pre).arg(to_bin.length());
        }
        to_binCount =Node_str.toLatin1() + to_bin;
        Update = to_binCount.data();
        if(ui->OPen_ChuanKou_Button->text()==tr("关闭串口"))
        {
            my_serialPort->write(Update,to_binCount.length());
        }
        else
        {
            QMessageBox::information(this,"提示","请打开串口");
        }
    }
    file.close();
}
void MainWindow::Update_app_info()
{
    QFile file(fileName);
    QByteArray to_bin;
    QByteArray to_binCount;
    char *Update;
    if(file.open(QIODevice::ReadOnly))
    {
        file.seek(0);//设置当前文件位置
        to_bin = file.read(16);
        if(ui->UpdatecomboBox->currentText() == tr("Update ID"))
        {
            if(!(ui->UpdatelineEdit->text().isEmpty()))
            {
                Node_str = QString("Qpdate=1234,Node=%1,Boot=%2,%3,").arg(ui->UpdatelineEdit->text()).arg(2).arg(to_bin.length());
            }
            else
            {
                QMessageBox::critical(this,tr("Wanrring"),"Update ID not is Empty!!!");
            }
        }
        else if(ui->UpdatecomboBox->currentText() == tr("ALL"))
        {
            Node_str = QString("Qpdate=1234,Node=0xffff,Boot=%2,%3,").arg(ui->UpdatelineEdit->text()).arg(2).arg(to_bin.length());
        }
        to_binCount =Node_str.toLatin1() + to_bin;
        Update = to_binCount.data();
        if(ui->OPen_ChuanKou_Button->text()==tr("关闭串口"))
        {
            my_serialPort->write(Update,to_binCount.length());
        }
        else
        {
            QMessageBox::information(this,"提示","请打开串口");
        }
    }
    file.close();
}

void MainWindow::stepOne()//进度条刷新
{//进行分包发送
    QFile file(fileName);
    QByteArray to_bin;
    QByteArray to_binCount;
    char *Update;

    if(file.open(QIODevice::ReadOnly))
    {
        file.seek(16 + (512*MD_Update_Flag));//设置当前文件位置
        to_bin =  file.read(512) ;//读加载文件指定大小
        if(ui->UpdatecomboBox->currentText() == tr("Update ID"))
        {
            if(!(ui->UpdatelineEdit->text().isEmpty()))
            {
                Node_str = QString("Qpdate=1234,Node=%1,Boot=%2,%3,").arg(ui->UpdatelineEdit->text()).arg((MD_Update_Flag + 3)).arg(to_bin.length());
            }
            else
            {
                QMessageBox::critical(this,tr("Wanrring"),"Update ID not is Empty!!!");
            }
        }
        else if(ui->UpdatecomboBox->currentText() == tr("ALL"))
        {
            Node_str = QString("Qpdate=1234,Node=FFFF,Boot=%1,%2,").arg((MD_Update_Flag + 3)).arg(to_bin.length());
        }
        to_binCount =Node_str.toLatin1() + to_bin;
        Update = to_binCount.data();
        if(ui->OPen_ChuanKou_Button->text()==tr("关闭串口"))
        {
            my_serialPort->write(Update,to_binCount.length());
        }
        MD_Update_Flag++;


    }

    if(ui->progressBar->value()+1 <= ui->progressBar->maximum())
    {
        ui->progressBar->setValue(ui->progressBar->value()+1);
    }
    else
    {
        ui->progressBar->setValue(ui->progressBar->minimum());
    }
}

void MainWindow::on_start_time_clicked()
{
    if(ui->start_time->text()==tr("start time"))
    {
        dateTime = QTime::currentTime();
        //防止刚打开这个软件，界面显示的是0，而不是具体的时间
        ui->SYSTime->display(dateTime.currentTime().toString("hh:mm:ss"));
        time2 = new QTimer(this);
        this->connect(time2, SIGNAL(timeout()), this, SLOT(countTime()));
        //每隔一秒，计数器触发一次
        time2->start(1000);
        ui->start_time->setText(tr("stop time"));
    }
    else
    {
        ui->start_time->setText(tr("start time"));
        time2->stop();
    }
}






void MainWindow::countTime()
{
    ui->SYSTime->display(dateTime.currentTime().toString("hh:mm:ss"));
}
void MainWindow::countNum(quint64 temp)
{
    ui->lcdNumber->display(int(temp));
}


void MainWindow::on_start_num_clicked()
{
    if(ui->start_num->text() == (tr("start_num")))
    {
        ui->lcdNumber->display(int(0));
        num=0;
        ui->start_num->setText(tr("stop_num"));
    }
    else
    {
        ui->start_num->setText(tr("start_num"));
    }
}

void MainWindow::on_Ver_Buttom_clicked()
{
    if(ui->Ver_Buttom->text() == tr("Version"))
    {
        ui->Ver_Buttom->setText(tr("clear"));
        Node_str = QString("Read=1234,Node=0000,Version=,");
        ui->Ver_textEdit->append(Node_str);
    }
    else
    {
        ui->Ver_textEdit->clear();
        ui->Ver_Buttom->setText(tr("Version"));
    }
}

void MainWindow::on_send1_clicked()
{
    if(ui->OPen_ChuanKou_Button->text()==tr("关闭串口"))
    {
        my_serialPort->write(ui->Ver_textEdit->toPlainText().toLatin1());
    }
    else
    {
        //QMessageBox::about(this,tr("提示信息"),tr("更新成功"));
        QMessageBox::information(this,"提示","请打开串口");
    }
}

void MainWindow::on_Wirte_Button_clicked()
{
    if(ui->Wirte_Button->text() == tr("Wirte"))
    {
        ui->Wirte_Button->setText(tr("clear"));
        Node_str = QString("Wirte=1234,Node=0000,");
        ui->Write_textEdit->append(Node_str);
    }
    else
    {
        ui->Write_textEdit->clear();
        ui->Wirte_Button->setText(tr("Wirte"));
    }
}

void MainWindow::on_send2_clicked()
{
    if(ui->OPen_ChuanKou_Button->text()==tr("关闭串口"))
    {
        my_serialPort->write(ui->Write_textEdit->toPlainText().toLatin1());
    }
    else
    {
        //QMessageBox::about(this,tr("提示信息"),tr("更新成功"));
        QMessageBox::information(this,"提示","请打开串口");
    }
}

void MainWindow::on_Read_Button_clicked()
{
    if(ui->Read_Button->text() == tr("Read"))
    {
        ui->Read_Button->setText(tr("clear"));
        Node_str = QString("Read=1234,Node=0000,");
        ui->Read_textEdit->append(Node_str);
    }
    else
    {
        ui->Read_textEdit->clear();
        ui->Read_Button->setText(tr("Read"));
    }
}

void MainWindow::on_send3_clicked()
{
    if(ui->OPen_ChuanKou_Button->text()==tr("关闭串口"))
    {
        my_serialPort->write(ui->Read_textEdit->toPlainText().toLatin1());
    }
    else
    {
        //QMessageBox::about(this,tr("提示信息"),tr("更新成功"));
        QMessageBox::information(this,"提示","请打开串口");
    }
}

