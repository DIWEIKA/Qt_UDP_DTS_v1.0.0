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
    connect(Alarm_Timer,&QTimer::timeout,this,&MainWindow::start_alarm);
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


//echarts�������� 1sˢ��һ��
/**
 * JSON�����ݲ���
 * 1.ʹ�� QJonsObject �����һ�� JSON ����
 * 2.ʹ�� QJsonArray �� JSON ���������һ����
 * 3.ʹ�� QJsonDocument ���� JSON ����ת�����ַ���
 * 4.����js��function()����
 * 5.���� QWebEngineView�� page()->runJavaScript("function(str)") ������ JS ����
*/
void MainWindow::echarts_load_temp()
{

//    /*------����Temp[]�¶�����--------*/
//    QJsonObject seriesData;
//    QJsonArray amplitude;

//    for(int i=1; i<15; i++) {
//        //������Ĳ������¶�ֵ��0 �򲻻����õ�
////        if(m_demodulation->Temp[i]==0) continue;

//        //����¶�ֵ̫С���򲻻����õ�
//        if(m_demodulation->Temp[i]<30) continue;

//         //����¶�ֵ̫���򲻻����õ�
//        if(m_demodulation->Temp[i]>250) continue;

//        amplitude.push_back(m_demodulation->Temp[i]);
//    }
//    seriesData.insert("amplitude", amplitude);

//    QString optionStr = QJsonDocument(seriesData).toJson();

//    QString js = QString("curve(%1)").arg(optionStr);

//    m_temp_distance_widget->page()->runJavaScript(js);





    QJsonObject template_temp_obj;
    QJsonArray template_temp_json1, template_temp_json2, template_temp_json3,
            template_temp_json4, template_temp_json5;

    int Temp_1,Temp_2,Temp_3,Temp_4,Temp_5;
    Temp_1 = 28;  //fake data
    Temp_2 = m_demodulation->Temp[33];
    Temp_3 = 31; //fake data
    Temp_4 = 26; //fake data
    Temp_5 = m_demodulation->Temp[37];



    QJsonObject temp_Obj; //��š�temperature��"region"������
    QJsonArray temp_json;

    //get the max of Temp, the region of the max Temp
    Temp_Max= Temp_1;

    vector<int>Temp_v{Temp_1,Temp_2,Temp_3,Temp_4,Temp_5};
    for(int m=0;m<5;++m){
        if(Temp_v[m]>Temp_Max){
            Temp_Max = Temp_v[m];
            max_region = m+1;
        }
    }

        /*--------����MAX_Temp-------*/
    temp_json.push_back(Temp_Max);

    temp_Obj.insert("temperature", temp_json);

     /*-------����template_temp�¶Ƚ�ģ-------*/

    template_temp_json1.push_back(Temp_1);
    template_temp_json2.push_back(Temp_2);
    template_temp_json3.push_back(Temp_3);
    template_temp_json4.push_back(Temp_4);
    template_temp_json5.push_back(Temp_5);

    template_temp_obj.insert("template_temp1", template_temp_json1);
    template_temp_obj.insert("template_temp2", template_temp_json2);
    template_temp_obj.insert("template_temp3", template_temp_json3);
    template_temp_obj.insert("template_temp4", template_temp_json4);
    template_temp_obj.insert("template_temp5", template_temp_json5);

    QString optionStr3 = QJsonDocument(template_temp_obj).toJson();

    QString js3 = QString("template(%1)").arg(optionStr3);

    m_temp_distance_widget->page()->runJavaScript(js3);

    /*-------����MAX_Region-------*/
    QJsonArray region_json;

    region_json.push_back(max_region);

    temp_Obj.insert("region", region_json);

    QString optionStr2 = QJsonDocument(temp_Obj).toJson();

    QString js2 = QString("highTemp(%1)").arg(optionStr2);

    m_temp_distance_widget->page()->runJavaScript(js2);
}


//���ȱ�������
void MainWindow::start_alarm()
{
    /*------�澯������״̬�£����¶ȸ���154�㿪ʼ��������3s���¶��Դ��ڣ���ʼ����------*/

    if(reset_flag==1 /*&& red_flag==0*/ && /*m_demodulation->MAX_Temp*/Temp_Max>=154){
        ++red_count;

        if(red_count>=3){
            red_flag = 1;
            reset_flag = 0;
        }
    }

    /*-------------����������״̬�£����¶�С��85�㿪ʼ��������3����¶���С�ڣ�������----------------*/

    else if(red_flag==1 /*&& reset_flag==0*/ && /*m_demodulation->MAX_Temp*/Temp_Max<85){

        ++reset_count;

        if(reset_count>=3){
            reset_flag = 1;
            red_flag = 0;
        }
    }

    /*-------------���������㱨��������������----------------*/
    else{
        red_count = 0;
        reset_count = 0;
    }

    /*--------����flag-------*/
    QJsonObject flag_Obj;
    QJsonArray flag_json1,flag_json2;

//    red_flag = 1;
//    reset_flag = 0;
    flag_json1.push_back(red_flag);
    flag_json2.push_back(reset_flag);

    flag_Obj.insert("red_flag", flag_json1);
    flag_Obj.insert("reset_flag", flag_json2);


    QString optionStr2 = QJsonDocument(flag_Obj).toJson();

    QString js_alarm = QString("baojing(%1)").arg(optionStr2);

    m_temp_distance_widget->page()->runJavaScript(js_alarm);
}

//��ʼ�������ݲ����
void MainWindow::start_detection()
{
    //��ʼ���������߳�
    m_udp_recv->start();

    //��ʼ��������߳�
    m_demodulation->start();

    //��ʱ������echarts_load_temp()��ʾ ms
    if(!Temp_Display_Timer->isActive()) Temp_Display_Timer->start(1000);

    //��ʱ������start_alarm()��ʾ ms
    if(!Alarm_Timer->isActive()) Alarm_Timer->start(1000);
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

