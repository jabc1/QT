#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMainWindow>
#include "QTimer"
#include "qDebug"
#include "QFileDialog"
#include "QFile"
#include "QDateTime"
#include <QMessageBox>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow//
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_OPen_ChuanKou_Button_clicked();
    void readMyCom();//用于读取数据
    void on_Clear_Button_clicked();

    void on_pushButton_clicked();

    void on_Open_Button_clicked();

    void on_Open2_clicked();

    void on_Update_Button_clicked();
    void stepOne();//刷新精度条
    void on_start_time_clicked();
    void countTime();
    void countNum(quint64 temp);

    void Update_app_reset(char pre);//定时器1s
    void Update_app_info();//定时器1s
    void UpdateStart();

    void on_start_num_clicked();

    void on_Ver_Buttom_clicked();

    void on_send1_clicked();

    void on_Wirte_Button_clicked();

    void on_send2_clicked();

    void on_Read_Button_clicked();

    void on_send3_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *my_serialPort;//(实例化一个指向串口的指针，可以用于访问串口)
    QByteArray requestData;//（用于存储从串口那读取的数据）
    QTimer *timer;//（用于计时）
    QTimer *timer1;//（用于精度条）
    QTimer *time2;//用于LCD
    QTimer *Update1,*Update2;//用于升级使用
    QString fileName,Node_str;//用于文件路径加载
    QByteArray arry;
    unsigned int length,num,MD_Update_Flag;
    QTime dateTime;
};

#endif // MAINWINDOW_H



