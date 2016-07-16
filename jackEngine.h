#ifndef JACK_ENGINE_H
#define JACK_ENGINE_H

#include <QObject>
#include <QStringList>
#include <jack/jack.h>
#include <QBasicTimer>
#include <QTimerEvent>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>

#define JACKENGINE_MIDI_OUT_PORT_NAME "midi_out"
#define JACKENGINE_DEFAULT_CLIENT_NAME "keytarboard"   // Default client name. Actual name is ourJackClientName, set in InitJackClient.

#define JACKENGINE_BUFFERSIZE 50
#define JACKENGINE_BUFFEREVENTSIZE 3

struct midiBufferEvent
{
    int len;
    unsigned char data[JACKENGINE_BUFFEREVENTSIZE];
};

struct midiBuffer
{
    midiBufferEvent data[JACKENGINE_BUFFERSIZE];
    int readindex;
    int writeindex;

    midiBuffer() : readindex(0), writeindex(0) {}

    // If the buffer is not empty, returns the size of the next readible event.
    // Returns zero if buffer is empty.
    int canRead()
    {
        if (readindex != writeindex) { return data[readindex].len; }
        else { return 0; }
    }

    // Reads an event data from the buffer to the specified buffer.
    // Returns the length of bytes read on success.
    // Returns zero if the buffer is empty.
    int read(unsigned char* retbuffer)
    {
        if (readindex != writeindex) {
            int len = data[readindex].len;
            memcpy(retbuffer, data[readindex].data, len);
            if (readindex >= JACKENGINE_BUFFERSIZE-1) { readindex = 0; }
            else { readindex++; }
            return len;
        } else { return 0; }
    }

    // Writes an event with specified number of bytes to buffer.
    // Returns size on success.
    // Returns zero if size is <= 0
    // Returns -1 if the buffer is full
    // Returns -2 if size is too big.
    int write(unsigned char* towrite, int size)
    {
        if (size <= 0) { return 0; }
        if (size > JACKENGINE_BUFFEREVENTSIZE) { return -2; }
        // Check if buffer is full
        int tempindex;
        if (writeindex >= JACKENGINE_BUFFERSIZE-1) { tempindex = 0; }
        else { tempindex = writeindex + 1; }
        if (tempindex == readindex) {
            // Buffer is full
            return -1;
        }
        data[writeindex].len = size;
        memcpy(data[writeindex].data, towrite, size);
        writeindex = tempindex;
        return size;
    }
};

class jackEngine : public QObject
{
    Q_OBJECT
public:
    explicit jackEngine(QObject *parent = 0);

    static void jackPortConnectCallback(jack_port_id_t a, jack_port_id_t b, int connect, void* arg);
    static void jackPortRegistrationCallback(jack_port_id_t port, int registered, void *arg);
    static int jackProcessCallback(jack_nframes_t nframes, void *arg);

    bool InitJackClient(QString name);
    void stopJackClient();
    bool clientIsActive();
    QString clientName();
    void pauseJackProcessing(bool pause);

    // JACK helper functions (Not specific to our client)
    QStringList getMidiInputPortsList();
    QStringList getMidiOutputPortsList();
    QStringList getAudioInputPortsList();
    QStringList getAudioOutputPortsList();

    void startJackTransport();
    void stopJackTransport();
    bool isTransportStopped();

    jack_client_t* client;
    jack_nframes_t nframes;
    jack_port_t* midi_output_port;

    midiBuffer buffer;

protected:
    bool clientActive;
    QString ourJackClientName;

    void error_abort(QString msg);

signals:
    void userMessage(QString msg);
    void JackPortsChanged();

    
public slots:
    int addMidiEventToBuffer(unsigned char* data, int size);
};

#endif // KONFYT_JACK_ENGINE_H
