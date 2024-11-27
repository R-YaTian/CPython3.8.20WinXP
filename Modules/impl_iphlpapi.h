/*
 * wine iphlpapi dll implementation
 *
 * Copyright (C) 2003,2006 Juan Lang
 * Copyright 2021 Huw Davies
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/*
 * YY-Thunks iphlpapi dll implementation
 *
 * Copyright (c) 2018 Chuyu-Team
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdbool.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <netioapi.h>

#if !defined(__Comment_Lib_iphlpapi)
#define __Comment_Lib_iphlpapi
#pragma comment(lib, "Iphlpapi.lib")
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define NDIS_IF_MAX_STRING_SIZE 256

static unsigned __fastcall CharToHex(_In_ wchar_t _ch)
{
    if (_ch >= L'0' && _ch <= L'9')
    {
        return _ch - L'0';
    }
    else if (_ch >= L'A' && _ch <= L'F')
    {
        return _ch - L'A' + 0xA;
    }
    else if (_ch >= L'a' && _ch <= L'f')
    {
        return _ch - L'a' + 0xA;
    }
    else
    {
        return -1;
    }
}

static BOOL __fastcall StringToGuid(_In_z_ const wchar_t* _szInput, _Out_ GUID* _pId)
{
    memset(_pId, 0, sizeof(*_pId));

    if (!_szInput)
        return FALSE;

    if (*_szInput == L'{')
        ++_szInput;

#define _IS_HEX_CHAR(C) (CharToHex(C) != (unsigned)-1)
    if (!(_IS_HEX_CHAR(_szInput[0]) && _IS_HEX_CHAR(_szInput[1]) && _IS_HEX_CHAR(_szInput[2]) && _IS_HEX_CHAR(_szInput[3])
        && _IS_HEX_CHAR(_szInput[4]) && _IS_HEX_CHAR(_szInput[5]) && _IS_HEX_CHAR(_szInput[6]) && _IS_HEX_CHAR(_szInput[7])
        && _szInput[8] == L'-'
        && _IS_HEX_CHAR(_szInput[9]) && _IS_HEX_CHAR(_szInput[10]) && _IS_HEX_CHAR(_szInput[11]) && _IS_HEX_CHAR(_szInput[12])
        && _szInput[13] == L'-'
        && _IS_HEX_CHAR(_szInput[14]) && _IS_HEX_CHAR(_szInput[15]) && _IS_HEX_CHAR(_szInput[16]) && _IS_HEX_CHAR(_szInput[17])
        && _szInput[18] == L'-'
        && _IS_HEX_CHAR(_szInput[19]) && _IS_HEX_CHAR(_szInput[20]) && _IS_HEX_CHAR(_szInput[21]) && _IS_HEX_CHAR(_szInput[22])
        && _szInput[23] == L'-'
        && _IS_HEX_CHAR(_szInput[24]) && _IS_HEX_CHAR(_szInput[25]) && _IS_HEX_CHAR(_szInput[26]) && _IS_HEX_CHAR(_szInput[27])
        && _IS_HEX_CHAR(_szInput[28]) && _IS_HEX_CHAR(_szInput[29]) && _IS_HEX_CHAR(_szInput[30]) && _IS_HEX_CHAR(_szInput[31])
        && _IS_HEX_CHAR(_szInput[32]) && _IS_HEX_CHAR(_szInput[33]) && _IS_HEX_CHAR(_szInput[34]) && _IS_HEX_CHAR(_szInput[35])))
    {
        return FALSE;
    }
#undef _IS_HEX_CHAR

    _pId->Data1 = (CharToHex(_szInput[0]) << 28) | (CharToHex(_szInput[1]) << 24) | (CharToHex(_szInput[2]) << 20) | (CharToHex(_szInput[3]) << 16)
        | (CharToHex(_szInput[4]) << 12) | (CharToHex(_szInput[5]) << 8) | (CharToHex(_szInput[6]) << 4) | (CharToHex(_szInput[7]) << 0);

    _pId->Data2 = (CharToHex(_szInput[9]) << 12) | (CharToHex(_szInput[10]) << 8) | (CharToHex(_szInput[11]) << 4) | (CharToHex(_szInput[12]) << 0);

    _pId->Data3 = (CharToHex(_szInput[14]) << 12) | (CharToHex(_szInput[15]) << 8) | (CharToHex(_szInput[16]) << 4) | (CharToHex(_szInput[17]) << 0);

    _pId->Data4[0] = (CharToHex(_szInput[19]) << 4) | (CharToHex(_szInput[20]) << 0);
    _pId->Data4[1] = (CharToHex(_szInput[21]) << 4) | (CharToHex(_szInput[22]) << 0);
    _pId->Data4[2] = (CharToHex(_szInput[24]) << 4) | (CharToHex(_szInput[25]) << 0);
    _pId->Data4[3] = (CharToHex(_szInput[26]) << 4) | (CharToHex(_szInput[27]) << 0);
    _pId->Data4[4] = (CharToHex(_szInput[28]) << 4) | (CharToHex(_szInput[29]) << 0);
    _pId->Data4[5] = (CharToHex(_szInput[30]) << 4) | (CharToHex(_szInput[31]) << 0);
    _pId->Data4[6] = (CharToHex(_szInput[32]) << 4) | (CharToHex(_szInput[33]) << 0);
    _pId->Data4[7] = (CharToHex(_szInput[34]) << 4) | (CharToHex(_szInput[35]) << 0);
    return TRUE;
}

static void MIB_IFROW_To_MIB_IF_ROW2(_In_ const MIB_IFROW* _pRow, _Out_ MIB_IF_ROW2* _pRow2)
{
    memset(_pRow2, 0, sizeof(*_pRow2));

    _pRow2->InterfaceLuid.Info.NetLuidIndex = _pRow->dwIndex;
    _pRow2->InterfaceLuid.Info.IfType = _pRow->dwType;
    _pRow2->InterfaceIndex = _pRow->dwIndex;

    StringToGuid(wcsrchr(_pRow->wszName, L'{'), &_pRow2->InterfaceGuid);
    const size_t _cchDescr = min(sizeof(_pRow->bDescr), _pRow->dwDescrLen);
    for (size_t i = 0; i != _cchDescr; ++i)
    {
        _pRow2->Description[i] = _pRow->bDescr[i];
    }
    _pRow2->PhysicalAddressLength = _pRow->dwPhysAddrLen;
    memcpy(_pRow2->PhysicalAddress, _pRow->bPhysAddr, min(sizeof(_pRow->bPhysAddr), _pRow->dwPhysAddrLen));
    memcpy(_pRow2->PermanentPhysicalAddress, _pRow->bPhysAddr, min(sizeof(_pRow->bPhysAddr), _pRow->dwPhysAddrLen));

    _pRow2->Mtu = _pRow->dwMtu;
    _pRow2->Type = _pRow->dwType;

    if (IF_TYPE_SOFTWARE_LOOPBACK == _pRow->dwType)
    {
        _pRow2->MediaType = (NDIS_MEDIUM) 17;
        _pRow2->AccessType = NET_IF_ACCESS_LOOPBACK;
        _pRow2->DirectionType = NET_IF_DIRECTION_SENDRECEIVE;
    }
    else
    {
        // 设置一个默认值，都假装自己是普通的网口
        _pRow2->MediaType = NdisMedium802_3;
        _pRow2->AccessType = NET_IF_ACCESS_BROADCAST;
        _pRow2->DirectionType = NET_IF_DIRECTION_SENDRECEIVE;
    }

    switch (_pRow->dwOperStatus)
    {
    case IF_OPER_STATUS_OPERATIONAL:
    case IF_OPER_STATUS_CONNECTED:
        _pRow2->OperStatus = IfOperStatusUp;
        _pRow2->MediaConnectState = MediaConnectStateConnected;
        break;
    default:
        _pRow2->OperStatus = IfOperStatusDown;
        break;
    }

    _pRow2->AdminStatus = (NET_IF_ADMIN_STATUS)_pRow->dwAdminStatus;
    _pRow2->ConnectionType = NET_IF_CONNECTION_DEDICATED;

    _pRow2->TransmitLinkSpeed = _pRow->dwSpeed;
    _pRow2->ReceiveLinkSpeed = _pRow->dwSpeed;
    _pRow2->InOctets = _pRow->dwInOctets;
    _pRow2->InUcastPkts = _pRow->dwInUcastPkts;
    _pRow2->InNUcastPkts = _pRow->dwInNUcastPkts;
    _pRow2->InDiscards = _pRow->dwInDiscards;
    _pRow2->InErrors = _pRow->dwInErrors;
    _pRow2->InUnknownProtos = _pRow->dwInUnknownProtos;
    // 伪造一下 InUcastOctets
    _pRow2->InUcastOctets = _pRow->dwInOctets;
    _pRow2->InMulticastOctets = 0;
    _pRow2->InBroadcastOctets = 0;
    _pRow2->OutOctets = _pRow->dwOutOctets;
    _pRow2->OutUcastPkts = _pRow->dwOutUcastPkts;
    _pRow2->OutNUcastPkts = _pRow->dwOutNUcastPkts;
    _pRow2->OutDiscards = _pRow->dwOutDiscards;
    _pRow2->OutErrors = _pRow->dwOutErrors;
    // 伪造一下 OutUcastOctets
    _pRow2->OutUcastOctets = _pRow->dwOutOctets;
    _pRow2->OutMulticastOctets = 0;
    _pRow2->OutBroadcastOctets = 0;
    _pRow2->OutQLen = _pRow->dwOutQLen;
}

NETIOAPI_API GetIfTable2ExDownlevel(_In_ MIB_IF_TABLE_LEVEL _eLevel, _Outptr_ PMIB_IF_TABLE2* _ppTable2)
{
    // Windows XP 无法支持_eLevel
    // 相比较无法启动，一些状态报告不对就这样凑合吧。
    UNREFERENCED_PARAMETER(_eLevel);

    if (!_ppTable2)
        return ERROR_INVALID_PARAMETER;

    *_ppTable2 = NULL;

    HANDLE _hProcessHeap = ((HANDLE**)NtCurrentTeb())[12][6];

    SIZE_T _cbTable = sizeof(MIB_IFTABLE) + 256 * sizeof(MIB_IFROW);
    MIB_IFTABLE* _pTable = (MIB_IFTABLE*) HeapAlloc(_hProcessHeap, HEAP_ZERO_MEMORY, _cbTable);
    if (!_pTable)
        return ERROR_NOT_ENOUGH_MEMORY;

    LSTATUS _lStatus;
    do
    {
        DWORD _uSize = _cbTable;
        _lStatus = GetIfTable(_pTable, &_uSize, FALSE);
        if (_lStatus != NO_ERROR)
            break;

        const SIZE_T _cbTable2 = sizeof(MIB_IF_TABLE2) + _pTable->dwNumEntries * sizeof(MIB_IF_ROW2);
        MIB_IF_TABLE2* _pTable2 = (MIB_IF_TABLE2*) HeapAlloc(_hProcessHeap, 0, _cbTable2);
        if (!_pTable2)
        {
            _lStatus = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        _pTable2->NumEntries = _pTable->dwNumEntries;
        for (DWORD i = 0; i != _pTable->dwNumEntries; ++i)
        {
            MIB_IFROW_To_MIB_IF_ROW2(&_pTable->table[i], &_pTable2->Table[i]);
        }

        *_ppTable2 = _pTable2;
    } while (false);

    HeapFree(_hProcessHeap, 0, _pTable);

    return _lStatus;
}

void NETIOAPI_API_ FreeMibTableImpl(_In_ PVOID _pMemory)
{
    if (_pMemory)
    {
        HANDLE _hProcessHeap = ((HANDLE**)NtCurrentTeb())[12][6];
        HeapFree(_hProcessHeap, 0, _pMemory);
    }
}

struct name_prefix
{
    const WCHAR* szName;
    DWORD uType;
};

static const struct name_prefix name_prefixes[] =
{
    { L"other", IF_TYPE_OTHER },
    { L"ethernet", IF_TYPE_ETHERNET_CSMACD },
    { L"tokenring", IF_TYPE_ISO88025_TOKENRING },
    { L"ppp", IF_TYPE_PPP },
    { L"loopback", IF_TYPE_SOFTWARE_LOOPBACK },
    { L"atm", IF_TYPE_ATM },
    { L"wireless", IF_TYPE_IEEE80211 },
    { L"tunnel", IF_TYPE_TUNNEL },
    { L"ieee1394", IF_TYPE_IEEE1394 }
};

DWORD NETIOAPI_API_ YYConvertInterfaceIndexToLuid(_In_ NET_IFINDEX InterfaceIndex, _Out_ PNET_LUID InterfaceLuid)
{
    MIB_IFROW _IfRow;
    _IfRow.dwIndex = InterfaceIndex;
    DWORD _lStatus = GetIfEntry(&_IfRow);
    if (_lStatus != ERROR_SUCCESS)
    {
        return _lStatus;
    }

    // 使用 InterfaceIndex 假冒 LuidIndex
    InterfaceLuid->Info.IfType = _IfRow.dwType;
    InterfaceLuid->Info.NetLuidIndex = InterfaceIndex;
    InterfaceLuid->Info.Reserved = 0;
    return ERROR_SUCCESS;
}

NETIO_STATUS NETIOAPI_API_ YYConvertInterfaceLuidToIndex(_In_ CONST NET_LUID* _pInterfaceLuid, _Out_ PNET_IFINDEX _pInterfaceIndex)
{
    if (_pInterfaceLuid == NULL || _pInterfaceIndex == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *_pInterfaceIndex = (ULONG) _pInterfaceLuid->Info.NetLuidIndex;
    return ERROR_SUCCESS;
}

/******************************************************************
 *    ConvertInterfaceLuidToNameW (IPHLPAPI.@) from wine
 */
DWORD NETIOAPI_API_ WineConvertInterfaceLuidToNameW(const NET_LUID* luid, WCHAR* name, SIZE_T len)
{
    DWORD i, needed;
    const WCHAR* prefix = NULL;
    WCHAR buf[IF_MAX_STRING_SIZE + 1];

    if (!luid || !name) return ERROR_INVALID_PARAMETER;

    for (i = 0; i < ARRAY_SIZE(name_prefixes); i++)
    {
        if (luid->Info.IfType == name_prefixes[i].uType)
        {
            prefix = name_prefixes[i].szName;
            break;
        }
    }

    if (prefix) needed = swprintf(buf, len, L"%s_%lld", prefix, luid->Info.NetLuidIndex);
    else needed = swprintf(buf, len, L"iftype%lld_%lld", luid->Info.IfType, luid->Info.NetLuidIndex);

    if (needed >= len) return ERROR_NOT_ENOUGH_MEMORY;
    memcpy(name, buf, (needed + 1) * sizeof(WCHAR));
    return ERROR_SUCCESS;
}

/******************************************************************
 *    ConvertInterfaceLuidToNameA (IPHLPAPI.@) from wine
 */
DWORD NETIOAPI_API_ WineConvertInterfaceLuidToNameA(const NET_LUID* luid, char* name, SIZE_T len)
{
    DWORD err;
    WCHAR nameW[IF_MAX_STRING_SIZE + 1];

    if (!luid) return ERROR_INVALID_PARAMETER;
    if (!name || !len) return ERROR_NOT_ENOUGH_MEMORY;

    err = WineConvertInterfaceLuidToNameW(luid, nameW, ARRAY_SIZE(nameW));
    if (err) return err;

    if (!WideCharToMultiByte(CP_ACP, 0, nameW, -1, name, len, NULL, NULL))
        err = GetLastError();
    return err;
}

/******************************************************************
 *    ConvertInterfaceNameToLuidW (IPHLPAPI.@) from wine
 */
DWORD NETIOAPI_API_ WineConvertInterfaceNameToLuidW(const WCHAR* name, NET_LUID* luid)
{
    const WCHAR* sep;
    DWORD type = ~0u, i;
    int iftype_len = wcslen(L"iftype");
    WCHAR buf[IF_MAX_STRING_SIZE + 1];

    if (!luid) return ERROR_INVALID_PARAMETER;
    memset(luid, 0, sizeof(*luid));

    if (!name || !(sep = wcschr(name, '_')) || sep >= name + ARRAY_SIZE(buf)) return ERROR_INVALID_NAME;
    memcpy(buf, name, (sep - name) * sizeof(WCHAR));
    buf[sep - name] = '\0';

    if (sep - name > iftype_len && !memcmp(buf, L"iftype", iftype_len * sizeof(WCHAR)))
    {
        type = wcstol(buf + iftype_len, NULL, 10);
    }
    else
    {
        for (i = 0; i < ARRAY_SIZE(name_prefixes); i++)
        {
            if (!wcscmp(buf, name_prefixes[i].szName))
            {
                type = name_prefixes[i].uType;
                break;
            }
        }
    }
    if (type == ~0u) return ERROR_INVALID_NAME;

    luid->Info.NetLuidIndex = wcstol(sep + 1, NULL, 10);
    luid->Info.IfType = type;
    return ERROR_SUCCESS;
}

/******************************************************************
 *    ConvertInterfaceNameToLuidA (IPHLPAPI.@) from wine
 */
DWORD NETIOAPI_API_ WineConvertInterfaceNameToLuidA(const char* name, NET_LUID* luid)
{
    WCHAR nameW[IF_MAX_STRING_SIZE];

    if (!name) return ERROR_INVALID_NAME;
    if (!MultiByteToWideChar(CP_ACP, 0, name, -1, nameW, ARRAY_SIZE(nameW)))
        return GetLastError();

    return WineConvertInterfaceNameToLuidW(nameW, luid);
}

/******************************************************************
 *    if_nametoindex (IPHLPAPI.@) from wine
 */
IF_INDEX NETIOAPI_API_ IPHLP_if_nametoindex(const char* name)
{
    IF_INDEX index;
    NET_LUID luid;
    DWORD err;

    err = WineConvertInterfaceNameToLuidA(name, &luid);
    if (err) return 0;

    err = YYConvertInterfaceLuidToIndex(&luid, &index);
    if (err) index = 0;
    return index;
}

/******************************************************************
 *    if_indextoname (IPHLPAPI.@) from wine
 */
char* NETIOAPI_API_ IPHLP_if_indextoname(NET_IFINDEX index, char* name)
{
    NET_LUID luid;
    DWORD err;

    err = YYConvertInterfaceIndexToLuid(index, &luid);
    if (err) return NULL;

    err = WineConvertInterfaceLuidToNameA(&luid, name, IF_MAX_STRING_SIZE);
    if (err) return NULL;
    return name;
}
