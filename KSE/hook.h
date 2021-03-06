#pragma once

#include <Windows.h>
#include "KSE_thread.h"
#include "function_address.h"

DWORD dummyProtection;

LPVOID funcPointer;
LPVOID *funcPointerP = &funcPointer;

/*
The hooking method KSE implements is based of the terrible idea of offloading all the registers to another thread to avoid the compiled functions from destroying the stack and registers 
and allowing the developer to still write all the high level code they want, runs fine and reasonably fast but feels disgusting.

The hook clones all the registers to variables managed by KSE and passes the pointer of a high level replacement function to KSEThread to be run, 
the replacement function then can modify those cloned registers and once it is finished it can return back to the hook which then moves all those 
cloned register variables back into the the registers and returns to normal operations.

Terrible and should probably be replaced.
*/

void CreateHook(Hook* hook)
{
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	LPVOID hookLoc = (LPVOID)hook->address;

	LPVOID hookFunc = VirtualAlloc(nullptr, systemInfo.dwPageSize, MEM_COMMIT, PAGE_READWRITE);
	char hookFuncCode[] = {
		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, hook.function
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [activeFunctionP], rax

		0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,					//mov rax, 1
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [isFunctionActiveP], rax
																	//loop:
		0x48, 0x83, 0xf8, 0x01,										//cmp rax, 1
		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [isFunctionActiveP]
		0x74, 0xf0,													//je loop
		0xc3														//ret
	};

	memcpy(&hookFuncCode[2], &hook->function, sizeof(DWORD64));
	memcpy(&hookFuncCode[12], &activeFunctionP, sizeof(DWORD64));

	memcpy(&hookFuncCode[29], &isFunctionActiveP, sizeof(DWORD64));
	memcpy(&hookFuncCode[43], &isFunctionActiveP, sizeof(DWORD64));
	
	memcpy(hookFunc, &hookFuncCode, 54);
	VirtualProtect(hookFunc, 54, PAGE_EXECUTE_READ, &dummyProtection);


	LPVOID registerFunc = VirtualAlloc(nullptr, systemInfo.dwPageSize, MEM_COMMIT, PAGE_READWRITE);
	char hookCode[] = {
		0x50,														//push rax
		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, registerFunc
		0xff, 0xe0,													//jmp rax
		0x58														//pop rax
	};
	memcpy(&hookCode[3], &registerFunc, sizeof(DWORD64));

	VirtualProtect(hookLoc, hook->length, PAGE_READWRITE, &dummyProtection);
	memset(hookLoc, 0x90, hook->length);
	memcpy(hookLoc, &hookCode, 14);
	VirtualProtect(hookLoc, hook->length, PAGE_EXECUTE_READ, &dummyProtection);

	char registerCode[] = {

		0x58,														//pop rax

		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov [0x1122334455667788], rax

		0x48, 0x89, 0xd8,											//mov rax, rbx
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x48, 0x89, 0xc8,											//mov rax, rcx
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x48, 0x89, 0xd0,											//mov rax, rdx
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x48, 0x89, 0xf0,											//mov rax, rsi
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x48, 0x89, 0xf8,											//mov rax, rdi
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x48, 0x89, 0xe8,											//mov rax, rbp
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x48, 0x89, 0xe0,											//mov rax, rsp
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x4c, 0x89, 0xc0,											//mov rax, r8
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x4c, 0x89, 0xc8,											//mov rax, r9
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x4c, 0x89, 0xd0,											//mov rax, r10
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x4c, 0x89, 0xd8,											//mov rax, r11
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x4c, 0x89, 0xe0,											//mov rax, r12
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x4c, 0x89, 0xe8,											//mov rax, r13
		0x48, 0xa3, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov [0x1122334455667788], rax

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x7f, 0x00, 									//movdqu [rax], xmm0

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x7f, 0x08,			  						 	//movdqu [rax], xmm1

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x7f, 0x10,									 	//movdqu [rax], xmm2

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x7f, 0x18, 									//movdqu [rax], xmm3

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x7f, 0x20, 									//movdqu [rax], xmm4

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x7f, 0x28, 									//movdqu [rax], xmm5

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x7f, 0x30, 									//movdqu [rax], xmm6

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x7f, 0x38, 									//movdqu [rax], xmm7

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x7f, 0x00, 								//movdqu [rax], xmm8

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x7f, 0x08, 								//movdqu [rax], xmm9

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x7f, 0x10, 								//movdqu [rax], xmm10

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x7f, 0x18, 								//movdqu [rax], xmm11

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x7f, 0x20, 								//movdqu [rax], xmm12

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x7f, 0x28, 								//movdqu [rax], xmm13

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x7f, 0x30, 								//movdqu [rax], xmm14

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x7f, 0x38,			 					//movdqu [rax], xmm15

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, 0x1122334455667788
		0xff, 0xd0,													//call rax

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x6f, 0x38, 								//movdqu xmm15, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x6f, 0x30, 								//movdqu xmm14, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x6f, 0x28, 								//movdqu xmm13, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x6f, 0x20, 								//movdqu xmm12, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x6f, 0x18, 								//movdqu xmm11, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x6f, 0x10, 								//movdqu xmm10, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x6f, 0x08, 								//movdqu xmm9, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x44, 0x0f, 0x6f, 0x00, 								//movdqu xmm8, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x6f, 0x38, 									//movdqu xmm7, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x6f, 0x30, 									//movdqu xmm6, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x6f, 0x28, 									//movdqu xmm5, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x6f, 0x20, 									//movdqu xmm4, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x6f, 0x18, 									//movdqu xmm3, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x6f, 0x10, 									//movdqu xmm2, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x6f, 0x08, 									//movdqu xmm1, [rax]

		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, 0x1122334455667788
		0xf3, 0x0f, 0x6f, 0x00, 									//movdqu xmm0, [rax]

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x49, 0x89, 0xc5,											//mov r13, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x49, 0x89, 0xc4,											//mov r12, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x49, 0x89, 0xc3,											//mov r11, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x49, 0x89, 0xc2,											//mov r10, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x49, 0x89, 0xc1,											//mov r9, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x49, 0x89, 0xc0,											//mov r8, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x48, 0x89, 0xc4,											//mov rsp, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x48, 0x89, 0xc5,											//mov rbp, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x48, 0x89, 0xc7,											//mov rdi, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x48, 0x89, 0xc6,											//mov rsi, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x48, 0x89, 0xc2,											//mov rdx, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x48, 0x89, 0xc1,											//mov rcx, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]
		0x48, 0x89, 0xc3,											//mov rbx, rax

		0x48, 0xa1, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, [0x1122334455667788]

		0x50,														//push rax
		0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,	//mov rax, 0x1122334455667788
		0xff, 0xe0													//jmp rax
	};
	memcpy(&registerCode[3], &RAXP, sizeof(DWORD64));
	memcpy(&registerCode[16], &RBXP, sizeof(DWORD64));
	memcpy(&registerCode[29], &RCXP, sizeof(DWORD64));
	memcpy(&registerCode[42], &RDXP, sizeof(DWORD64));
	memcpy(&registerCode[55], &RSIP, sizeof(DWORD64));
	memcpy(&registerCode[68], &RDIP, sizeof(DWORD64));
	memcpy(&registerCode[81], &RBPP, sizeof(DWORD64));
	memcpy(&registerCode[94], &RSPP, sizeof(DWORD64));
	memcpy(&registerCode[107], &R8P, sizeof(DWORD64));
	memcpy(&registerCode[120], &R9P, sizeof(DWORD64));
	memcpy(&registerCode[133], &R10P, sizeof(DWORD64));
	memcpy(&registerCode[146], &R11P, sizeof(DWORD64));
	memcpy(&registerCode[159], &R12P, sizeof(DWORD64));
	memcpy(&registerCode[172], &R13P, sizeof(DWORD64));
	memcpy(&registerCode[182], &XMM0P, sizeof(DWORD64));
	memcpy(&registerCode[196], &XMM1P, sizeof(DWORD64));
	memcpy(&registerCode[210], &XMM2P, sizeof(DWORD64));
	memcpy(&registerCode[224], &XMM3P, sizeof(DWORD64));
	memcpy(&registerCode[238], &XMM4P, sizeof(DWORD64));
	memcpy(&registerCode[252], &XMM5P, sizeof(DWORD64));
	memcpy(&registerCode[266], &XMM6P, sizeof(DWORD64));
	memcpy(&registerCode[280], &XMM7P, sizeof(DWORD64));
	memcpy(&registerCode[294], &XMM8P, sizeof(DWORD64));
	memcpy(&registerCode[309], &XMM9P, sizeof(DWORD64));
	memcpy(&registerCode[324], &XMM10P, sizeof(DWORD64));
	memcpy(&registerCode[339], &XMM11P, sizeof(DWORD64));
	memcpy(&registerCode[354], &XMM12P, sizeof(DWORD64));
	memcpy(&registerCode[369], &XMM13P, sizeof(DWORD64));
	memcpy(&registerCode[384], &XMM14P, sizeof(DWORD64));
	memcpy(&registerCode[399], &XMM15P, sizeof(DWORD64));

	//Copies pointer to hook function into code.
	memcpy(&registerCode[414], &hookFunc, sizeof(DWORD64));


	memcpy(&registerCode[426], &XMM15P, sizeof(DWORD64));
	memcpy(&registerCode[441], &XMM14P, sizeof(DWORD64));
	memcpy(&registerCode[456], &XMM13P, sizeof(DWORD64));
	memcpy(&registerCode[471], &XMM12P, sizeof(DWORD64));
	memcpy(&registerCode[486], &XMM11P, sizeof(DWORD64));
	memcpy(&registerCode[501], &XMM10P, sizeof(DWORD64));
	memcpy(&registerCode[516], &XMM9P, sizeof(DWORD64));
	memcpy(&registerCode[531], &XMM8P, sizeof(DWORD64));
	memcpy(&registerCode[546], &XMM7P, sizeof(DWORD64));
	memcpy(&registerCode[560], &XMM6P, sizeof(DWORD64));
	memcpy(&registerCode[574], &XMM5P, sizeof(DWORD64));
	memcpy(&registerCode[588], &XMM4P, sizeof(DWORD64));
	memcpy(&registerCode[602], &XMM3P, sizeof(DWORD64));
	memcpy(&registerCode[616], &XMM2P, sizeof(DWORD64));
	memcpy(&registerCode[630], &XMM1P, sizeof(DWORD64));
	memcpy(&registerCode[644], &XMM0P, sizeof(DWORD64));
	memcpy(&registerCode[658], &R13P, sizeof(DWORD64));
	memcpy(&registerCode[671], &R12P, sizeof(DWORD64));
	memcpy(&registerCode[684], &R11P, sizeof(DWORD64));
	memcpy(&registerCode[697], &R10P, sizeof(DWORD64));
	memcpy(&registerCode[710], &R9P, sizeof(DWORD64));
	memcpy(&registerCode[723], &R8P, sizeof(DWORD64));
	memcpy(&registerCode[736], &RSPP, sizeof(DWORD64));
	memcpy(&registerCode[749], &RBPP, sizeof(DWORD64));
	memcpy(&registerCode[762], &RDIP, sizeof(DWORD64));
	memcpy(&registerCode[775], &RSIP, sizeof(DWORD64));
	memcpy(&registerCode[788], &RDXP, sizeof(DWORD64));
	memcpy(&registerCode[801], &RCXP, sizeof(DWORD64));
	memcpy(&registerCode[814], &RBXP, sizeof(DWORD64));
	memcpy(&registerCode[827], &RAXP, sizeof(DWORD64));



	LPVOID endJump = (LPVOID)((DWORD64)hookLoc + 13);
	memcpy(&registerCode[838], &endJump, sizeof(DWORD64));

	memcpy(registerFunc, &registerCode, 848);
	VirtualProtect(registerFunc, systemInfo.dwPageSize, PAGE_EXECUTE_READ, &dummyProtection);
}