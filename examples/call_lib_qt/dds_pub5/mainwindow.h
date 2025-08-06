#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "RealtimeNetMiddleware.h"


#include <QThread>
#include <QString>
#include <QDebug>
#include <windows.h>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void write();
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
