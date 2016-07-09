#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QMap>
#include "jackEngine.h"
#include "midiDefines.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QApplication* appPtr, QWidget *parent = 0);
    ~MainWindow();

    void setupKeyMap();
    void sendMidiEvent(unsigned char type, unsigned char data1, unsigned char data2);
    bool eventFilter(QObject *object, QEvent *event);

    QApplication* app;

    void userMessage(QString msg);


private slots:


private:
    Ui::MainWindow *ui;
    jackEngine jack;

    QMap<int, int> keyMap;
};

#endif // MAINWINDOW_H
