#include <windows.h>
#include "ring0.h"

#ifdef WIN95
//DWORD g_result =0, g_eax =0;
// 修改地址属性
__declspec(naked) void Ring0ModifyPageProtection()
{
	_asm
	{
	//Mov [g_result], 1 
		Mov EAX, ECX
		Shr EAX, 22
		Test DWORD PTR [0FFBFE000h + EAX * 4], 1
		Jz Fail

 	//Mov [g_result], 2 
		Mov EAX, ECX
		Shr EAX, 12
		Mov EBX, EAX
		Mov EAX, DWORD PTR [0FF800000h + EAX * 4]
		Test EAX, 1
		//Jz Fail	// changed by Paladin 2003.01.23

	//Mov [g_result], 3 //

		Mov EAX, 1
		Cmp EDX, PAGE_READWRITE
		Je PageReadWrite

		And DWORD PTR [0FF800000h + EBX * 4], 0xFFFFFFFD
		Jmp Done

PageReadWrite:

		Or DWORD PTR [0FF800000h + EBX * 4], 2
		Jmp Done

Fail:

		Xor EAX, EAX

Done:

		Retf
	}
}

// 使系统进入Ring0模式下
GDTR gdtr;
GDT_DESCRIPTOR *pGDTDescriptor;
bool CallRing0(PVOID pvRing0FuncAddr, PVOID pvAddr, DWORD dwPageProtection)
{

	bool Result;

	if(pvAddr ==NULL) return false;

	_asm Sgdt [gdtr]

	pGDTDescriptor = (GDT_DESCRIPTOR *)(gdtr.dwGDTBase + 8);

	

	for (WORD wGDTIndex = 1; wGDTIndex < (gdtr.wGDTLimit / 8); wGDTIndex++)
	{
		if (pGDTDescriptor->Type == 0   &&
			pGDTDescriptor->System == 0 &&
			pGDTDescriptor->DPL == 0    &&
			pGDTDescriptor->Present == 0)
		{

		CALLGATE_DESCRIPTOR *pCallgate;

		pCallgate =	(CALLGATE_DESCRIPTOR *) pGDTDescriptor;
		pCallgate->Offset_0_15 = LOWORD(pvRing0FuncAddr);
		pCallgate->Selector = 0x28;
		pCallgate->ParamCount =	0;
		pCallgate->Unused = 0;
		pCallgate->Type = 0xc;  
		pCallgate->System = 0;  
		pCallgate->DPL = 3;    
		pCallgate->Present = 1;
		pCallgate->Offset_16_31 = HIWORD(pvRing0FuncAddr);

		

		WORD CallgateAddr[3];

		CallgateAddr[0] = 0x0;
		CallgateAddr[1] = 0x0;
		CallgateAddr[2] = (wGDTIndex << 3) | 3;

		

		_asm
		{
			Mov ECX, [pvAddr]
			Mov EDX, [dwPageProtection]
			Cli
			Call FWORD PTR [CallgateAddr]
			Sti
			Mov DWORD PTR [Result], EAX
		}

		

		memset(pGDTDescriptor, 0, 8);

		return Result;
	}

	
	pGDTDescriptor++;
	}

	

	return false;
}

// 改变内存状态为可读写
BOOL RemovePageProtection(PVOID pvAddr)
{
	return CallRing0((PVOID)Ring0ModifyPageProtection, pvAddr, PAGE_READWRITE);
}


BOOL SetPageProtection(PVOID pvAddr)
{
	return CallRing0((PVOID)Ring0ModifyPageProtection, pvAddr, PAGE_READONLY);
}

#endif
