/* ----------------------------------------------------------
文件名称：WMI_DeviceQuery.h

功能描述：
	基于WMI获取设备属性：
		0：网卡原生MAC地址
		1：硬盘序列号
		2：主板序列号
		3：CPU ID
		4：BIOS序列号
		5：主板型号
		6：网卡当前MAC地址

接口函数：
	WMI_DeviceQuery
------------------------------------------------------------ */
#pragma once

#include <windows.h>

#ifndef MACRO_T_DEVICE_PROPERTY
	#define MACRO_T_DEVICE_PROPERTY

	#define PROPERTY_MAX_LEN	128	// 属性字段最大长度
	typedef struct _T_DEVICE_PROPERTY
	{
		TCHAR szProperty[PROPERTY_MAX_LEN];
	} T_DEVICE_PROPERTY;
#endif

#define WMI_QUERY_TYPENUM	7	// WMI查询支持的类型数

/*
功能：通过WMI获取设备属性
参数说明：
	iQueryType：需要查询的设备属性
			0：网卡原生MAC地址
			1：硬盘序列号
			2：主板序列号
			3：CPU ID
			4：BIOS序列号
			5：主板型号
			6：网卡当前MAC地址
	properties：存储设备属性值
	iSize：可存储的最大设备个数
返回值：
	 -1：不支持的设备属性值
	 -2：WMI连接失败
	 -3：不正确的WQL查询语句
	>=0：获取的设备个数	
*/
extern "C" __declspec(dllexport) INT __stdcall WMI_DeviceQuery( INT iQueryType, T_DEVICE_PROPERTY *properties, INT iSize );

