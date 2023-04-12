#pragma once

#include <string>

typedef void* HANDLE;

namespace mobiledoas {

    // CSerialConnection is a helper class for communicating to a device via an RS232 serial bus.
    // The current implementation is Windows specific.

    class CSerialConnection final
    {
    public:
        CSerialConnection();
        ~CSerialConnection();

        // --- Moving the CSerialConnection is allowed
        CSerialConnection(CSerialConnection&& other);
        CSerialConnection& operator=(CSerialConnection&& other);

        // --- This class manages a serial connection handle and is thus not copyable
        CSerialConnection(const CSerialConnection&) = delete;
        CSerialConnection& operator=(const CSerialConnection&) = delete;

        /** Initializes the serial port communication using the provided speed and port number.
            @param speed The baudrate to use.
            @param portNumber The index of the port to use (e.g. 0 for COM0 or 3 for COM3)
            @return true If the connection was successfully opened, otherwise false. */
        bool Init(int portNumber, long speed);

        /** Initializes the serial port communication using the provided speed and a previously set port number.
            @param speed The baudrate to use.
            @return true If the connection was successfully opened, otherwise false. */
        bool Init(long speed);

        /** Initializes the serial port communication using an already set port and baudrate.
            This may be called after calling Init() (and closing the connection) OR after calling
            SetBaudrate(...) and SetPort(...).
            @return true If the connection was successfully opened, otherwise false. */
        bool Init();

        /** Setting the baudrate */
        void SetBaudrate(long speed) { this->baudrate = speed; }

        /** Setting the com-port to use */
        void SetPort(int portNumber);
        void SetPort(const std::string& port);

        /** Getting the status */
        long GetBaudrate() const { return this->baudrate; }
        const char* GetPort() const { return this->serialPort; }

        /** Checks the serial port for more data.
            @param timeOut The serial read timeout, in milliseconds.
            @return true if there is more data to be read out from 'Read' */
        bool Check(long timeOut);

        void Write(void* ptTxt, long byteNum);
        long Read(void* ptBuf, long byteNum);
        void FlushSerialPort(long timeOut);
        void Close();

        /** InitCommunication sets up the parameters for the serial bus.
            @return zero if all is ok. */
        int  InitCommunication();

        int  ChangeBaudRate();
        void CloseAll();
        int  ResetSpectrometer(long speed);

        // ---------------- Status of the communication ----------------

        /* Pointer to a running flag (in spectrometer->runFlag).
            If this is set (!= nullptr) then this flag will be checked before reading
            the gps and the reading will cancelled if this flag is set to false. */
        volatile bool* isRunning = nullptr;

    private:
        // This is the handle to the serial port communication
        HANDLE hComm = nullptr;

        /* data definition */
        char serbuf[10];   // the serial communication buffer
        int  serbufpt = 0;   // current character is serbuf[serbufpt]

        char serialPort[20];

        long baudrate = 57600;    // spectrometer baudrate

        long br2 = 0;
        long br1 = 0;
    };

}

