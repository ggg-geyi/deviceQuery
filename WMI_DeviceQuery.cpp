#include "WMI_DeviceQuery.h"
//#include "UnmanagedCode.h"
#include <comutil.h>
#include <Wbemidl.h>
#include <tchar.h>
#include <strsafe.h>
#include <algorithm>
#include <atlconv.h>
#include <ntddndis.h>
#include <iostream>

using namespace std;

#pragma comment (lib, "comsuppw.lib")
#pragma comment (lib, "wbemuuid.lib")


typedef struct _T_WQL_QUERY
{
	CHAR*	szSelect;		// SELECT���
	WCHAR*	szProperty;		// �����ֶ�
} T_WQL_QUERY;

// WQL��ѯ���
const T_WQL_QUERY szWQLQuery[] = {

	// ����ԭ��MAC��ַ
	"SELECT * FROM Win32_NetworkAdapter WHERE (MACAddress IS NOT NULL) AND (NOT (PNPDeviceID LIKE 'ROOT%'))",
	L"PNPDeviceID",

	// Ӳ�����к�
	"SELECT * FROM Win32_DiskDrive WHERE (SerialNumber IS NOT NULL) AND (MediaType LIKE 'Fixed hard disk%')",
	L"SerialNumber",

	// �������к�
	"SELECT * FROM Win32_BaseBoard WHERE (SerialNumber IS NOT NULL)",
	L"SerialNumber",	

	// ������ID
	"SELECT * FROM Win32_Processor WHERE (ProcessorId IS NOT NULL)",
	L"ProcessorId",

	// BIOS���к�
	"SELECT * FROM Win32_BIOS WHERE (SerialNumber IS NOT NULL)",
	L"SerialNumber",

	// �����ͺ�
	"SELECT * FROM Win32_BaseBoard WHERE (Product IS NOT NULL)",
	L"Product",

	// ������ǰMAC��ַ
	"SELECT * FROM Win32_NetworkAdapter WHERE (MACAddress IS NOT NULL) AND (NOT (PNPDeviceID LIKE 'ROOT%'))",
	L"MACAddress",

	// 
	//"SELECT * FROM Win32_OperatingSystem WHERE (Name IS NOT NULL)",
	//L"Name",

};

// ͨ����PNPDeviceID����ȡ����ԭ��MAC��ַ
static BOOL WMI_DoWithPNPDeviceID( const TCHAR *PNPDeviceID, TCHAR *MacAddress, UINT uSize )
{
	TCHAR	DevicePath[MAX_PATH];
	HANDLE	hDeviceFile;	
	BOOL	isOK = FALSE;

	// �����豸·����
	StringCchCopy( DevicePath, MAX_PATH, TEXT("\\\\.\\") );
	StringCchCat( DevicePath, MAX_PATH, PNPDeviceID );
	StringCchCat( DevicePath, MAX_PATH, TEXT("#{ad498944-762f-11d0-8dcb-00c04fc3358c}") );

	// ����PNPDeviceID���еġ�\���滻�ɡ�#�����Ի���������豸·����
	std::replace( DevicePath + 4, DevicePath + 4 + _tcslen(PNPDeviceID), TEXT('\\'), TEXT('#') ); 

	// ��ȡ�豸���
	hDeviceFile = CreateFile( DevicePath,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if( hDeviceFile != INVALID_HANDLE_VALUE )
	{	
		ULONG	dwID;
		BYTE	ucData[8];
		DWORD	dwByteRet;		

		// ��ȡ����ԭ��MAC��ַ
		dwID = OID_802_3_PERMANENT_ADDRESS;
		isOK = DeviceIoControl( hDeviceFile, IOCTL_NDIS_QUERY_GLOBAL_STATS, &dwID, sizeof(dwID), ucData, sizeof(ucData), &dwByteRet, NULL );
		if( isOK )
		{	// ���ֽ�����ת����16�����ַ���
			for( DWORD i = 0; i < dwByteRet; i++ )
			{
				StringCchPrintf( MacAddress + (i << 1), uSize - (i << 1), TEXT("%02X"), ucData[i] );
			}
		}

		CloseHandle( hDeviceFile );
	}

	return isOK;
}

static BOOL WMI_DoWithHarddiskSerialNumber( TCHAR *SerialNumber, UINT uSize )
{
	UINT	iLen;
	UINT	i;

	iLen = _tcslen( SerialNumber );
	if( iLen == 40 )	// InterfaceType = "IDE"
	{	// ��Ҫ��16���Ʊ��봮ת��Ϊ�ַ���
		TCHAR ch, szBuf[32];
		BYTE b;		

		for( i = 0; i < 20; i++ )
		{	// ��16�����ַ�ת��Ϊ��4λ
			ch = SerialNumber[i * 2];
			if( (ch >= '0') && (ch <= '9') )
			{
				b = ch - '0';
			}
			else if( (ch >= 'A') && (ch <= 'F') )
			{
				b = ch - 'A' + 10;
			}
			else if( (ch >= 'a') && (ch <= 'f') )
			{
				b = ch - 'a' + 10;
			}
			else
			{	// �Ƿ��ַ�
				break;
			}

			b <<= 4;

			// ��16�����ַ�ת��Ϊ��4λ
			ch = SerialNumber[i * 2 + 1];
			if( (ch >= '0') && (ch <= '9') )
			{
				b += ch - '0';
			}
			else if( (ch >= 'A') && (ch <= 'F') )
			{
				b += ch - 'A' + 10;
			}
			else if( (ch >= 'a') && (ch <= 'f') )
			{
				b += ch - 'a' + 10;
			}
			else
			{	// �Ƿ��ַ�
				break;
			}

			szBuf[i] = b;
		}

		if( i == 20 )
		{	// ת���ɹ�
			szBuf[i] = L'\0';
			StringCchCopy( SerialNumber, uSize, szBuf );
			iLen = _tcslen( SerialNumber );
		}
	}

	// ÿ2���ַ�����λ��
	for( i = 0; i < iLen; i += 2 )
	{
		std::swap( SerialNumber[i], SerialNumber[i+1] );
	}

	// ȥ���ո�
	std::remove( SerialNumber, SerialNumber + _tcslen(SerialNumber) + 1, L' ' );

	return TRUE;
}

static BOOL WMI_DoWithProperty( INT iQueryType, TCHAR *szProperty, UINT uSize )
{
	BOOL isOK = TRUE;

	switch( iQueryType )
	{
	case 0:		// ����ԭ��MAC��ַ		
		isOK = WMI_DoWithPNPDeviceID( szProperty, szProperty, uSize );
		break;

	case 1:		// Ӳ�����к�
		isOK = WMI_DoWithHarddiskSerialNumber( szProperty, uSize );
		break;

	case 6:		// ������ǰMAC��ַ
		// ȥ��ð��
		std::remove( szProperty, szProperty + _tcslen(szProperty) + 1, L':' );
		break;

	default:
		// ȥ���ո�
		std::remove( szProperty, szProperty + _tcslen(szProperty) + 1, L' ' );
	}

	return isOK;
}

static BOOL isSupportDiskWMI()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO *)&os))
	{
		//vista֮ǰ��֧��
		if (os.dwMajorVersion <= 5)
		{
			return FALSE;
		}
	}
	return TRUE;
}

// ����Windows Management Instrumentation��Windows����淶��
// ����MSDN����
INT __stdcall WMI_DeviceQuery( INT iQueryType, T_DEVICE_PROPERTY *properties, INT iSize )
{
	////����COM���õķ���  HRESULT com��������
	HRESULT hres;
	INT	iTotal = 0;
	
	if (properties == NULL)
	{
		return -1;
	}

	// �жϲ�ѯ�����Ƿ�֧��
	if( (iQueryType < 0) || (iQueryType >= sizeof(szWQLQuery)/sizeof(T_WQL_QUERY)) )
	{
		cout << "Unsupport QueryType" << endl;
		return -1;	// ��ѯ���Ͳ�֧��
	}
	// Ӳ�����кŻ�ȡ���ò�ѯӲ���ķ�ʽ��ȡ,XP��֧��WMI��ʽ��ȡӲ����Ϣ
	/*
	if (iQueryType == 1 && !isSupportDiskWMI()) {
		DiskInfo handle = DiskInfo::GetDiskInfo();
		UINT count = handle.LoadDiskInfo();
		if (count > 0) {
			for (UINT i = 0; i < count; i++) {
				if (iTotal >= iSize)
				{
					break;
				}
				char* serialNumber = handle.SerialNumber(i);
				int iLength;
				iLength = MultiByteToWideChar(CP_ACP, 0, serialNumber, strlen(serialNumber) + 1, NULL, 0);
				MultiByteToWideChar(CP_ACP, 0, serialNumber, strlen(serialNumber) + 1, properties[iTotal].szProperty, iLength);
				iTotal++;
			}
		} else {
			UINT zeroRightCount = handle.m_serizalNoVec.size();
			for (UINT i = 0; i < zeroRightCount; i++) {
				if (iTotal >= iSize)
				{
					break;
				}
				char* serialNumber = handle.m_serizalNoVec[i];
				int iLength;
				iLength = MultiByteToWideChar(CP_ACP, 0, serialNumber, strlen(serialNumber) + 1, NULL, 0);
				MultiByteToWideChar(CP_ACP, 0, serialNumber, strlen(serialNumber) + 1, properties[iTotal].szProperty, iLength);
				iTotal++;
			}
		}
		return iTotal;
	}
   */
    // step1 ��ʼ��COM
    hres = CoInitializeEx( NULL, COINIT_MULTITHREADED ); 
    if( FAILED(hres) )
    {
		cout << "Failed to initialize COM library. Error code = 0x"
			<< hex << hres << endl;
        return -2;
    }

    // step2 ����COM�İ�ȫ��֤����
	hres = CoInitializeSecurity( 
		NULL, 
		-1, 
		NULL, 
		NULL, 
		RPC_C_AUTHN_LEVEL_DEFAULT, 
		RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
		);
	if( FAILED(hres) && hres != RPC_E_TOO_LATE)
    {
		cout << "Failed to initialize security. Error code = 0x"
			<< hex << hres << endl;
        CoUninitialize();
        return -2;
    }
    
	// step3 ���WMI����COM�ӿ�
	// ����һ��CLSID_WebmLocator����
    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance( 
		CLSID_WbemLocator,             
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator,
		reinterpret_cast<LPVOID*>(&pLoc)
		); 
    if( FAILED(hres) )
    {
		CoUninitialize();
        return -2;
    }

    // step4 ͨ�����ӽӿ�����WMI���ں˶�����"ROOT\\CIMV2"
	IWbemServices *pSvc = NULL;
	hres = pLoc->ConnectServer(
         _bstr_t( L"ROOT\\CIMV2" ),
         NULL,
         NULL,
         NULL,
         0,
         NULL,
         NULL,
         &pSvc
		 );    
    if( FAILED(hres) )
    {
		cout << "Could not connect. Error code = 0x"
			<< hex << hres << endl;
		pLoc->Release(); 
        CoUninitialize();
        return -2;
    }

	// step5 �����������İ�ȫ����
    hres = CoSetProxyBlanket(
		pSvc,
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		NULL,
		RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_NONE
		);
	if( FAILED(hres) )
    {
		cout << "Could not set proxy blanket. Error code = 0x"
			<< hex << hres << endl;
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
        return -2;
    }

    // step6 ͨ�������������WMI��������
    IEnumWbemClassObject *pEnumerator = NULL;
    hres = pSvc->ExecQuery(
		bstr_t("WQL"), 
		bstr_t( szWQLQuery[iQueryType].szSelect ),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &pEnumerator
		);
	if( FAILED(hres) )
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return -3;
    }
	
    // step7 ѭ��ö�����еĽ������
	IWbemClassObject *pclsObj = NULL;
	ULONG uReturn = 0;
    while( pEnumerator )
    {
		if( (properties != NULL) && (iTotal >= iSize) )
		{
			break;
		}
        pEnumerator->Next(
			WBEM_INFINITE,
			1, 
            &pclsObj,
			&uReturn
			);
        if( uReturn == 0 )
        {
            break;
        }
		if( properties != NULL )
		{	// ��ȡ����ֵ
			VARIANT vtProperty;
			//Variant ��һ��������������ͣ����˶���String���ݼ��û����������⣬���԰����κ���������ݡ�
			//Variant Ҳ���԰���Empty��Error��Nothing��Null������ֵ��
			//������VarType������TypeName������������δ��� Variant �е����ݡ�
			VariantInit( &vtProperty );	
			hres = pclsObj->Get( szWQLQuery[iQueryType].szProperty, 0, &vtProperty, NULL, NULL );
			if (FAILED(hres))
			{
				cout << "Could not Get szProperty. Error code = 0x"
					<< hex << hres << endl;
				//�ͷ���Դ
				CoUninitialize();
				return -2;
			}
			USES_CONVERSION; StringCchCopy( properties[iTotal].szProperty, PROPERTY_MAX_LEN, W2T(vtProperty.bstrVal) );  // ����vtProperty����Ϊbstr
			VariantClear( &vtProperty );

			// ������ֵ����һ���Ĵ���
			if( WMI_DoWithProperty( iQueryType, properties[iTotal].szProperty, PROPERTY_MAX_LEN ) )
			{
				iTotal++;
			}
		}
		else
		{
			iTotal++;
		}

		pclsObj->Release();
    } // End While

    // �ͷ���Դ
	pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
	
    return iTotal;
}
