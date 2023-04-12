#include <MobileDoasLib/Communication/SerialConnection.h>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <afxwin.h>         // MFC core and standard components


using namespace mobiledoas;

CSerialConnection::CSerialConnection() {
    memset(serbuf, 0, sizeof(serbuf));
    sprintf_s(this->serialPort, "COM0");
}

CSerialConnection::~CSerialConnection()
{
    this->Close();
}

CSerialConnection::CSerialConnection(CSerialConnection&& other)
{
    this->hComm = std::move(other.hComm);
    this->baudrate = other.baudrate;
    this->br1 = other.br1;
    this->br2 = other.br2;
    this->isRunning = std::move(other.isRunning);
    memcpy(this->serbuf, other.serbuf, sizeof(serbuf));
    this->serbufpt = std::move(other.serbufpt);
    memcpy(this->serialPort, other.serialPort, sizeof(serialPort));

    other.hComm = nullptr;
}

CSerialConnection& CSerialConnection::operator=(CSerialConnection&& other)
{
    this->hComm = std::move(other.hComm);
    this->baudrate = other.baudrate;
    this->br1 = other.br1;
    this->br2 = other.br2;
    this->isRunning = std::move(other.isRunning);
    memcpy(this->serbuf, other.serbuf, sizeof(serbuf));
    this->serbufpt = std::move(other.serbufpt);
    memcpy(this->serialPort, other.serialPort, sizeof(serialPort));

    other.hComm = nullptr;

    return *this;
}


void CSerialConnection::SetPort(int portNumber)
{
    // This construct makes it possible to handle COM-ports above 9
    if (portNumber > 9)
    {
        sprintf_s(this->serialPort, "\\\\.\\COM%d", portNumber);
    }
    else
    {
        sprintf_s(this->serialPort, "COM%d", portNumber);
    }
}

void CSerialConnection::SetPort(const std::string& port)
{
    int portNumber;
    sscanf_s(port.c_str(), "COM%d", &portNumber);
    this->SetPort(portNumber);
}

//-----------------------------------------------------------------
bool CSerialConnection::Init(int portNumber, long speed)
{
    this->SetPort(portNumber);

    this->baudrate = speed;

    return this->Init();
}

bool CSerialConnection::Init(long speed)
{
    this->baudrate = speed;

    return this->Init();
}

bool CSerialConnection::Init()
{
    if (nullptr != hComm)
    {
        this->Close();
    }

    DCB dcb;

    hComm = CreateFileA(this->serialPort,
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_FLAG_WRITE_THROUGH,
        0);

    if (hComm == INVALID_HANDLE_VALUE)
    {
        //CString msg;
        //msg.Format("Could not open serial port: %s", serialPort);
        //MessageBox(NULL, msg,  "Error", MB_OK);
        hComm = nullptr;
        return false;
    }

    FillMemory(&dcb, sizeof(dcb), 0);

    if (!GetCommState(hComm, &dcb))
    {
        hComm = nullptr;
        return false;
    }

    dcb.BaudRate = this->baudrate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fAbortOnError = FALSE;
    dcb.fDsrSensitivity = 0;
    dcb.fInX = 0;
    dcb.fRtsControl = 0;
    dcb.fDtrControl = 0;

    if (!SetCommState(hComm, &dcb))
    {
        hComm = nullptr;
        MessageBox(NULL, TEXT("Error in SetCommState!"), NULL, MB_OK);
        return false;
    }

    return true;
}

bool CSerialConnection::Check(long timeOut)
{
    DWORD nofBytesRead = 0;
    COMMTIMEOUTS timeouts;
    if (serbufpt)
    {
        // There's more data to read out from this buffer
        return true;
    }

    GetCommTimeouts(hComm, &timeouts);

    timeouts.ReadIntervalTimeout = MAXWORD;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.ReadTotalTimeoutConstant = timeOut;

    if (SetCommTimeouts(hComm, &timeouts) == 0)
    {
        std::cerr << " Error reading from serial connection, could not set communication timeouts." << std::endl;
        return false;
    }

    if (FALSE == ReadFile(hComm, serbuf, 1, &nofBytesRead, NULL))
    {
        DWORD errorCode = GetLastError();
        std::cerr << " Error reading from serial connection, last error was: " << errorCode << std::endl;
        return false;
    }

    if (nofBytesRead == 0)
    {
        return false;
    }

    serbufpt += nofBytesRead;

    return true;
}
//-----------------------------------------------------------------
void CSerialConnection::Write(void* ptTxt, long byteNum)
{
    DWORD dwWritten;
    WriteFile(hComm, ptTxt, byteNum, &dwWritten, NULL);
}

//-----------------------------------------------------------------
long CSerialConnection::Read(void* ptBuf, long byteNum)
{
    char* bp;
    long lreal;
    COMMTIMEOUTS timeouts;
    DWORD dwRead;
    bp = (char*)ptBuf;
    for (lreal = 0; lreal < byteNum && serbufpt; lreal++)
    {
        bp[lreal] = serbuf[lreal];
        serbufpt--;
    }
    if (byteNum == lreal) return(lreal);

    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    int ret = SetCommTimeouts(hComm, &timeouts);
    if (0 == ret || (nullptr != isRunning && *isRunning == false)) {
        //	  MessageBox(NULL,TEXT("Error setting time-outs in ReadSerial."),TEXT("ERROR"),MB_OK);
        return 0;
    }

    ReadFile(hComm, &bp[lreal], byteNum - lreal, &dwRead, NULL);
    lreal += dwRead;
    return(lreal);

}
//-----------------------------------------------------------------
void CSerialConnection::FlushSerialPort(long timeOut)
{
    char txt[1];
    while (this->Check(timeOut))
    {
        this->Read(&txt, 1);
    }
}
//-----------------------------------------------------------------
void CSerialConnection::Close()
{
    if (hComm != nullptr)
    {
        int ret = CloseHandle(hComm);
        if (ret == 0)
        {
            // failure, the the reason for the error
            DWORD errorCode = GetLastError();
            std::cerr << " Failed to close the serial connection, errorcode was: " << errorCode << std::endl;
        }
        hComm = nullptr;
    }
}

int CSerialConnection::InitCommunication()
{
    switch (baudrate)
    {
    case 2400:
        br1 = 0;
        br2 = 2400;
        break;
    case 4800:
        br1 = 1;
        br2 = 4800;
        break;
    case 9600:
        br1 = 2;
        br2 = 9600;
        break;
    case 19200:
        br1 = 3;
        br2 = 19200;
        break;
    case 38400:
        br1 = 4;
        br2 = 38400;
        break;
    case 57600:
        br1 = 5;
        br2 = 57600;
        break;
    case 115200:
        br1 = 6;
        br2 = 115200;
        break;
    default:
    {
        MessageBox(NULL, TEXT("Invalid baudrate"), TEXT("Error"), MB_OK);
        return 1;
    }

    }
    if (ChangeBaudRate())
    {
        MessageBox(NULL, TEXT("can not change baudrate"), TEXT("Error"), MB_OK);
        return(1);
    }
    return(0);
}

//-----------------------------------------------------------------
int CSerialConnection::ChangeBaudRate()
{
    char txt[256];
    char msg[100];
    long stat;

    do
    {
        stat = 1;
        if (ResetSpectrometer(115200))
            if (ResetSpectrometer(57600))
                if (ResetSpectrometer(38400))
                    if (ResetSpectrometer(19200))
                        if (ResetSpectrometer(9600))
                            if (ResetSpectrometer(4800))
                                if (ResetSpectrometer(2400))
                                {
                                    MessageBox(NULL, TEXT("Could not reset spectrometer"), NULL, MB_OK);
                                    stat = 0;
                                    return 1;
                                }

        if (stat)
            MessageBox(NULL, TEXT("Got contact with spectrometer"), TEXT("Notice"), MB_OK);

        txt[0] = 'b';		//Set binary mode
        txt[1] = 'B';
        this->Write(txt, 2);

        if (this->Check(200))
            this->Read(&txt, 1);
        else
        {
            MessageBox(NULL, TEXT("TIMEOUT-bB"), NULL, MB_OK);
            return 1;
        }

        txt[0] = 'K';		// Change Baud Rate to br1
        txt[1] = 0;
        txt[2] = (char)br1;
        this->Write(txt, 3);

        if (this->Check(200))
        {
            this->Read(txt, 1);
            if (txt[0] != 0x06)
            {
                MessageBox(NULL, TEXT("NAK1"), NULL, MB_OK);
                return 1;
            }
        }
        else
        {
            MessageBox(NULL, TEXT("Timeout 2"), NULL, MB_OK);
            return 1;
        }

        this->Close();

        if (!this->Init(br2))
        {
            this->CloseAll();	//Initialize serial communication with baudrate br2.     
            return 1;
        }
        txt[0] = 'K';
        txt[1] = 0;
        txt[2] = (char)br1;
        this->Write(txt, 3);

        stat = 0;
        while (this->Check(1000))
        {
            this->Read(txt, 1);
            if (txt[0] != 0x06)
            {
                sprintf_s(msg, "NAK2 %d", txt[0]);
                MessageBoxA(NULL, msg, "Notice", MB_OK);
                return 1;
            }
            else
                stat = 1;
        }

        if (stat == 0)
        {
            MessageBox(NULL, TEXT("Timeout 3"), TEXT("Notice"), MB_OK);
            return 1;
        }
    } while (stat == 0);

    return(0);
}

void CSerialConnection::CloseAll()
{
    this->Close();
    MessageBox(NULL, TEXT("communication problem, restart data collection"), TEXT("Error"), MB_OK);
}

//-----------------------------------------------------------------
int CSerialConnection::ResetSpectrometer(long speed)
{
    char txt[10], cmd[10];

    cmd[0] = 'Q';	//set all operating parameters to their default values

    if (!this->Init(speed))
    {
        this->CloseAll();
        return 0;
    }

    while (this->Check(10))
        this->Read(txt, 1);

    this->Write(&cmd, 1);

    while (this->Check(100))
        this->Read(&txt[0], 1);

    if (txt[0] == 0x06)  // spectrometer replies 'ACK'
        return(0);

    if (txt[0] == 21)	//spectrometer replies 'NAK'
        return(0);

    this->Close();

    return(1);
}
