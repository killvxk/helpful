// low_integrity.cpp
//

#include "stdafx.h"
#include "Windows.h"
#include "sddl.h"

HANDLE hgToken;
HANDLE hgProcess;
int check_process;

// https://msdn.microsoft.com/en-us/library/bb625966.aspx
void ShowTokenIntegrityLevel()
{
	DWORD dwLength;
	DWORD dwErr = ERROR_SUCCESS;
	PTOKEN_MANDATORY_LABEL pTIL = NULL;
	DWORD dwIntegLevel;

	// Get the token from current process, otherwise use the token
	if (check_process == 1) {
		hgProcess = GetCurrentProcess();
		OpenProcessToken(hgProcess, TOKEN_QUERY | TOKEN_QUERY_SOURCE, &hgToken);
	}

	// Get the Integrity level.
	if (!GetTokenInformation(hgToken, TokenIntegrityLevel, NULL, 0, &dwLength))
	{
		dwErr = GetLastError();
		if (dwErr == ERROR_INSUFFICIENT_BUFFER)
		{
			pTIL = (PTOKEN_MANDATORY_LABEL)LocalAlloc(0, dwLength);
			if (pTIL != NULL)
			{
				if (GetTokenInformation(hgToken, TokenIntegrityLevel, pTIL, dwLength, &dwLength))
				{
					dwIntegLevel = *GetSidSubAuthority(pTIL->Label.Sid,
						(DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1));

					if (dwIntegLevel < SECURITY_MANDATORY_MEDIUM_RID)
					{
						// Low Integrity
						wprintf(L"Low Integrity Token\n");
					}
					else if (dwIntegLevel >= SECURITY_MANDATORY_MEDIUM_RID &&
						dwIntegLevel < SECURITY_MANDATORY_HIGH_RID)
					{
						// Medium Integrity
						wprintf(L"Medium Integrity Token\n");
					}
					else if (dwIntegLevel >= SECURITY_MANDATORY_HIGH_RID)
					{
						// High Integrity
						wprintf(L"High Integrity Token\n");
					}
					else if (dwIntegLevel >= SECURITY_MANDATORY_SYSTEM_RID)
					{
						// System Integrity
						wprintf(L"System Integrity Token");
					}
				}
				LocalFree(pTIL);
			}
		}
	}
	//CloseHandle(hToken);
}


// https://msdn.microsoft.com/en-us/library/bb625960.aspx
int CreateLowProcess(LPWSTR wszProcessName)
{

	BOOL                  fRet;
	HANDLE                hToken = NULL;
	HANDLE                hNewToken = NULL;
	TOKEN_MANDATORY_LABEL TIL = { 0 };
	PROCESS_INFORMATION   ProcInfo = { 0 };
	STARTUPINFO           StartupInfo = { 0 };
	
	WCHAR wszIntegritySid[20] = L"S-1-16-4096"; // Low integrity SID
	PSID pIntegritySid = NULL;

	fRet = OpenProcessToken(GetCurrentProcess(),
		TOKEN_DUPLICATE |
		TOKEN_ADJUST_DEFAULT |
		TOKEN_QUERY |
		TOKEN_ASSIGN_PRIMARY,
		&hToken);

	if (!fRet) { goto CleanExit; }

	fRet = DuplicateTokenEx(hToken,
		0,
		NULL,
		SecurityImpersonation,
		TokenPrimary,
		&hNewToken);

	if (!fRet) { goto CleanExit; }

	fRet = ConvertStringSidToSid(wszIntegritySid, &pIntegritySid);

	if (!fRet) { goto CleanExit; }

	TIL.Label.Attributes = SE_GROUP_INTEGRITY;
	TIL.Label.Sid = pIntegritySid;


	// Set the process integrity level
	DWORD infolength = sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid);

	// Changing the sid of our duplicate token
	fRet = SetTokenInformation(hNewToken, TokenIntegrityLevel, &TIL, infolength);

	if (!fRet) { goto CleanExit; }

	// Check what level our new token is at!
	check_process = 0;

	// Set the token to our global variable!
	hgToken = hNewToken;
	ShowTokenIntegrityLevel();

	// Create the new process with our new token!
	fRet = CreateProcessAsUser(hNewToken,
		NULL,
		wszProcessName,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&StartupInfo,
		&ProcInfo);

// clean and exit
CleanExit:
	if (ProcInfo.hProcess != NULL) { CloseHandle(ProcInfo.hProcess); }
	if (ProcInfo.hThread != NULL) { CloseHandle(ProcInfo.hThread); }
	LocalFree(pIntegritySid);
	if (hNewToken != NULL) { CloseHandle(hNewToken); }
	if (hToken != NULL) { CloseHandle(hToken); }

	return 1;
}



int _tmain(int argc, LPWSTR argv[])
{
	if (argc < 2)
	{
		printf("Simple Low Integrity Tester\n"); 
		printf("start low_integrity.exe \"C:\\Windows\\System32\\cmd.exe\"\n");
		return 1;
	}
	LPWSTR wszProcessName = argv[1];
	printf("Attempting to start process in low integrity\n");
	// check the process token, otherwise use the token we specify
	check_process = 1;
	ShowTokenIntegrityLevel();
	CreateLowProcess(wszProcessName);
	return 0;
}

