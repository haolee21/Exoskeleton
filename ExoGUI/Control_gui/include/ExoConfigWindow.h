#ifndef EXOCONFIGWINDOW_H
#define EXOCONFIGWINDOW_H
#include <QtCharts/QtCharts>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>

class ExoConfigWindow:QWidget
{
private:
    QChart *exoConfig;
    QLineSeries *exoConfig;

public:
    ExoConfigWindow(/* args */);
    ~ExoConfigWindow();
};

ExoConfigWindow::ExoConfigWindow(/* args */)
{
}

ExoConfigWindow::~ExoConfigWindow()
{
}
#endif