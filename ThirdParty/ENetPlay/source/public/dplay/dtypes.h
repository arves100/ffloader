#ifndef _DIRECTPLAY_TYPES_H_
#define _DIRECTPLAY_TYPES_H_

#ifndef DP_NO_BASE_TYPES

#define VOID void
typedef void* LPVOID;

typedef int INT;
typedef unsigned int UINT;

typedef long LONG;
typedef unsigned long ULONG;

typedef char CHAR;
typedef unsigned char UCHAR;

typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;

typedef short SHORT;
typedef unsigned short USHORT;

typedef char* LPSTR;
typedef const LPSTR LPCSTR;

typedef wchar_t* LPWSTR;
typedef const LPWSTR LPCWSTR;

typedef UCHAR BYTE;
typedef BYTE* LPBYTE;

typedef SHORT WORD;
typedef SHORT* LPWORD;

typedef LONG DWORD;
typedef DWORD* LPDWORD;

typedef LONGLONG QWORD;
typedef QWORD* LPQWORD;

typedef DWORD HRESULT;

#define THISCALL __thiscall
#define STDCALL __stdcall
#define CDECL __cdecl
#define PURE = 0

#endif // !DP_NO_BASE_TYPES

#ifndef DP_NO_OLE_TYPES

#define STDMETHOD(x) HRESULT x
#define STDMETHOD_(r, x) r x
#define DECLARE_INTERFACE_(t, b) struct t : b


#endif // !DP_NO_OLE_TYPES


#endif // _DIRECTPLAY_TYPES_H_
