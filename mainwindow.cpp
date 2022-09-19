#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
//    raw_data(new double[1250100]),
    m_demodulation(new demodulation(this)),
    m_temp_distance_save(new Temp_distance_save(m_demodulation)),
    m_udp_recv(new udp_recv(this)),
    m_udp_send(new udp_send(this))
//    m_hextodec(new HEXtoDEC(m_udp_recv))
{
    ui->setupUi(this);

    read_config();

//    set_style_sheet();

//    set_background_image();

    setWindowTitle(APP_TITLE);

    init_widget_temp_distance();
//    init_widget_max_temp();

    //���嶨ʱ��
    Temp_Display_Timer = new QTimer(); // �¶�-���벨��ͼ��ʾ��ʱ��
    Temp_Display_Timer->setTimerType(Qt::PreciseTimer);
    Temp_save_Timer = new QTimer(); // �¶�-���벨��ͼ�洢��ʱ��
    Temp_save_Timer->setTimerType(Qt::PreciseTimer);
    Alarm_Timer = new QTimer();//�¶ȹ��ȱ�����ʱ��
    Alarm_Timer->setTimerType(Qt::PreciseTimer);

    //��ʱ����ʱ��Ӧ
    connect(Temp_Display_Timer,&QTimer::timeout,this,&MainWindow::echarts_load_temp);
//    connect(Temp_save_Timer,&QTimer::timeout,this,&MainWindow::Open_Temp_Save_Thread);
//    connect(Alarm_Timer,&QTimer::timeout,this,&MainWindow::start_alarm);
    //�߳̽���ʱ��Ӧ
    connect(m_udp_recv,&udp_recv::finished,this,&MainWindow::FinishUdp_recvThread);
    connect(m_udp_send,&udp_send::finished,this,&MainWindow::FinishUdp_sendThread);
    connect(m_demodulation,&demodulation::finished,this,&MainWindow::FinishDemodulationThread);
    connect(m_temp_distance_save,&Temp_distance_save::finished,this,&MainWindow::FinishTemp_saveThread);

    start_detection();

//    m_udp_send->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//void MainWindow::paintEvent(QPaintEvent *){
//    label->resize(this->size());
//}

//��ȡ�����ļ�
void MainWindow::read_config()
{
    QSettings *settings = new QSettings("C:/Qt_UDP_DTS/config.ini",QSettings::IniFormat);

    //Read
    settings->beginGroup("MAIN");
    int APP_TITLE = settings->value("APP_TITLE","").toInt();
    qDebug()<<"APP_TITLE = "<<APP_TITLE<<endl;
    settings->endGroup();

    settings->beginGroup("RESOURCE");
//    BGD_IMAGE = settings->value("BGD_IMAGE","").toString();
//    qDebug()<<"BGD_IMAGE = "<<BGD_IMAGE<<endl;
    TEMP_DIST_HTML = settings->value("TEMP_DIST_HTML","").toString();
    qDebug()<<"TEMP_DIST_HTML = "<<TEMP_DIST_HTML<<endl;
    STYLE_SHEET = settings->value("STYLE_SHEET","").toString();
    qDebug()<<"STYLE_SHEET = "<<STYLE_SHEET<<endl;
    settings->endGroup();

    settings->beginGroup("DETECTION");
    int ALM_TEMP_THR = settings->value("ALM_TEMP_THR",-1).toInt();
    qDebug()<<"ALM_TEMP_THR = "<<ALM_TEMP_THR<<endl;
    settings->endGroup();

}

//����qt��ʽ��
void MainWindow::set_style_sheet()
{
    qDebug()<<QDir::currentPath()<<endl;
    QFile file(STYLE_SHEET);
    file.open(QFile::ReadOnly);
    QString styleSheet = tr(file.readAll());
    qApp->setStyleSheet(styleSheet);
}

//���ñ���ͼ
//void MainWindow::set_background_image()
//{
//    QPixmap Images(BGD_IMAGE);
//    label = new QLabel(this);
//    label->setScaledContents(true);
//    label->setPixmap(Images);
//    label->lower();
//}



//��ʼ��html
void MainWindow::init_widget_temp_distance()
{
    /*--����WebEngineView��html�ļ�--*/
    m_temp_distance_widget = ui->temp_distance_widget;
    m_temp_distance_widget->setContextMenuPolicy(Qt::NoContextMenu);
    m_temp_distance_widget->load(QUrl::fromLocalFile(/*QDir::currentPath()+*/TEMP_DIST_HTML));

    //QWebEngineView �� load ��һ��ҳ���ᷢ��һ������ɵ��ź�,���� QWebEngineView �Ĵ�С������ EChart �Ĵ�С��
    connect(m_temp_distance_widget,&QWebEngineView::loadFinished,this,&MainWindow::onResizeEcharts5);
}


//echarts��������
void MainWindow::echarts_load_temp()
{
    /*------����Temp[]--------*/
    //1.ʹ�� QJonsObject �����һ�� JSON ����
    QJsonObject seriesData;

    //2.ʹ�� QJsonArray �� JSON ���������һ����
    QJsonArray amplitude;
    for(int i=4; i<128; i++) {
        //������Ĳ������¶�ֵ��0 �򲻻����õ�
//        if(m_demodulation->Temp[i]==0) continue;

        //����¶�ֵ̫С���򲻻����õ�
//        if(m_demodulation->Temp[i]<-120) continue;

         //����¶�ֵ>300���򲻻����õ�
        if(m_demodulation->Temp[i]>250) continue;

        amplitude.push_back(m_demodulation->Temp[i]);
    }
    seriesData.insert("amplitude", amplitude);

    //3.ʹ�� QJsonDocument ���� JSON ����ת�����ַ���
    QString optionStr = QJsonDocument(seriesData).toJson();

    //4.����js��init2()����
    QString js = QString("init2(%1)").arg(optionStr);

    //5.���� QWebEngineView�� page()->runJavaScript("function(str)") ������ JS ����
    m_temp_distance_widget->page()->runJavaScript(js);

    /*--------����MAX_Temp-------*/
    //1.ʹ�� QJonsObject �����һ�� JSON ����
    QJsonObject temp_Obj;

    //2.ʹ�� QJsonArray �� JSON ���������һ����
    QJsonArray temp_json;
    temp_json.push_back(m_demodulation->MAX_Temp);

    temp_Obj.insert("temperature", temp_json);

    //3.ʹ�� QJsonDocument ���� JSON ����ת�����ַ���
    QString optionStr2 = QJsonDocument(temp_Obj).toJson();

    //4.����js��load2()����
    QString js2 = QString("load2(%1)").arg(optionStr2);

    //5.���� QWebEngineView�� page()->runJavaScript("function(str)") ������ JS ����
    m_temp_distance_widget->page()->runJavaScript(js2);

    /*-------����template_temp-------*/
    //1.ʹ�� QJonsObject �����һ�� JSON ����
    QJsonObject template_temp_obj;

    //2.ʹ�� QJsonArray �� JSON ���������һ����
    QJsonArray template_temp_json1, template_temp_json2, template_temp_json3,
            template_temp_json4, template_temp_json5;

//    for(int j=2; j<14;j+=3){
//        template_temp_json.push_back(m_demodulation->Temp[j]);
//    }
    template_temp_json1.push_back(m_demodulation->Temp[28]);
    template_temp_json2.push_back(m_demodulation->Temp[3]);
    template_temp_json3.push_back(m_demodulation->Temp[30]);
    template_temp_json4.push_back(m_demodulation->Temp[32]);
    template_temp_json5.push_back(m_demodulation->Temp[8]);

    template_temp_obj.insert("template_temp1", template_temp_json1);
    template_temp_obj.insert("template_temp2", template_temp_json2);
    template_temp_obj.insert("template_temp3", template_temp_json3);
    template_temp_obj.insert("template_temp4", template_temp_json4);
    template_temp_obj.insert("template_temp5", template_temp_json5);

    //3.ʹ�� QJsonDocument ���� JSON ����ת�����ַ���
    QString optionStr3 = QJsonDocument(template_temp_obj).toJson();

    //4.����js��load3()����
    QString js3 = QString("load3(%1)").arg(optionStr3);

    //5.���� QWebEngineView�� page()->runJavaScript("function(str)") ������ JS ����
    m_temp_distance_widget->page()->runJavaScript(js3);

//    //�¶ȹ��ȱ�����ʱ
//    if(m_demodulation->MAX_Temp >= ALARM_TEMP_THRESHOLD) start_alarm();
//    else {
//        if(Alarm_Timer->isActive()) Alarm_Timer->stop();
//        alarm_count = 0;
//    }
////    ui->lcd_alarm->display(QString("%1").arg(alarm_count));
}

void MainWindow::Open_Temp_Save_Thread()
{
//    ui->m_textBrowser->insertPlainText(QDateTime::currentDateTime().toString("yyyy-MMdd-hh:mm:ss ")+QString("Start Saving Data! ")+"\n");

    m_temp_distance_save->start();
}

void MainWindow::start_alarm()
{
    alarm_count++;
    //    ui->lcd_alarm->display(QString("%1").arg(alarm_count));
}

//��ʼ�������ݲ����
void MainWindow::start_detection()
{
    //��ʼ���������߳�
    m_udp_recv->start();

//    QThread::msleep(1000); //�ӳ�ms

//    m_hextodec->start();

//    QThread::msleep(1000); //�ӳ�ms

    //��ʼ��������߳�
    m_demodulation->start();

    //��ʱ������echarts_load_temp()��ʾ ms
    if(!Temp_Display_Timer->isActive()) Temp_Display_Timer->start(1000);
}


void MainWindow::FinishUdp_recvThread()
{
    m_udp_recv->quit();
    m_udp_recv->wait();
}

void MainWindow::FinishUdp_sendThread()
{
    m_udp_send->quit();
    m_udp_send->wait();
}


void MainWindow::FinishDemodulationThread()
{
    m_demodulation->quit();
    m_demodulation->wait();
}

void MainWindow::FinishTemp_saveThread()
{
    m_temp_distance_save->quit();
    m_temp_distance_save->wait();
}

//����Ӧ���ڱ仯
void MainWindow::onResizeEcharts5()
{
    isLoaded5 = true;
    QJsonObject sizeData;
    sizeData.insert("width", m_temp_distance_widget->width() - 20);
    sizeData.insert("height", m_temp_distance_widget->height() - 20);
    QString sizeStr = QJsonDocument(sizeData).toJson();
    QString js = QString("setSize(%1)").arg(sizeStr);
    m_temp_distance_widget->page()->runJavaScript(js);
}


//��Ϊ����ʵ���ڴ��ڸı��Сʱ ECharts Ҳ���ű仯����������Ҫ�� resizeEvent ʱ����ҲҪ���ô�С
void MainWindow::resizeEvent(QResizeEvent *event)
{
    if(isLoaded5) onResizeEcharts5();

}

//����趨���ֵ��Ѱ����ֵ
//void MainWindow::on_btn_threshold_clicked()
//{
//    m_demodulation->threshold = ui->edit_threshold->text().toInt();

//    QMessageBox::about(this,tr("Attention"),tr("Threshold Set Succeeded!"));
//}

