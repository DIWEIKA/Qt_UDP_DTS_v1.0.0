#ifndef RAW_WAVE_WIDGET_H
#define RAW_WAVE_WIDGET_H

#include <QWidget>
#include "mainwindow.h"

using namespace std;

namespace Ui {
class raw_wave_widget;
}

class MainWindow;

class raw_wave_widget : public QWidget
{
    Q_OBJECT
public:
    raw_wave_widget();

    Ui::raw_wave_widget *ui;
    MainWindow* m_mainwindow;

    double* show_data;
    QCustomPlot* m_customPlot;

    void init_widget();

public slots:
    void display_wave(double* _displaydata);

signals:

private slots:
    void on_btn_reset_clicked();

};

#endif // RAW_WAVE_WIDGET_H