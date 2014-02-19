#pragma once


// CRunProgressDlg dialog

class CRunProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRunProgressDlg)

public:
	CRunProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRunProgressDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_RUN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_currentPosition;
	CString m_imageFilename;
	int m_stopsMade;
	int m_stopsTotal;
	int m_turnsMade;
	int m_turnsTotal;
	CString m_calculatedAngle;
	CString m_startTime;
	CString m_estEndTime;
	CString m_estRunTime;

	// Called when the worker thread completes a rotation of the table
	afx_msg LRESULT OnTurnCompleted(WPARAM wParam, LPARAM lParam);

	// Called when the worker thread completes a set of image captures
	afx_msg LRESULT OnStopCompleted(WPARAM wParam, LPARAM lParam);

	// Called when the worker thread captures a single image
	afx_msg LRESULT OnImageCaptured(WPARAM wParam, LPARAM lParam);
};

// Tracks details of a data collection run task.
struct RunTask
{
	CWnd* m_dialog;
	CString m_baseFilename;
	int m_turnsTotal;
	int m_stopsPerTurn;
	int m_framesPerStop;
	int m_exposureTimeSeconds;
	BOOL m_running;
};

// Function for the manual camera worker thread
UINT takeRunImages( LPVOID pParam );