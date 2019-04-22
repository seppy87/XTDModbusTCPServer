#pragma once

#ifdef DEF_WINDOWS
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
	#ifdef DEF_DLL
		#ifdef DEF_IMPORT
			#define DLL __declspec(dllimport)
		#else
			#define DLL __declspec(dllexport)
		#endif
	#else
		#define DLL /**/
	#endif
#else
	#define DLL /**/
	using UINT16 = unsigned short;
#endif

#include<exception>