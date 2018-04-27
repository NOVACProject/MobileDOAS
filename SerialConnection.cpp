

#include "stdafx.h"
#include "serialconnection.h"

CSerialConnection::CSerialConnection(void)
{
	baudrate = 57600;
	serbufpt = 0;
	hComm = nullptr;
}

CSerialConnection::~CSerialConnection(void)
{
}


//-----------------------------------------------------------------
bool CSerialConnection::Init(long speed){
	DCB dcb;
	int portNumber;

	// To be able to handle COM-ports above 9
	sscanf(serialPort, "COM%d", &portNumber);
	if(portNumber > 9){
		sprintf(serialPort, "\\\\.\\COM%d", portNumber);
	}

	hComm = CreateFile( serialPort,  
					GENERIC_READ | GENERIC_WRITE, 
					0, 
					0, 
					OPEN_EXISTING,
					FILE_FLAG_WRITE_THROUGH,
					0);
	if (hComm == INVALID_HANDLE_VALUE){
		//CString msg;
		//msg.Format("Could not open serial port: %s", serialPort);
		//MessageBox(NULL, msg,  "Error", MB_OK);
		return FALSE;
	}

	FillMemory(&dcb, sizeof(dcb), 0);

	if (!GetCommState(hComm, &dcb))     // get current DCB
		return FALSE;

	dcb.BaudRate = speed;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.fAbortOnError = FALSE;
	dcb.fDsrSensitivity=0;
	dcb.fInX=0;
	dcb.fRtsControl=0;
	dcb.fDtrControl=0;

	if (!SetCommState(hComm, &dcb)){
		MessageBox(NULL,TEXT("Error in SetCommState!"),NULL,MB_OK);
		return FALSE;
	}

	return TRUE;
}

int CSerialConnection::Check(long timeOut)
{

	DWORD dwRead;
	COMMTIMEOUTS timeouts;
	if(serbufpt) return(1);
	  
	GetCommTimeouts(hComm,&timeouts);

	timeouts.ReadIntervalTimeout = MAXWORD; 
	timeouts.ReadTotalTimeoutMultiplier = 1;
	timeouts.ReadTotalTimeoutConstant = timeOut;

	if (SetCommTimeouts(hComm, &timeouts)==0){
		//	  MessageBox(NULL,TEXT("Error setting time-outs in serial.Check."),TEXT("ERROR"),MB_OK);
		return 0;
	}

	ReadFile(hComm, serbuf, 1, &dwRead, NULL);
	if(dwRead==0) return(0);
	serbufpt+=dwRead;

	return(1);
}
//-----------------------------------------------------------------
void CSerialConnection::Write(void *ptTxt,long byteNum)
{
	DWORD dwWritten;
	WriteFile(hComm, ptTxt, byteNum, &dwWritten, NULL);
}

//-----------------------------------------------------------------
long CSerialConnection::Read(void *ptBuf,long byteNum)
{
	char *bp;
	long lreal;
	COMMTIMEOUTS timeouts;
	DWORD dwRead;
	bp=(char*)ptBuf;
	for(lreal=0;lreal<byteNum && serbufpt;lreal++)
		{       
			bp[lreal]=serbuf[lreal];
			serbufpt--;
		}
	if(byteNum==lreal) return(lreal);
	  
	timeouts.ReadIntervalTimeout = MAXDWORD; 
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	if ((!SetCommTimeouts(hComm, &timeouts)) && isRunning) {
		//	  MessageBox(NULL,TEXT("Error setting time-outs in ReadSerial."),TEXT("ERROR"),MB_OK);
		return 0;
	}

	ReadFile(hComm, &bp[lreal], byteNum-lreal, &dwRead, NULL);
	lreal+=dwRead;
	return(lreal);

}
//-----------------------------------------------------------------
void CSerialConnection::FlushSerialPort(long timeOut)
{
	char txt[1];
	while(this->Check(timeOut))
		this->Read(&txt,1);
}
//-----------------------------------------------------------------
void CSerialConnection::Close()
{
	if(hComm != nullptr)
		CloseHandle(hComm);
}

int CSerialConnection::InitCommunication()
{
	switch(baudrate)
	{
		case 2400:
			br1=0;
			br2=2400;
			break;
		case 4800:
			br1=1;
			br2=4800;
			break;
		case 9600:
			br1=2;
			br2=9600;
			break;
		case 19200:
			br1=3;
			br2=19200;
			break;
		case 38400:
			br1=4;
			br2=38400;
			break;
		case 57600:
			br1=5;
			br2=57600;
			break;
		case 115200:
			br1=6;
			br2=115200;
			break;
		default:
			{
				MessageBox(NULL,TEXT("Invalid baudrate"),TEXT("Error"),MB_OK);
				return 1;
			}
		
	}
	if(ChangeBaudRate()) 
	{
		MessageBox(NULL,"can not change baudrate",TEXT("Error"),MB_OK);
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
		   stat=1;
		  if(ResetSpectrometer(115200))
			if(ResetSpectrometer(57600))
			  if(ResetSpectrometer(38400))
				if(ResetSpectrometer(19200))
					if(ResetSpectrometer(9600))
						if(ResetSpectrometer(4800))
							if(ResetSpectrometer(2400))
							{
								MessageBox(NULL,TEXT("Could not reset spectrometer"),NULL,MB_OK);
								stat=0;
								return 1;
							}	  		

		if(stat)  
			MessageBox(NULL,TEXT("Got contact with spectrometer"),TEXT("Notice"),MB_OK);

		txt[0]='b';		//Set binary mode
		txt[1]='B';
		this->Write(txt,2);

		if(this->Check(200))
			this->Read(&txt,1);
		else
		{
			MessageBox(NULL,TEXT("TIMEOUT-bB"),NULL,MB_OK);
			return 1;
		}

		txt[0] = 'K';		// Change Baud Rate to br1
		txt[1] = 0;
		txt[2] = (char)br1;
		this->Write(txt,3);

		if(this->Check(200))
		{
			this->Read(txt,1);
			if(txt[0]!=0x06)
			{
				MessageBox(NULL,TEXT("NAK1"),NULL,MB_OK);
				return 1;
			}
		}
		else
		{
			MessageBox(NULL,TEXT("Timeout 2"),NULL,MB_OK);
			return 1;
		}

		this->Close();  

		if(!this->Init(br2))
		{
			this->CloseAll();	//Initialize serial communication with baudrate br2.     
			return 1;
		}
		txt[0] = 'K';
		txt[1] = 0;
		txt[2] = (char)br1;
		this->Write(txt,3);

		stat=0;
		while(this->Check(1000))
		{
			this->Read(txt,1);
			if(txt[0]!=0x06)
			{
				sprintf(msg,"NAK2 %d",txt[0]);
				MessageBox(NULL,msg,TEXT("Notice"),MB_OK);
				return 1;
			}
			else
				stat=1;      
		}

		if(stat==0)
		{
			MessageBox(NULL,"Timeout 3",TEXT("Notice"),MB_OK);
			return 1;
		}
	} while(stat==0);

	return(0);
}

void CSerialConnection::CloseAll()
{
	this->Close();
	MessageBox(NULL,TEXT("communication problem, restart data collection"),TEXT("Error"),MB_OK);
}

//-----------------------------------------------------------------
int CSerialConnection::ResetSpectrometer(long speed)
{
	char txt[10],cmd[10];

	cmd[0]='Q';	//set all operating parameters to their default values

	if(!this->Init(speed))
	{
		this->CloseAll();
		return 0;
	}

	while(this->Check(10))
		this->Read(txt,1);

	this->Write(&cmd,1);

	while(this->Check(100))
		this->Read(&txt[0],1);
  
	if(txt[0]==0x06)  // spectrometer replies 'ACK'
		return(0); 

	if(txt[0]==21)	//spectrometer replies 'NAK'
		return(0);

	this->Close();

	return(1);
}
