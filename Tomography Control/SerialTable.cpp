#include "stdafx.h"

#include "Tomography Control.h"
#include "Tomography ControlDlg.h"
#include "Exceptions.h"

#define BUFFER_SIZE 2000

SerialTable::SerialTable(CWnd* wnd, LPCSTR gszPort) : Table(wnd)
{
	this -> m_inputEvent.ResetEvent();
	this -> m_inputBuffer.Empty();
	this -> m_outputBuffer.Empty();
	
    FillMemory(&this -> m_dcb, sizeof(this -> m_dcb), 0);
    FillMemory(&this -> m_commTimeouts, sizeof(this -> m_commTimeouts), 0);
    this -> m_dcb.DCBlength = sizeof(this -> m_dcb);

    this -> m_hComm = CreateFile( gszPort,  
                    GENERIC_READ | GENERIC_WRITE, 
                    0, 
                    NULL, 
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (this -> m_hComm == INVALID_HANDLE_VALUE) {
		throw new bad_serial_port_error("Could not open serial port.");
    }

	// Set up the port details
    if (!BuildCommDCB("baud=19200 parity=N data=8 stop=1", &this -> m_dcb)) {   
        // Couldn't build the DCB. Usually a problem
        // with the communications specification string.
        throw new bad_serial_port_error("Could not construct serial port configuration.");
    }
    else
	{
		// DCB is ready for use.
	}

	if (SetCommState(this -> m_hComm, &this -> m_dcb) == 0)
	{
        throw new bad_serial_port_error("Could not set port state.");
	}
	
	this -> m_commTimeouts.ReadIntervalTimeout = MAXDWORD;
	this -> m_commTimeouts.ReadTotalTimeoutMultiplier = 0;
	this -> m_commTimeouts.ReadTotalTimeoutConstant = 0;
	this -> m_commTimeouts.WriteTotalTimeoutMultiplier = 0;
	this -> m_commTimeouts.WriteTotalTimeoutConstant = 0;

	SetCommTimeouts(this -> m_hComm, &this -> m_commTimeouts);
	this -> Start();
}

SerialTable::~SerialTable() 
{
    CloseHandle(this -> m_hComm);
}

/* Get pending input from the table, and write new input out to it. */
void SerialTable::DoIO()
{
	DoWrite();
	DoRead();
}
	
void SerialTable::DoRead()
{
	DWORD bytesRead = 0;
	char tempBuffer[BUFFER_SIZE];
	
	tempBuffer[0] = NULL;
	
	if (!ReadFile(this -> m_hComm, tempBuffer, BUFFER_SIZE - 1, &bytesRead, NULL))
	{
		char errorBuffer[ERROR_BUFFER_SIZE];

		sprintf_s(errorBuffer, ERROR_BUFFER_SIZE - 1, "Error reading from table: %d", GetLastError());

		MessageBox(NULL, errorBuffer, "Tomography Control", MB_ICONERROR);
		return;
	}

	// Copy the incoming string to the output buffer
	if (bytesRead > 0)
	{
		// Lock the IO buffers while we exchange data with them
		this -> m_bufferLock.Lock();

		tempBuffer[bytesRead] = NULL;
		this -> m_outputBuffer += tempBuffer;
		
		this -> PulseMessageReceived();

		this -> m_bufferLock.Unlock();
	}
}

void SerialTable::DoWrite()
{
	// Lock the IO buffers while we exchange data with them
	this -> m_bufferLock.Lock();

	// If we have input to be sent, take a copy then release the locks on the buffers
	if (!this -> m_inputBuffer.IsEmpty())
	{
		LPCSTR input = this -> m_inputBuffer;
		DWORD bytesWritten;

		// Write the contents of the temp buffer out to serial
		WriteFile(this -> m_hComm, input, strlen(input), &bytesWritten, NULL);
		
		CString copiedChars = this -> m_inputBuffer.Left(bytesWritten);

		// Copy the input to the temporary buffer as if we'd just read it in
		this -> m_outputBuffer += copiedChars;

		// Clear the input buffer
		this -> m_inputBuffer.Delete(0, bytesWritten);

		this -> PulseMessageReceived();
	}

	this -> m_bufferLock.Unlock();
}