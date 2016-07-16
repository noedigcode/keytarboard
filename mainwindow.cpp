#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QApplication *appPtr, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->setVisible(false);
    ui->mainToolBar->setVisible(false);

    this->app = appPtr;

    setupKeyMap();

    userMessage("Keys are mapped like a guitar, with Shift, Caps Lock, Tab and tilde being the open strings E A D G.");
    userMessage("Space bar starts and stops Jack transport.");
    userMessage("----------------------------");

    if ( jack.InitJackClient("Keytarboard") ) {
        userMessage("Jack client started: " + jack.clientName());
        app->installEventFilter(this);
    } else {
        userMessage("ERROR starting Jack client.");
    }


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupKeyMap()
{
    int row1base = 52;
    int row2base = row1base + 5;
    int row3base = row2base + 5;
    int row4base = row3base + 5;

    keyMap.clear();

    int temp = row1base;
    keyMap.insert( Qt::Key_Shift, temp++ );
    keyMap.insert( Qt::Key_Z, temp++ );
    keyMap.insert( Qt::Key_X, temp++ );
    keyMap.insert( Qt::Key_C, temp++ );
    keyMap.insert( Qt::Key_V, temp++ );
    keyMap.insert( Qt::Key_B, temp++ );
    keyMap.insert( Qt::Key_N, temp++ );
    keyMap.insert( Qt::Key_M, temp++ );
    keyMap.insert( Qt::Key_Comma, temp++ );
    keyMap.insert( Qt::Key_Period, temp++ );
    keyMap.insert( Qt::Key_Slash, temp++ );

    temp = row2base;
    keyMap.insert( Qt::Key_CapsLock, temp++ );
    keyMap.insert( Qt::Key_A, temp++ );
    keyMap.insert( Qt::Key_S, temp++ );
    keyMap.insert( Qt::Key_D, temp++ );
    keyMap.insert( Qt::Key_F, temp++ );
    keyMap.insert( Qt::Key_G, temp++ );
    keyMap.insert( Qt::Key_H, temp++ );
    keyMap.insert( Qt::Key_J, temp++ );
    keyMap.insert( Qt::Key_K, temp++ );
    keyMap.insert( Qt::Key_L, temp++ );
    keyMap.insert( Qt::Key_Semicolon, temp++ );
    keyMap.insert( Qt::Key_Apostrophe, temp++ );

    temp = row3base;
    keyMap.insert( Qt::Key_Tab, temp++ );
    keyMap.insert( Qt::Key_Q, temp++ );
    keyMap.insert( Qt::Key_W, temp++ );
    keyMap.insert( Qt::Key_E, temp++ );
    keyMap.insert( Qt::Key_R, temp++ );
    keyMap.insert( Qt::Key_T, temp++ );
    keyMap.insert( Qt::Key_Y, temp++ );
    keyMap.insert( Qt::Key_U, temp++ );
    keyMap.insert( Qt::Key_I, temp++ );
    keyMap.insert( Qt::Key_O, temp++ );
    keyMap.insert( Qt::Key_P, temp++ );
    keyMap.insert( Qt::Key_BracketLeft, temp++ );
    keyMap.insert( Qt::Key_BracketRight, temp++ );
    keyMap.insert( Qt::Key_Backslash, temp++ );

    temp = row4base;
    keyMap.insert( Qt::Key_QuoteLeft, temp++ );
    keyMap.insert( Qt::Key_1, temp++ );
    keyMap.insert( Qt::Key_2, temp++ );
    keyMap.insert( Qt::Key_3, temp++ );
    keyMap.insert( Qt::Key_4, temp++ );
    keyMap.insert( Qt::Key_5, temp++ );
    keyMap.insert( Qt::Key_6, temp++ );
    keyMap.insert( Qt::Key_7, temp++ );
    keyMap.insert( Qt::Key_8, temp++ );
    keyMap.insert( Qt::Key_9, temp++ );
    keyMap.insert( Qt::Key_0, temp++ );
    keyMap.insert( Qt::Key_Minus, temp++ );
    keyMap.insert( Qt::Key_Equal, temp++ );
    keyMap.insert( Qt::Key_Backspace, temp++ );
}

void MainWindow::sendMidiEvent(unsigned char type, unsigned char data1, unsigned char data2)
{
    unsigned char event[3];
    event[0] = type;
    event[1] = data1;
    event[2] = data2;
    int count = 3;
    if (type == MIDI_EVENT_TYPE_PROGRAM) {
        count = 2;
    }
    if ( jack.addMidiEventToBuffer(event, count) == -1 ) {
        userMessage("Jack engine buffer full!");
    }

}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{


    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->isAutoRepeat()) { return true; }

        if (keyMap.contains(keyEvent->key())) {
            sendMidiEvent(MIDI_EVENT_TYPE_NOTEOFF, keyMap.value(keyEvent->key()), 0);
            return true;
        }

        return false;
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->isAutoRepeat()) { return true; }

        if (keyMap.contains(keyEvent->key())) {
            sendMidiEvent(MIDI_EVENT_TYPE_NOTEON, keyMap.value(keyEvent->key()), 80);
            return true;
        }

        if (keyEvent->key() == Qt::Key_Space) {
            if (jack.isTransportStopped()) {
                jack.startJackTransport();
            } else {
                jack.stopJackTransport();
            }
            return true;
        }

        // Unmanaged
        userMessage("Unmanaged key: " + keyEvent->text() + " (0x" + QString::number( keyEvent->key(), 16 ) + ")");
        return true;
    }

    if (event->type() == QEvent::MouseMove) {
        //userMessage("Mouse Move!");
    }
    return false;
}

void MainWindow::userMessage(QString msg)
{
    ui->textBrowser->append(msg);
}


