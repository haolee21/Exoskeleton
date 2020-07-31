#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QBoxLayout>
#include <QtCharts/QtCharts>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
#include <QtWidgets/QMenuBar>
#include "MainWindow.h"
#include "Connector.h"
int main(int argc, char *argv[ ])
{
    QApplication app(argc, argv);
    // QLabel *label = new QLabel("test");

    // // prepare some data for plotting
    QLineSeries* series = new QLineSeries();
    *series << QPointF(1, 5) << QPointF(3, 7) << QPointF(7, 6) << QPointF(9, 7) << QPointF(12, 6)
                << QPointF(16, 7) << QPointF(18, 5);
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Simple chart example");
    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setRange(0, 20);
    chart->axes(Qt::Vertical).first()->setRange(0, 10);
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);


    // QBoxLayout *buttomLay = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
    // QPushButton *but1 = new QPushButton("but1");
    // QPushButton *but2 = new QPushButton("but2");
    // buttomLay->addWidget(but1);
    // buttomLay->addWidget(but2);
    

    // QWidget *testWindow = new QWidget;
    // testWindow->setLayout(buttomLay);
    // testWindow->resize(500,600);
    // testWindow->show();
    QHBoxLayout *baseLayout = new QHBoxLayout();
    QWidget *baseWidget = new QWidget();
    Connector *conn = new Connector();
    MainWindow *mw = new MainWindow(conn);

    baseLayout->addWidget(mw);
    baseLayout->addWidget(chartView);
    baseWidget->setLayout(baseLayout);
    baseWidget->resize(1400, 800);
    baseWidget->show();

    // QBoxLayout *testLay = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
    // testLay->addWidget(&mw);
    // testLay->addWidget(test1);

    // QWidget *mainWidget = new QWidget();
    // mainWidget->setLayout(mainLayout);
    // mainWidget->show();
    return app.exec();
}
