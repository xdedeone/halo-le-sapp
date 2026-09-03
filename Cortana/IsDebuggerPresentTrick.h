#pragma once

class AntiAntiIsDP
{
public:
	AntiAntiIsDP()
	{
		Activate();
	}
	bool Check()
	{
		__asm
		{
			mov al, 2
			mov ecx, dword ptr fs : [0x18]
			mov ecx, dword ptr ds : [ecx + 0x30]
			cmp byte ptr ds : [ecx + 2], 2
			sete al
		}
	}
	void Activate()
	{
		__asm
		{	mov ecx, dword ptr fs : [0x18]
			mov ecx, dword ptr ds : [ecx + 0x30]
			or byte ptr ds : [ecx + 2], 2
		}
	}
	void Deactivate()
	{
		__asm
		{	mov ecx, dword ptr fs : [0x18]
			mov ecx, dword ptr ds : [ecx + 0x30]
			mov byte ptr ds : [ecx + 2], 2
		}
	}
};