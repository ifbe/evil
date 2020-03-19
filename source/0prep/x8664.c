#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
static char* reg64[8] = {"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi"};
static char* reg32[8] = {"eax","ecx","edx","ebx","esp","ebp","esi","edi"};
static char* reg16[8] = {"ax","cx","dx","bx","ax","cx","dx","bx"};
static char* reg08[8] = {"al","cl","dl","bl","ah","ch","dh","bh"};




void disasm_x8664_one(u8* buf, int len)
{
	int j;
	u8* p;
	for(j=0;j<len;j++)
	{
		printf("%x:	", j);
		p = buf+j;

		if((0x01 == p[0])&&(0xc0 == (p[1]&0xc0)))
		{
			printf("%s += %s\n", reg32[p[1]&0x7], reg32[(p[1]&0x3f)>>3]);
			j += 1;
		}
		else if((0x08 == p[0])&&(0xc0 == (p[1]&0xc0)))
		{
			printf("%s |= %s\n", reg08[p[1]&0x7], reg08[(p[1]&0x3f)>>3]);
			j += 1;
		}
		else if((0x09 == p[0])&&(0xc0 == (p[1]&0xc0)))
		{
			printf("%s |= %s\n", reg32[p[1]&0x7], reg32[(p[1]&0x3f)>>3]);
			j += 1;
		}
		else if(0xc == p[0])
		{
			printf("al |= 0x%x\n", p[1]);
			j += 1;
		}
		else if(0xd == p[0])
		{
			printf("eax |= 0x%x\n", *(u32*)(p+1));
			j += 4;
		}
		else if(
			((0x0f == p[0])&&(0x1f == p[1])) |
			((0x66 == p[0])&&(0x0f == p[1])&&(0x1f == p[2])))
		{
			if(0x66 == p[0])
			{
				p++;
				j++;
			}

			if(0 == p[2])
			{
				printf("nop3\n");
				j += 2;
			}
			else if(0x84 == p[2])
			{
				printf("nop8\n");
				j += 7;
			}
			else if(0x80 == p[2])
			{
				printf("nop7\n");
				j += 6;
			}
			else if(0x44 == p[2])
			{
				printf("nop5\n");
				j += 4;
			}
			else
			{
				printf("nop4\n");
				j += 3;
			}
		}
		else if((0x0f == p[0])&&(0x29 == p[1])&&(0x44 == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp+0x%x] = xmm0\n", p[4]);
			j += 4;
		}
		else if((0x0f == p[0])&&(0x29 == p[1])&&(0x4c == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp+0x%x] = xmm1\n", p[4]);
			j += 4;
		}
		else if((0x0f == p[0])&&(0x29 == p[1])&&(0x54 == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp+0x%x] = xmm2\n", p[4]);
			j += 4;
		}
		else if((0x0f == p[0])&&(0x29 == p[1])&&(0x9c == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp+0x%x] = xmm3\n", p[4]);
			j += 7;
		}
		else if((0x0f == p[0])&&(0x29 == p[1])&&(0xa4 == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp+0x%x] = xmm4\n", p[4]);
			j += 7;
		}
		else if((0x0f == p[0])&&(0x29 == p[1])&&(0xac == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp+0x%x] = xmm5\n", p[4]);
			j += 7;
		}
		else if((0x0f == p[0])&&(0x29 == p[1])&&(0xb4 == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp+0x%x] = xmm6\n", p[4]);
			j += 7;
		}
		else if((0x0f == p[0])&&(0x29 == p[1])&&(0xbc == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp+0x%x] = xmm7\n", p[4]);
			j += 7;
		}
		else if((0x0f == p[0])&&(0x47 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("if(a)%s = %s\n", reg32[(p[2]&0x3f)>>3], reg32[p[2]&0x7]);
			j += 2;
		}
		else if((0x0f == p[0])&&(0x80 == (p[1]&0xf0)))
		{
			if(0x80 == p[1])printf("if(o)");
			else if(0x81 == p[1])printf("if(no)");
			else if(0x82 == p[1])printf("if(c)");
			else if(0x83 == p[1])printf("if(nc)");
			else if(0x84 == p[1])printf("if(z)");
			else if(0x85 == p[1])printf("if(nz)");
			else if(0x86 == p[1])printf("if(na)");
			else if(0x87 == p[1])printf("if(a)");
			else if(0x88 == p[1])printf("if(s)");
			else if(0x89 == p[1])printf("if(ns)");
			else if(0x8a == p[1])printf("if(pe)");
			else if(0x8b == p[1])printf("if(po)");
			else if(0x8c == p[1])printf("if(l)");
			else if(0x8d == p[1])printf("if(nl)");
			else if(0x8e == p[1])printf("if(ng)");
			else if(0x8f == p[1])printf("if(g)");
			printf("jmp 0x%x\n", *(int*)(p+2) + (j+6));
			j += 5;
		}
		else if((0x0f == p[0])&&(0x94 == p[1])&&(0xc1 == p[2]))
		{
			printf("setz cl\n");
			j += 2;
		}
		else if((0x0f == p[0])&&(0xb6 == p[1])&&(0x02 == p[2]))
		{
			printf("eax = [rdx, 1]\n");
			j += 2;
		}
		else if((0x0f == p[0])&&(0xb6 == p[1])&&(0x3b == p[2]))
		{
			printf("edi = [rbx,1]\n");
			j += 2;
		}
		else if((0x0f == p[0])&&(0xb6 == p[1])&&(0x75 == p[2]))
		{
			printf("esi = [rbp + 0x%x,1]\n", p[3]);
			j += 3;
		}
		else if((0x0f == p[0])&&(0xb7 == p[1])&&(0x6a == p[2]))
		{
			printf("ebp = [rdx+0x%x, 2]\n", p[3]);
			j += 3;
		}
		else if((0x0f == p[0])&&(0xb7 == p[1])&&(0xc0 == p[2]))
		{
			printf("eax = ax\n");
			j += 2;
		}
		else if((0x0f == p[0])&&(0xbe == p[1])&&(0x0a == p[2]))
		{
			printf("ecx = (s32)[rdx,1]\n");
			j += 2;
		}
		else if((0x0f == p[0])&&(0xbe == p[1])&&(0xc8 == p[2]))
		{
			printf("ecx = (s32)al\n");
			j += 2;
		}
		else if(0x25 == p[0])
		{
			printf("eax &= 0x%x\n", *(u32*)(p+1));
			j += 4;
		}
		else if((0x29 == p[0])&&(0xc0 == (p[1]&0xc0)))
		{
			printf("%s -= %s\n", reg32[p[1]&0x7], reg32[(p[1]&0x3f)>>3]);
			j += 1;
		}
		else if((0x30 == p[0])&&(0xc0 == (p[1]&0xc0)))
		{
			printf("%s ^= %s\n", reg08[p[1]&0x7], reg08[(p[1]&0x3f)>>3]);
			j += 1;
		}
		else if((0x31 == p[0])&&(0xc0 == (p[1]&0xc0)))
		{
			printf("%s ^= %s\n", reg32[p[1]&0x7], reg32[(p[1]&0x3f)>>3]);
			j += 1;
		}
		else if((0x38 == p[0])&&(0xc0 == (p[1]&0xc0)))
		{
			printf("%s ? %s\n", reg08[p[1]&0x7], reg08[(p[1]&0x3f)>>3]);
			j += 1;
		}
		else if((0x39 == p[0])&&(0xeb == p[1]))
		{
			printf("ebx ? ebp\n");
			j += 1;
		}
		else if((0x39 == p[0])&&(0xd0 == (p[1]&0xf8)))
		{
			printf("%s ? edx\n", reg32[p[1]&0x7]);
			j += 1;
		}
		else if(0x3c == p[0])
		{
			printf("al ? 0x%x\n", p[1]);
			j += 1;
		}
		else if(0x3d == p[0])
		{
			printf("eax ? 0x%x\n", *(u32*)(p+1));
			j += 4;
		}
		else if((0x40 == p[0])&&(0x0f == p[1])&&(0x94 == p[2])&&(0xc6 == p[3]))
		{
			printf("setz sil\n");
			j += 3;
		}
		else if((0x40 == p[0])&&(0x0f == p[1])&&(0xb6 == p[2])&&(0xf5 == p[3]))
		{
			printf("esi = bpl\n");
			j += 3;
		}
		else if((0x40 == p[0])&&(0x80 == p[1])&&(0xfe == p[2]))
		{
			printf("sil ? 0x%x\n", p[3]);
			j += 3;
		}
		else if((0x40 == p[0])&&(0x80 == p[1])&&(0xff == p[2]))
		{
			printf("dil ? 0x%x\n", p[3]);
			j += 3;
		}
		else if((0x40 == p[0])&&(0x84 == p[1])&&(0xce == p[2]))
		{
			printf("flag = sil & cl\n");
			j += 2;
		}
		else if((0x40 == p[0])&&(0xf6 == p[1])&&(0xc6 == p[2]))
		{
			printf("sil ? 0x%x\n", p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0x0f == p[1])&&(0x4d == p[2])&&(0xc0 == (p[3]&0xc0)))
		{
			printf("if(nl)%s = r%dd\n", reg32[(p[3]&0x3f)>>3], 8+(p[3]&7));
			j += 3;
		}
		else if((0x41 == p[0])&&(0x0f == p[1])&&(0xb6 == p[2])&&(0x0 == (p[3]&0xc0)))
		{
			if(4 == (p[3]&0x7))
			{
				if(0x24 == p[4])
				{
					printf("%s = [r%d,1]\n", reg32[p[3]>>3], 8+(p[3]&0x7));
				}
				else
				{
					printf("%s = [r%d + %s]\n",
						reg32[p[3]>>3],
						8+(p[4]&0x7),
						reg64[(p[4]&0x38)>>3]
					);
				}
				j += 4;
			}
			else
			{
				printf("%s = [r%d,1]\n", reg32[p[3]>>3], 8+(p[3]&0x7));
				j += 3;
			}
		}//410FB6341F        movzx esi,byte [r15+rbx]
		else if((0x41 == p[0])&&(0x0f == p[1])&&(0xb6 == p[2])&&(0x45 == p[3]))
		{
			printf("eax = [r13+0x%x,1]\n", p[4]);
			j += 4;
		}
		else if((0x41 == p[0])&&(0x0f == p[1])&&(0xbe == p[2])&&(0x7a == p[3]))
		{
			if(p[4] < 0x80)printf("edi = (s32)[r10 + 0x%x, 1]\n", p[4]);
			else printf("edi = (s32)[r10 - 0x%x, 1]\n", 0x100-p[4]);
			j += 4;
		}
		else if((0x41 == p[0])&&(0x0f == p[1])&&(0xbe == p[2])&&(0x0c == p[3])&&(0x06 == p[4]))
		{
			printf("ecx = (s32)[r14+rax,1]\n");
			j += 4;
		}
		else if((0x41 == p[0])&&(0x0f == p[1])&&(0xbe == p[2])&&(0x3c == p[3])&&(0x36 == p[4]))
		{
			printf("edi = (s32)[r14+rsi,1]\n");
			j += 4;
		}
		else if((0x41 == p[0])&&(0x29 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("r%dd -= %s\n", 8+(p[2]&7), reg32[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x41 == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("r%dd ? %s\n", 8+(p[2]&7), reg32[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x41 == p[0])&&(0x50 == (p[1]&0xf8)))
		{
			printf("push r%d\n", 8+(p[1]&0x7));
			j += 1;
		}
		else if((0x41 == p[0])&&(0x58 == (p[1]&0xf8)))
		{
			printf("pop r%d\n", 8+(p[1]&0x7));
			j += 1;
		}
		else if((0x41 == p[0])&&(0x80 == p[1])&&(0x3c == p[2])&&(0x04 == p[3]))
		{
			printf("[r12+rax,1] ? 0x%x\n", p[4]);
			j += 4;
		}
		else if((0x41 == p[0])&&(0x80 == p[1])&&(0x49 == p[2]))
		{
			printf("[r9+0x%x] |= 0x%x\n", p[3], p[4]);
			j += 4;
		}
		else if((0x41 == p[0])&&(0x83 == p[1])&&(0xc0 == (p[2]&0xf8)))
		{
			printf("r%dd += 0x%x\n", 8+(p[2]&0x7), p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0x83 == p[1])&&(0xe6 == p[2]))
		{
			printf("r14d &= 0x%x\n", p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0x83 == p[1])&&(0xe8 == p[2]))
		{
			printf("r8d -= 0x%x\n", p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0x83 == p[1])&&(0xf9 == p[2]))
		{
			printf("r9d ? 0x%x\n", p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0x83 == p[1])&&(0xfd == p[2]))
		{
			printf("r13d ? 0x%x\n", p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0x85 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("flag = r%dd & %s\n", 8+(p[2]&0x7), reg32[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x41 == p[0])&&(0x88 == p[1])&&(0x0 == (p[2]&0xc0)))
		{
			printf("[r%d] = %s\n", 8+(p[2]&0x7), reg08[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x41 == p[0])&&(0x88 == p[1])&&(0x41 == p[2]))
		{
			printf("[r9 + 0x%x] = al\n", p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0x88 == p[1])&&(0x49 == p[2]))
		{
			printf("[r9 + 0x%x] = cl\n", p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0x88 == p[1])&&(0x71 == p[2]))
		{
			printf("[r9 + 0x%x] = sil\n", p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0x89 == p[1])&&(0x0 == (p[2]&0xc0)))
		{
			printf("[%d] = %s\n", 8+(p[2]&0x7), reg32[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x41 == p[0])&&(0x89 == p[1])&&(0x50 == p[2]))
		{
			printf("[r8+0x%x],edx\n", p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0x89 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("r%dd = %s\n", 8+(p[2]&0x7), reg32[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x41 == p[0])&&(0x8b == p[1])&&(0x10 == p[2]))
		{
			printf("edx = [r8]\n");
			j += 2;
		}
		else if((0x41 == p[0])&&(0x8b == p[1])&&(0x40 == (p[2]&0xc0)))
		{
			printf("%s = [r%d + 0x%x]\n", reg32[(p[2]&0x3f)>>3], 8+(p[2]&0x7), p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0x8d == p[1])&&(0x45 == p[2]))
		{
			printf("eax = r13 + 0x%x\n", p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0x8d == p[1])&&(0x6c == p[2])&&(0x35 == p[3]))
		{
			printf("ebp = r13 + rsi + 0x%x\n", p[4]);
			j += 4;
		}
		else if((0x41 == p[0])&&(0xc1 == p[1])&&(0xe0 == (p[2]&0xf8)))
		{
			printf("r%dd <<= 0x%x\n", 8+(p[2]&0x7), p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0xc1 == p[1])&&(0xe8 == (p[2]&0xf8)))
		{
			printf("r%dd >>= 0x%x\n", p[2]&0x7, p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0xc1 == p[1])&&(0xfd == p[2]))
		{
			printf("sar r13d, 0x%x\n", p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0xc6 == p[1])&&(0x01 == p[2]))
		{
			printf("[r9, 1] = 0x%x\n", p[3]);
			j += 3;
		}
		else if((0x41 == p[0])&&(0xc6 == p[1])&&(0x04 == p[2])&&(0x0c == p[3]))
		{
			printf("[r12+rcx,1] = 0x%x\n", p[4]);
			j += 4;
		}
		else if((0x41 == p[0])&&(0xc6 == p[1])&&(0x41 == p[2]))
		{
			printf("[r9 + 0x%x, 1] = 0x%x\n", p[3], p[4]);
			j += 4;
		}
		else if((0x41 == p[0])&&(0xc7 == p[1])&&(0x40 == (p[2]&0xc0)))
		{
			printf("[r%d + 0x%x] = 0x%x\n", 8+(p[2]&0x7), p[3], *(u32*)(p+4));
			j += 7;
		}
		else if((0x41 == p[0])&&(0xf6 == p[1])&&(0xc4 == p[2]))
		{
			printf("r12b ? 0x%x\n", p[3]);
			j += 3;
		}
		else if((0x43 == p[0])&&(0x8d == p[1])&&(0x74 == p[2])&&(0xad == p[3]))
		{
			printf("esi = r13 + r13*4 + 0x%x\n", p[4]);
			j += 4;
		}
		else if((0x44 == p[0])&&(0x01 == p[1])&&(0xea == p[2]))
		{
			printf("edx += r13d\n");
			j += 2;
		}
		else if((0x44 == p[0])&&(0x29 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s -= r%dd\n", reg32[p[2]&0x7], 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x44 == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s ? r%dd\n", reg32[p[2]&0x7], 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x44 == p[0])&&(0x85 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("flag = %s & r%dd\n", reg32[(p[2]&0x7)], 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x44 == p[0])&&(0x89 == p[1])&&(0x4c == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp + 0x%x] = r9d\n", p[4]);
			j += 4;
		}
		else if((0x44 == p[0])&&(0x89 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s = r%dd\n", reg32[p[2]&0x7], 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x44 == p[0])&&(0x8b == p[1])&&(0x05 == p[2]))
		{
			printf("r8d = [rip + 0x%x]\n", *(u32*)(p+3));
			j += 6;
		}
		else if((0x44 == p[0])&&(0x8b == p[1])&&(0x3b == p[2]))
		{
			printf("r15d = [rbx]\n");
			j += 2;
		}
		else if((0x44 == p[0])&&(0x8b == p[1])&&(0x4a == p[2]))
		{
			printf("r9d = [rdx+0x%x]\n", p[3]);
			j += 3;
		}
		else if((0x44 == p[0])&&(0x8b == p[1])&&(0x4c == p[2])&&(0x24 == p[3]))
		{
			printf("r9d = [rsp + 0x%x]\n", p[4]);
			j += 4;
		}
		else if((0x44 == p[0])&&(0x8d == p[1])&&(0x34 == p[2])&&(0x03 == p[3]))
		{
			printf("r14d = rbx + rax\n");
			j += 3;
		}
		else if((0x44 == p[0])&&(0x8d == p[1])&&(0x6c == p[2])&&(0x77 == p[3]))
		{
			if(p[4] < 0x80)printf("r13d = rdi + rsi*2 + 0x%x\n", p[4]);
			else printf("r13d = rdi + rsi*2 - 0x%x\n", 0x100-p[4]);
			j += 4;
		}
		else if((0x44 == p[0])&&(0x8d == p[1])&&(0x40 == (p[2]&0xc0)))
		{
			printf("r%dd = %s", 8+((p[2]&0x3f)>>3), reg64[p[2]&0x7]);
			if(p[3] < 0x80)printf(" + 0x%x\n", p[3]);
			else printf(" - 0x%x\n", 0x100-p[3]);
			j += 3;
		}
		else if((0x45 == p[0])&&(0x0f == p[1])&&(0x49 == p[2])&&(0xee == p[3]))
		{
			printf("if(ns)r13d = r14d\n");
			j += 3;
		}
		else if((0x45 == p[0])&&(0x0f == p[1])&&(0xbe == p[2])&&(0x0c == p[3])&&(0x36 == p[4]))
		{
			printf("r9d = [r14+rsi,1]\n");
			j += 4;
		}
		else if((0x45 == p[0])&&(0x31 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("r%dd ^= r%dd\n", 8+(p[2]&0x7), 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x45 == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("r%dd ? r%dd\n", 8+(p[2]&0x7), 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x45 == p[0])&&(0x85 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("flag = r%dd & r%dd\n", 8+(p[2]&0x7), 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x45 == p[0])&&(0x89 == p[1])&&(0x13 == p[2]))
		{
			printf("[r11] = r10d\n");
			j += 2;
		}
		else if((0x45 == p[0])&&(0x8d == p[1])&&(0x68 == (p[2]&0xf8)))
		{
			printf("r13d = r%d + 0x%x\n", 8+(p[2]&0x7), p[3]);
			j += 3;
		}
		else if((0x45 == p[0])&&(0x8d == p[1])&&(0x7d == p[2]))
		{
			printf("r15d = r13 + 0x%x\n", p[3]);
			j += 3;
		}
		else if((0x48 == p[0])&&(0x01 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s += %s\n", reg64[p[2]&0x7], reg64[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x48 == p[0])&&(0x03 == p[1])&&(0x05 == p[2]))
		{
			printf("rax += [+0x%x]\n", *(u32*)(p+3));
			j += 6;
		}
		else if((0x48 == p[0])&&(0x03 == p[1])&&(0x1d == p[2]))
		{
			printf("rbx += [rip+0x%x]\n", *(u32*)(p+3));
			j += 6;
		}
		else if((0x48 == p[0])&&(0x09 == p[0])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s |= %s\n", reg64[p[2]&0x7], reg64[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x48 == p[0])&&(0x0f == p[1])&&(0x4f == p[2])&&(0x05 == p[3]))
		{
			printf("if(g)rax = [+%x]\n", *(u32*)(p+4));
			j += 7;
		}
		else if((0x48 == p[0])&&(0x0f == p[1])&&(0x4f == p[2])&&(0xc2 == p[3]))
		{
			printf("if(g)rax = rdx\n");
			j += 3;
		}
		else if((0x48 == p[0])&&(0x03 == p[1])&&(0x4d == p[2]))
		{
			printf("rcx += [rbp + 0x%x]\n", p[3]);
			j += 3;
		}
		else if((0x48 == p[0])&&(0x21 == p[1])&&(0xf8 == p[2]))
		{
			printf("rax &= rdi\n");
			j += 2;
		}
		else if((0x48 == p[0])&&(0x29 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s -= %s\n", reg64[p[2]&0x7], reg64[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x48 == p[0])&&(0x31 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s ^= %s\n", reg64[p[2]&0x7], reg64[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x48 == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s ? %s\n", reg64[p[2]&0x7], reg64[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x48 == p[0])&&(0x3d == p[1]))
		{
			printf("rax ? 0x%x\n", *(u32*)(p+2));
			j += 5;
		}
		else if((0x48 == p[0])&&(0x63 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s = (s64)%s\n", reg64[(p[2]&0x3f)>>3], reg32[p[2]&0x7]);
			j += 2;
		}
		else if((0x48 == p[0])&&(0x81 == p[1])&&(0xc0 == (p[2]&0xf8)))
		{
			printf("%s += 0x%x\n", reg64[p[2]&0x7], *(u32*)(p+3));
			j += 6;
		}
		else if((0x48 == p[0])&&(0x81 == p[1])&&(0xec == p[2]))
		{
			printf("rsp -= 0x%x\n", *(u32*)(p+3));
			j += 6;
		}
		else if((0x48 == p[0])&&(0x83 == p[1])&&(0xc0 == (p[2]&0xf8)))
		{
			if(p[3] < 0x80)printf("%s += 0x%x\n", reg64[p[2]&0x7], p[3]);
			else printf("%s -= 0x%x\n", reg64[p[2]&0x7], 0x100-p[3]);
			j += 3;
		}
		else if((0x48 == p[0])&&(0x83 == p[1])&&(0xc8 == (p[2]&0xf8)))
		{
			printf("%s |= 0x%x\n", reg64[p[2]&0x7], p[3]);
			j += 3;
		}
		else if((0x48 == p[0])&&(0x83 == p[1])&&(0xe8 == (p[2]&0xf8)))
		{
			printf("%s -= %x\n", reg64[p[2]&0x7], p[3]);
			j += 3;
		}
		else if((0x48 == p[0])&&(0x83 == p[1])&&(0xf8 == (p[2]&0xf8)))
		{
			printf("%s ? 0x%x\n", reg64[p[2]&0x7], p[3]);
			j += 3;
		}
		else if((0x48 == p[0])&&(0x89 == p[1])&&(0x02 == p[2]))
		{
			printf("[rdx] = rax\n");
			j += 2;
		}
		else if((0x48 == p[0])&&(0x89 == p[1])&&(0x05 == p[2]))
		{
			printf("[+%x] = rax\n", *(u32*)(p+3));
			j += 6;
		}
		else if((0x48 == p[0])&&(0x89 == p[1])&&(0x45 == p[2]))
		{
			printf("[rbp+0x%x] = rax\n", p[3]);
			j += 3;
		}
		else if((0x48 == p[0])&&(0x89 == p[1])&&(0x3d == p[2]))
		{
			printf("[+%x] = rdi\n", *(u32*)(p+3));
			j += 6;
		}
	/*
		else if((0x48 == p[0])&&(0x89 == p[1])&&(0x48 == (p[2]&0xf8))
		{
			printf("[%s+0x%x]=rax\n", reg32[p[2]&0x7], p[3]);
			j += 3;
		}
	*/
		else if((0x48 == p[0])&&(0x89 == p[1])&&(0x44 == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp + 0x%x] = rax\n", p[4]);
			j += 4;
		}
		else if((0x48 == p[0])&&(0x89 == p[1])&&(0x4c == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp + 0x%x] = rcx\n", p[4]);
			j += 4;
		}
		else if((0x48 == p[0])&&(0x89 == p[1])&&(0x54 == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp + 0x%x] = rdx\n", p[4]);
			j += 4;
		}
		else if((0x48 == p[0])&&(0x89 == p[1])&&(0x74 == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp + 0x%x] = rsi\n", p[4]);
			j += 4;
		}
		else if((0x48 == p[0])&&(0x89 == p[1])&&(0x7c == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp + 0x%x] = rdi\n", p[4]);
			j += 4;
		}
		else if((0x48 == p[0])&&(0x89 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s = %s\n", reg64[p[2]&0x7], reg64[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x48 == p[0])&&(0x8b == p[1])&&(0x35 == p[2]))
		{
			printf("rsi = [+%x]\n", *(u32*)(p+3));
			j += 6;
		}
		else if((0x48 == p[0])&&(0x8b == p[1])&&(0x05 == p[2]))
		{
			printf("rax = [+%x]\n", *(int*)(p+3)+j+7);
			j += 6;
		}
		else if((0x48 == p[0])&&(0x8b == p[1])&&(0x15 == p[2]))
		{
			printf("rdx = [+0x%x]\n", *(u32*)(p+3));
			j += 6;
		}
		else if((0x48 == p[0])&&(0x8b == p[1])&&(0x18 == p[2]))
		{
			printf("rbx = [rax]\n");
			j += 2;
		}
		else if((0x48 == p[0])&&(0x8b == p[1])&&(0x39 == p[2]))
		{
			printf("rdi = [rcx]\n");
			j += 2;
		}
		else if((0x48 == p[0])&&(0x8b == p[1])&&(0x3d == p[2]))
		{
			printf("rdi = [+0x%x]\n", *(u32*)(p+3));
			j += 6;
		}
		else if((0x48 == p[0])&&(0x8b == p[1])&&(0x4d == p[2]))
		{
			printf("rcx = [rbp + 0x%x]\n", p[3]);
			j += 3;
		}
		else if((0x48 == p[0])&&(0x8d == p[1])&&(0x3d == p[2]))
		{
			printf("rdi = rip + 0x%x\n", *(u32*)(p+3));
			j += 6;
		}
		else if((0x48 == p[0])&&(0x8d == p[1])&&(0x41 == p[2]))
		{
			if(p[3] < 0x80)printf("rax = rcx + 0x%x\n", p[3]);
			else printf("rax = rcx - 0x%x\n", 0x100 - p[3]);
			j += 3;
		}
		else if((0x48 == p[0])&&(0x8d == p[1])&&(0x44 == p[2])&&(0x24 == p[3]))
		{
			printf("rax = rsp + 0x%x\n", p[4]);
			j += 4;
		}
		else if((0x48 == p[0])&&(0x8d == p[1])&&(0x6c == p[2])&&(0x07 == p[3]))
		{
			printf("rbp = rdi + rax + 0x%x\n", p[4]);
			j += 4;
		}
		else if((0x48 == p[0])&&(0x8d == p[1])&&(0x84 == p[2])&&(0x24 == p[3]))
		{
			printf("rax = [rsp + 0x%x]\n", p[4]);
			j += 7;
		}
		else if((0x48 == p[0])&&(0x8d == p[1])&&(0x80 == (p[2]&0xf8)))
		{
			printf("rax = %s + 0x%x\n", reg64[p[2]&0x7], *(u32*)(p+3));
			j += 6;
		}
		else if((0x48 == p[0])&&(0x8d == p[1])&&(0xbc == p[2])&&(0x17 == p[3]))
		{
			printf("rdi = rdi + rdx + 0x%x\n", *(u32*)(p+4));
			j += 7;
		}
		else if((0x48 == p[0])&&(0x98 == p[1]))
		{
			printf("cdqe\n");
			j += 1;
		}
		else if((0x48 == p[0])&&(0xc1 == p[1])&&(0xe0 == (p[2]&0xf8)))
		{
			printf("%s <<= %x\n", reg64[p[2]&0x7], p[3]);
			j += 3;
		}
		else if((0x48 == p[0])&&(0xc1 == p[1])&&(0xe8 == (p[2]&0xf8)))
		{
			printf("%s >>= %x\n", reg64[p[2]&0x7], p[3]);
			j += 3;
		}
		else if((0x48 == p[0])&&(0xc7 == p[1])&&(0x05 == p[2]))
		{
			printf("[+%x] = %x\n", *(u32*)(p+3), *(u32*)(p+7));
			j += 10;
		}
		else if((0x49 == p[0])&&(0x01 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("r%d += %s\n", 8+(p[2]&0x7), reg64[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x49 == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xf8)))
		{
			printf("r%d ? %s\n", 8+(p[2]&0x7), reg64[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x49 == p[0])&&(0x63 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s = (s64)r%dd\n", reg64[(p[2]&0x3f)>>3], 8+(p[2]&0x7));
			j += 2;
		}
		else if((0x49 == p[0])&&(0x81 == p[1])&&(0xc2 == p[2]))
		{
			printf("r10 += 0x%x\n", *(u32*)(p+3));
			j += 6;
		}
		else if((0x49 == p[0])&&(0x83 == p[1])&&(0xc0 == (p[2]&0xf8)))
		{
			printf("r%d += 0x%x\n", 8+(p[2]&0x7), p[3]);
			j += 3;
		}
		else if((0x49 == p[0])&&(0x89 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("r%d = %s\n", 8+(p[2]&0x7), reg64[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x49 == p[0])&&(0x8d == p[1])&&(0x04 == p[2])&&(0x3c == p[3]))
		{
			printf("rax = r12+rdi\n");
			j += 3;
		}
		else if((0x49 == p[0])&&(0x8d == p[1])&&(0x3c == p[2])&&(0x04 == p[3]))
		{
			printf("rdi = r12+rax\n");
			j += 3;
		}
		else if((0x49 == p[0])&&(0x8d == p[1])&&(0x58 == (p[2]&0xf8)))
		{
			printf("rbx = r%d + 0x%x\n", 8+(p[2]&0x7), p[3]);
			j += 3;
		}
		else if((0x49 == p[0])&&(0xc1 == p[1])&&(0xe0 == (p[2]&0xf8)))
		{
			printf("r%d <<= 0x%x\n", 8+(p[2]&0x7), p[3]);
			j += 3;
		}
		else if((0x49 == p[0])&&(0xc1 == p[1])&&(0xe8 == (p[2]&0xf8)))
		{
			printf("r%d >>= 0x%x\n", 8+(p[2]&0x7), p[3]);
			j += 3;
		}
		else if((0x4b == p[0])&&(0x8d == p[1])&&(0x3c == p[2])&&(0x2c == p[3]))
		{
			printf("rdi = r12+r13\n");
			j += 3;
		}
		else if((0x4a == p[0])&&(0x8d == p[1])&&(0x4c == p[2])&&(0x1f == p[3]))
		{
			printf("rcx = rdi + r11 + 0x%x\n", p[4]);
			j += 4;
		}
		else if((0x4c == p[0])&&(0x01 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s += r%d\n", reg64[p[2]&0x7], 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x4c == p[0])&&(0x03 == p[1])&&(0x05 == p[2]))
		{
			printf("r8 += [rip + 0x%x]\n", *(u32*)(p+3));
			j += 6;
		}
		else if((0x4c == p[0])&&(0x0f == p[1])&&(0x4c == p[2])&&(0xe8 == p[3]))
		{
			printf("if(l)r13 = rax\n");
			j += 3;
		}
		else if((0x4c == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s ? r%d\n", reg64[(p[2]&0x7)], 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x4c == p[0])&&(0x63 == p[1])&&(0xef == p[2]))
		{
			printf("r13 = (s64)edi\n");
			j += 2;
		}
		else if((0x4c == p[0])&&(0x89 == p[1])&&(0x44 == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp + 0x%x] = r8\n", p[4]);
			j += 4;
		}
		else if((0x4c == p[0])&&(0x89 == p[1])&&(0x4c == p[2])&&(0x24 == p[3]))
		{
			printf("[rsp + 0x%x] = r9\n", p[4]);
			j += 4;
		}
		else if((0x4c == p[0])&&(0x89 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s = r%d\n", reg64[p[2]&0x7], 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x4c == p[0])&&(0x8b == p[1])&&(0x25 == p[2]))
		{
			printf("r12 = [+0x%x]\n", *(u32*)(p+3));
			j += 6;
		}
		else if((0x4c == p[0])&&(0x8b == p[1])&&(0x29 == p[2]))
		{
			printf("r13 = [rcx]\n");
			j += 2;
		}
		else if((0x4c == p[0])&&(0x8b == p[1])&&(0x7c == p[2])&&(0x24 == p[3]))
		{
			printf("r15 = rsp + 0x%x\n", p[4]);
			j += 4;
		}
		else if((0x4c == p[0])&&(0x8d == p[1])&&(0x0c == p[2])&&(0x02 == p[3]))
		{
			printf("r9 = [rdx+rax]\n");
			j += 3;
		}
		else if((0x4c == p[0])&&(0x8d == p[1])&&(0x44 == p[2])&&(0x24 == p[3]))
		{
			printf("r8 = rsp + 0x%x\n", p[4]);
			j += 4;
		}
		else if((0x4d == p[0])&&(0x01 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("r%d += r%d\n", 8+(p[2]&0x7), 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x4d == p[0])&&(0x31 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("r%d ^= r%d\n", 8+(p[2]&0x7), 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x4d == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("r%d ? r%d\n", 8+(p[2]&0x7), 8+((p[2]&0x3f)>>3));
			j += 2;
		}
		else if((0x4d == p[0])&&(0x63 == p[1])&&(0xc0 == p[2]))
		{
			printf("r8 = (s64)r8d\n");
			j += 2;
		}
		else if((0x4d == p[0])&&(0x8d == p[1])&&(0x54 == p[2]))
		{
			printf("r10 = r14 + rsi + 0x%x\n", p[4]);
			j += 4;
		}
		else if((0x4d == p[0])&&(0x8d == p[1])&&(0x99 == p[2]))
		{
			printf("r11 = r9 + 0x%x\n", *(u32*)(p+3));
			j += 6;
		}
		else if((p[0] >= 0x50)&&(p[0] <= 0x57))
		{
			printf("push %s\n", reg64[p[0]&0x7]);
		}
		else if((p[0] >= 0x58)&&(p[0] <= 0x5f))
		{
			printf("pop %s\n", reg64[p[0]&0x7]);
		}
		else if((0x66 == p[0])&&(0x09 == p[0])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s |= %s\n", reg16[p[2]&0x7], reg16[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x66 == p[0])&&(0x2e == p[1])&&(0x0f == p[2])&&(0x1f == p[3]))
		{
			printf("nop10\n");
			j += 9;
		}
		else if((0x66 == p[0])&&(0x31 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s ^= %s\n", reg16[p[2]&0x7], reg16[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x66 == p[0])&&(0x39 == p[1])&&(0xc0 == (p[2]&0xc0)))
		{
			printf("%s ? %s\n", reg16[p[2]&0x7], reg16[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x66 == p[0])&&(0x41 == p[1])&&(0x89 == p[2])&&(0x0 == (p[3]&0xc0)))
		{
			printf("[%d] = %s\n", 8+(p[3]&0x7), reg16[(p[3]&0x3f)>>3]);
			j += 3;
		}
		else if((0x66 == p[0])&&(0x41 == p[1])&&(0x89 == p[2])&&(0x41 == p[3]))
		{
			printf("[r9+0x%x] = ax\n", p[4]);
			j += 4;
		}
		else if((0x66 == p[0])&&(0x66 == p[1]))
		{
			j += 1;
			printf("6666\n");
			if(0x2e == p[4])j += 1;
		}
		else if((0x66 == p[0])&&(0x83 == p[1])&&(0xc8 == (p[2]&0xf8)))
		{
			printf("%s |= 0x%x\n", reg16[p[2]&0x7], p[3]);
			j += 3;
		}
		else if((0x66 == p[0])&&(0x83 == p[1])&&(0xf8 == (p[2]&0xf8)))
		{
			printf("%s ? 0x%x\n", reg16[p[2]&0x7], p[3]);
			j += 3;
		}
		else if((0x66 == p[0])&&(0x89 == p[1])&&(0x0 == (p[2]&0xc0)))
		{
			printf("[%s] = %s\n", reg64[p[2]&0x7], reg16[(p[2]&0x3f)>>3]);
			j += 2;
		}
		else if((0x66 == p[0])&&(0x89 == p[1])&&(0x42 == p[2]))
		{
			printf("[rdx+0x%x] = ax\n", p[3]);
			j += 3;
		}
		else if((0x66 == p[0])&&(0x90 == p[1]))
		{
			printf("xchg ax,ax\n");
			j += 1;
		}
		else if(0x70 == (p[0]&0xf0))
		{
			if(0x70 == p[0])printf("if(o)");
			else if(0x71 == p[0])printf("if(no)");
			else if(0x72 == p[0])printf("if(c)");
			else if(0x73 == p[0])printf("if(nc)");
			else if(0x74 == p[0])printf("if(z)");
			else if(0x75 == p[0])printf("if(nz)");
			else if(0x76 == p[0])printf("if(na)");
			else if(0x77 == p[0])printf("if(a)");
			else if(0x78 == p[0])printf("if(s)");
			else if(0x79 == p[0])printf("if(ns)");
			else if(0x7a == p[0])printf("if(pe)");
			else if(0x7b == p[0])printf("if(po)");
			else if(0x7c == p[0])printf("if(l)");
			else if(0x7d == p[0])printf("if(nl)");
			else if(0x7e == p[0])printf("if(ng)");
			else if(0x7f == p[0])printf("if(g)");
			printf("jmp %x\n", (char)p[1] + (j+2));
			j += 1;
		}
		else if((0x80 == p[0])&&(0x3d == p[1]))
		{
			printf("[rip+0x%x,1] ? 0x%x\n", *(u32*)(p+2), p[6]);
			j += 6;
		}
		else if((0x80 == p[0])&&(0x4a == p[1]))
		{
			printf("[rdx + 0x%x, 1] |= 0x80\n", p[2]);
			j += 3;
		}
		else if((0x80 == p[0])&&(0xc8 == (p[1]&0xf8)))
		{
			printf("%s |= 0x%x\n", reg08[p[1]&0x7], p[2]);
			j += 2;
		}
		else if((0x80 == p[0])&&(0xf8 == (p[1]&0xf8)))
		{
			printf("%s ? 0x%x\n", reg08[p[1]&0x7], p[2]);
			j += 2;
		}
		else if((0x81 == p[0])&&(0xc8 == (p[1]&0xf8)))
		{
			printf("%s |= 0x%x\n", reg32[p[1]&0x7], *(u32*)(p+2));
			j += 5;
		}
		else if((0x81 == p[0])&&(0xe0 == (p[1]&0xf8)))
		{
			printf("%s &= 0x%x\n", reg32[p[1]&0x7], *(u32*)(p+2));
			j += 5;
		}
		else if((0x81 == p[0])&&(0xf8 == (p[1]&0xf8)))
		{
			printf("%s ? 0x%x\n", reg32[p[1]&0x7], *(u32*)(p+2));
			j += 5;
		}
		else if((0x83 == p[0])&&(0xc0 == (p[1]&0xf8)))
		{
			printf("%s += 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 2;
		}
		else if((0x83 == p[0])&&(0xc8 == (p[1]&0xf8)))
		{
			printf("%s |= 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 2;
		}
		else if((0x83 == p[0])&&(0xe0 == (p[1]&0xf8)))
		{
			printf("%s &= 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 2;
		}
		else if((0x83 == p[0])&&(0xe8 == (p[1]&0xf8)))
		{
			printf("%s -= 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 2;
		}
		else if((0x83 == p[0])&&(0xf8 == (p[1]&0xf8)))
		{
			printf("%s ? 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 2;
		}
		else if((0x84 == p[0])&&(0xc0 == p[1]))
		{
			printf("flag = al & al\n");
			j += 1;
		}
		else if((0x84 == p[0])&&(0xc9 == p[1]))
		{
			printf("flag = cl & cl\n");
			j += 1;
		}
		else if((0x85 == p[0])&&(0xc0 == (p[1]&0xc0)))
		{
			printf("flag = %s & %s\n", reg32[p[1]&0x7], reg32[(p[1]&0x3f)>>3]);
			j += 1;
		}
		else if((0x88 == p[0])&&(0x0c == p[1])&&(0x17 == p[2]))
		{
			printf("[rdi+rdx] = cl\n");
			j += 2;
		}
		else if((0x88 == p[0])&&(0x0 == (p[1]&0xc0)))
		{
			printf("[%s] = %s\n", reg64[p[1]&0x7], reg08[(p[1]&0x3f)>>3]);
			j += 1;
		}
		else if((0x89 == p[0])&&(0x0 == (p[1]&0xc0)))
		{
			printf("[%s] = %s\n", reg64[p[1]&0x7], reg32[(p[1]&0x3f)>>3]);
			j += 1;
		}
		else if((0x89 == p[0])&&(0x45 == p[1]))
		{
			printf("[rbp + 0x%x] = eax\n", p[2]);
			j += 2;
		}
		else if((0x89 == p[0])&&(0x44 == p[1])&&(0x24 == p[2]))
		{
			printf("[rsp + 0x%x] = eax\n", p[3]);
			j += 3;
		}
		else if((0x89 == p[0])&&(0x4c == p[1])&&(0x24 == p[2]))
		{
			printf("[rsp + 0x%x] = ecx\n", p[3]);
			j += 3;
		}
		else if((0x89 == p[0])&&(0x54 == p[1])&&(0x24 == p[2]))
		{
			printf("[rsp + 0x%x] = edx\n", p[3]);
			j += 3;
		}
		else if((0x89 == p[0])&&(0x40 == (p[1]&0xc0)))
		{
			printf("[%s + 0x%x] = %s\n", reg64[p[1]&0x7], p[2], reg32[(p[1]&0x3f)>>3]);
			j += 2;
		}
		else if((0x89 == p[0])&&(0x82 == p[1]))
		{
			printf("[rdx + 0x%x] = eax\n", *(u32*)(p+2));
			j += 5;
		}
		else if((0x89 == p[0])&&(0xba == p[1]))
		{
			printf("[rdx+0x%x] = edi\n", *(u32*)(p+2));
			j += 5;
		}
		else if((0x89 == p[0])&&(0xc0 == (p[1]&0xc0)))
		{
			printf("%s = %s\n", reg32[p[1]&0x7], reg32[(p[1]&0x3f)>>3]);
			j += 1;
		}
		else if((0x8b == p[0])&&(0x05 == p[1]))
		{
			printf("eax = [rip+0x%x]\n", *(u32*)(p+2));
			j += 5;
		}
		else if((0x8b == p[0])&&(0x09 == p[1]))
		{
			printf("ecx = [rcx]\n");
			j += 1;
		}
		else if((0x8b == p[0])&&(0x1d == p[1]))
		{
			printf("ebx = [rip + 0x%x]\n", *(u32*)(p+2));
			j += 5;
		}
		else if((0x8b == p[0])&&(0x44 == p[1])&&(0x24 == p[2]))
		{
			printf("eax = [rsp + 0x%x]\n", p[3]);
			j += 3;
		}
		else if((0x8b == p[0])&&(0x54 == p[1])&&(0x24 == p[2]))
		{
			printf("edx = [rsp + 0x%x]\n", p[3]);
			j += 3;
		}
		else if((0x8b == p[0])&&(0x74 == p[1])&&(0x24 == p[2]))
		{
			printf("esi = [rsp + 0x%x]\n", p[3]);
			j += 3;
		}
		else if((0x8b == p[0])&&(0x40 == (p[1]&0xc0)))
		{
			printf("%s = [%s + 0x%x]\n", reg32[(p[1]&0x3f)>>3], reg64[p[1]&0x7], p[2]);
			j += 2;
		}
		else if((0x8b == p[0])&&(0x82 == p[1]))
		{
			printf("eax = [rdx+0x%x]\n", *(u32*)(p+2));
			j += 5;
		}
		else if((0x8d == p[0])&&(0x04 == p[1])&&(0x13 == p[2]))
		{
			printf("eax = rbx+rdx\n");
			j += 2;
		}
		else if((0x8d == p[0])&&(0x0c == p[1])&&(0x02 == p[2]))
		{
			printf("ecx = rdx+rax\n");
			j += 2;
		}
		else if((0x8d == p[0])&&(0x0c == p[1])&&(0x18 == p[2]))
		{
			printf("ecx = rax+rbx\n");
			j += 2;
		}
		else if((0x8d == p[0])&&(0x14 == p[1])&&(0x06 == p[2]))
		{
			printf("edx = rsi+rax\n");
			j += 2;
		}
		else if((0x8d == p[0])&&(0x40 == (p[1]&0xc0)))
		{
			printf("%s = %s", reg32[(p[1]&0x3f)>>3], reg64[p[1]&0x7]);
			if(p[2] < 0x80)printf(" + 0x%x\n", p[2]);
			else printf(" - 0x%x\n", 0x100-p[2]);
			j += 2;
		}
		else if((0x8d == p[0])&&(0x84 == p[1])&&(0x10 == p[2]))
		{
			printf("eax = rax + rdx + 0x%x\n", *(u32*)(p+3));
			j += 6;
		}
		else if(0x90 == p[0])
		{
			printf("nop\n");
		}
		else if(0xa8 == p[0])
		{
			printf("flag = al & 0x%x\n", p[1]);
			j += 1;
		}
		else if(0xa9 == p[0])
		{
			printf("flag = eax & 0x%x\n", *(u32*)(p+1));
			j += 4;
		}
		else if(0xb8 == (p[0]&0xf8))
		{
			printf("%s = 0x%x\n", reg32[p[0]&0x7], *(u32*)(p+1));
			j += 4;
		}
		else if((0xc1 == p[0])&&(0xf8 == (p[1]&0xf8)))
		{
			printf("sar %s,0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 2;
		}
		else if((0xc1 == p[0])&&(0xe0 == (p[1]&0xf8)))
		{
			printf("%s <<= 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 2;
		}
		else if((0xc1 == p[0])&&(0xe8 == (p[1]&0xf8)))
		{
			printf("%s >>= 0x%x\n", reg32[p[1]&0x7], p[2]);
			j += 2;
		}
		else if(0xc3 == p[0])
		{
			printf("ret\n");
		}
		else if(0xc6 == p[0])
		{
			if((0x04 == p[1])&&(0x17 == p[2]))
			{
				printf("[rdi+rdx] = 0x%x\n", p[3]);
				j += 3;
			}
			else if(0x42 == p[1])
			{
				printf("[rdx + 0x%x, 1] = 0x%x\n", p[2], p[3]);
				j += 3;
			}
			else if(0x5 == p[1])
			{
				printf("[rip + 0x%x, 1] = 0x%x\n", *(u32*)(p+2), p[6]);
				j += 6;
			}
			else
			{
				printf("[%s,1] = 0x%x\n", reg64[p[1]&0x7], p[2]);
				j += 2;
			}
		}
		else if((0xc7 == p[0])&&(0x43 == p[1]))
		{
			printf("[rbx + 0x%x, 4] = 0x%x\n", p[2], *(u32*)(p+3));
			j += 6;
		}
		else if((0xc7 == p[0])&&(0x44 == p[1])&&(0x24 == p[2]))
		{
			printf("[rsp + 0x%x,4] = 0x%x\n", p[3], *(u32*)(p+4));
			j += 7;
		}
		else if(0xcc == p[0])
		{
			printf("int3\n");
		}
		else if((0xd1 == p[0])&&(0xe0 == (p[1]&0xf8)))
		{
			printf("%s <<= 1\n", reg32[p[1]&0x7]);
			j += 1;
		}
		else if((0xd1 == p[0])&&(0xe8 == (p[1]&0xf8)))
		{
			printf("%s >>= 1\n", reg32[p[1]&0x7]);
			j += 1;
		}
		else if((0xd3 == p[0])&&(0xe0 == (p[1]&0xf0)))
		{
			printf("%s <<= cl\n", reg32[p[1]&0x7]);
			j += 1;
		}
		else if((0xd3 == p[0])&&(0xe8 == (p[1]&0xf0)))
		{
			printf("%s >>= cl\n", reg32[p[1]&0x7]);
			j += 1;
		}
		else if(0xe8 == p[0])
		{
			printf("call 0x%x\n", *(u32*)(p+1) + (j+5));
			j += 4;
		}
		else if(0xe9 == p[0])
		{
			printf("jmp 0x%x\n", *(u32*)(p+1) + (j+5));
			j += 4;
		}
		else if(0xeb == p[0])
		{
			printf("jmp 0x%x\n", (j+2) + ((char)p[1]));
			j += 1;
		}
		else if((0xf2 == p[0])&&(0x0f == p[1])&&(0x10 == p[2])&&(0x01 == p[3]))
		{
			printf("xmm0 = [rcx]\n");
			j += 3;
		}
		else if(0xf3 == p[0])
		{
			printf("rep\n");
		}
		else if((0xf6 == p[0])&&(0xc0 == (p[1]&0xf8)))
		{
			printf("flag = %s & 0x%x\n", reg08[p[1]&0x7], p[2]);
			j += 2;
		}
		else
		{
			printf("%02x\n", p[0]);
		}
	}
}
void disasm_x8664(int argc, char** argv)
{
	int fd,ret;
	u8 buf[0x100000];
	if(argc < 2)return;

	fd = open(argv[1] , O_RDONLY);
	if(fd <= 0)
	{
		printf("error@open\n");
		return;
	}

	ret = read(fd, buf, 0x100000);
	if(ret <= 0)
	{
		goto theend;
	}

	disasm_x8664_one(buf, ret);

theend:
	close(fd);
}
