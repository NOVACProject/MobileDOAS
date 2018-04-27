#pragma once

class CSerialConnection
{
public:
	CSerialConnection(void);
	~CSerialConnection(void);

	bool	Init(long speed);
	int		Check(long timeOut);
	void	Write(void *ptTxt,long byteNum);
	long	Read(void *ptBuf,long byteNum);
	void	FlushSerialPort(long timeOut);
	void	Close();
	int		InitCommunication();
	int		ChangeBaudRate();
	void	CloseAll();
	int		ResetSpectrometer(long speed);

	/* data definition */
	char	serbuf[10];			// the serial communication buffer
	int		serbufpt;			// current character is serbuf[serbufpt]

	char serialPort[20];
	long m_delay;				/* communication delay */

	long baudrate;				// spectrometer baudrate
	long sysBaud;				// system baudrate

	long	br2;
	long	br1;

	volatile bool* isRunning;	/* pointer to spectrometer->runFlag */
	CString *statusMsg;			/* pointer to spectrometer->m_statusMsg */
//	ErrorLogHandler errorLog;	/* same error handler as spectrometer uses */

	int startChn, stopChn;

	HANDLE	hComm;

private:

};


