#include "jackEngine.h"
#include <stdio.h>
#include <iostream>


bool jack_process_busy = false;
int jack_process_disable = 0; // Non-zero value = disabled


// ============================================================================
// gidJackClient class
// ============================================================================




jackEngine::jackEngine(QObject *parent) :
    QObject(parent)
{
    this->clientActive = false;
    buffer = midiBuffer();
}

void jackEngine::jackPortConnectCallback(jack_port_id_t a, jack_port_id_t b, int connect, void* arg)
{

}

void jackEngine::jackPortRegistrationCallback(jack_port_id_t port, int registered, void *arg)
{

}

int jackEngine::jackProcessCallback(jack_nframes_t nframes, void *arg)
{
    jack_process_busy = true;
    if (jack_process_disable) {
        jack_process_busy = false;
        return 0;
    }
    // --------------------------------

    jackEngine* e = (jackEngine*)arg;

    void* portBuffer = jack_port_get_buffer( e->midi_output_port, nframes );
    jack_midi_clear_buffer(portBuffer);

    // for each event in our buffer...
    for (int size=e->buffer.canRead(); size>0; size=e->buffer.canRead()) {

        // Get Jack output buffer
        jack_midi_data_t* out_buffer = jack_midi_event_reserve( portBuffer, 0, size);
        // Copy buffer data to output buffer
        e->buffer.read(out_buffer);

    }

    // --------------------------------
    jack_process_busy = false;
    return 0;
}

void jackEngine::startJackTransport()
{
    jack_transport_start(client);
}

void jackEngine::stopJackTransport()
{
    jack_transport_stop(client);
}

bool jackEngine::isTransportStopped()
{
    return jack_transport_query(client, NULL) == JackTransportStopped;
}


bool jackEngine::clientIsActive()
{
    return this->clientActive;
}

QString jackEngine::clientName()
{
    return ourJackClientName;
}

/* Pauses (pause=true) or unpauses (pause=false) the Jack process callback function.
 * When pause=true, the Jack process callback is disabled and this function blocks
 * until the current execution of the process callback (if any) competes, before
 * returning, thus ensuring that the process callback will not execute until this
 * function is called again with pause=false. */
void jackEngine::pauseJackProcessing(bool pause)
{
    /* When jack_process_disable is non-zero, Jack process is disabled. Here it is
     * incremented or decremented, accounting for the fact that this function might
     * be called multiple times within nested functions. Only once all of the callers
     * have also called this function with pause=false, will it reach zero, enabling
     * the Jack process callback again. */

    if (pause) {
        jack_process_disable++;
        while (jack_process_busy) {}
    } else {
        if (jack_process_disable) {
            jack_process_disable--;
        }
    }
}

bool jackEngine::InitJackClient(QString name)
{
    // Try to become a client of the jack server
    if ( (client = jack_client_open(name.toLocal8Bit(), JackNullOption, NULL)) == NULL) {
        userMessage("JACK: Error becoming client.");
        this->clientActive = false;
        return false;
    } else {
        ourJackClientName = jack_get_client_name(client); // jack_client_open modifies the given name if another client already uses it.
        userMessage("JACK: Client created: " + ourJackClientName);
        this->clientActive = true;
    }

    // Set up midi input port
    //setOurMidiInputPortName( ourJackClientName + ":" +
    //                        QString::fromLocal8Bit(GID_JACK_MIDI_IN_PORT_NAME));

    // Set up callback functions
    jack_set_port_connect_callback(client, jackEngine::jackPortConnectCallback, this);
    jack_set_port_registration_callback(client, jackEngine::jackPortRegistrationCallback, this);
    jack_set_process_callback (client, jackEngine::jackProcessCallback, this);


    // Set up output midi port
    midi_output_port = jack_port_register ( client,
                                           JACKENGINE_MIDI_OUT_PORT_NAME,
                                           JACK_DEFAULT_MIDI_TYPE,
                                           JackPortIsOutput, 0);
    nframes = jack_get_buffer_size(client);

    // Activate the client
    if (jack_activate(client)) {
        userMessage("JACK: Cannot activate client.");
        jack_free(client);
        this->clientActive = false;
        return false;
    } else {
        userMessage("JACK: Activated client.");
        this->clientActive = true;
    }

    return true;
}

void jackEngine::stopJackClient()
{
    if (clientIsActive()) {
        jack_client_close(client);
        this->clientActive = false;
    }
}

// Works exactly the same as the midiBuffer write function.
int jackEngine::addMidiEventToBuffer(unsigned char *data, int size)
{
    return buffer.write(data, size);
}


// Print error message to stdout, and abort app.
void jackEngine::error_abort(QString msg)
{
    std::cout << "\n" << "Konfyt ERROR, ABORTING: gidJackClient:" << msg.toLocal8Bit().constData();
    abort();
}

