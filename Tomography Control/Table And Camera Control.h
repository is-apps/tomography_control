
// Tomography Control.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

class CTableAndCameraControl
{
public:
	CTableAndCameraControl();
	void SendTableCommand(char* command);
};