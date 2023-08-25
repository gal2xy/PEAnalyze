- 寻址范围 —— 计算机位数(32,64)

- 内存单元 —— 1字节

- <span style="color:red">**window存储，小端模式，低位在前。**</span>

  例如：

  在内存中为E4 7C B3 B6，则实际数据为B6B37CE4
  
- OEP：原始程序入口





### 通用寄存器

![](pictures/通用寄存器.png)

**通用寄存器在cpu中**

cpu、主存（内存RAM）、外存三者关系如下：

![](pictures/CPU_主存(内存)_外存关系.png)



### mov指令

- movl：用于32位bit（双字）
- movw：用于16位bit（字）
- movb：用于8位bit（字节）

mov指令的两个操作数不能同时为内存单元，因为mov指令执行需要一个总线周期，内存单元的数据到CPU需要一个总线周期。倘若同时为内存单元，则需要先读到CPU中，再存到内存中，此时需要两个总线周期，因此是不可行的。



<span style="color:red">32位机是指它的寻址范围是32位(每一个编号对应的内存单位是8字节)，并不是指寄存器是32位。</span>

内存单元是8字节，内存编号用 [编号] 表示。

**涉及到内存读写，一定要指明宽度！！！**(且立即数的宽度要一致)

如：

- mov **byte** ptr ds:[0x12345678], 0xff
- mov **word** ptr ds:[0x12345678], 0xffff
- mov **dword** ptr ds:[0x12345678], 0xffffffff

 

### LEA指令

获取内存编号

如：

​	mov EAX,[0x12345678]	将地址编号0x12345678赋给EAX，而不是地址编号对应的内容



### ADC指令

带进位加

### SBB指令

带借位减

### xchg指令

两个操作数交换

### MOVS指令

移动数据（内存到内存，**内存就需要指明宽度**）

movsb(字节)

movsw(字)

movsd(双字)

例：

movs  dword ptr ES:[EDI], dword ptr ES:[ESI]	（**执行完后，EDI和ESI的值都要+4{双字}**）

### MOVSX指令

带符号扩展传输指令

### MOVZX指令

零扩展传输指令

### STOS指令

将AL/AX/EAX的值存储到[EDI]指定的内存单元

EDI的增长方向由DF标志位决定

STOSB(字节)

STOSW(字)

STOSD(双字)

### REP指令

重复 ，次数为cx的值

### CMP指令

实质是减法，但只改变标志寄存器

### 条件跳转指令



### JMP指令

跳转指令

### CALL指令

函数调用指令

返回地址压入栈，

### RETN指令

返回指令

返回地址出栈



### 移位指令

- SAL 算术左移

  CF存高位，低位补0

- SAR 算术右移

  CF存低位，高位补符号位

- SHL 逻辑左移

- SHR 逻辑右移

- ROL 循环左移

- ROR 循环右移

  ![](pictures/循环移位.png)

- 带进位的循环左移 RCL

- 带进位的循环右移 RCR

  ![](pictures/带进位的循环移位.png)



### 寻址公式

reg代表寄存器。

1. [立即数]

2. [reg]

3. [reg+立即数]

4. [reg+reg*{1,2,4,8}]

   当为[reg+ reg*4]是，可能是整形数组

5. [reg+reg*{1,2,4,8}+立即数]



### 堆栈

堆栈一般是从高地址向低地址生长。

一般EBP存栈底，ESP存栈顶

- push：操作数入栈

  r指寄存器、m指内存、imm指立即数

  push r16/m16		ESP - 2

  push r32/ m32		ESP - 4

  push imm8/imm16/imm32		不管多少位，ESP都是 - 4

  

- pop：栈顶数据出栈

  pop r16/m16		ESP - 2

  pop r32/m32		ESP - 4

**push/pop：当操作数位内存或者寄存器时，ESP变化依据容器的大小**

- pushad/popad

  将8个通用寄存器(EAX、ECX、EDX、EBX、ESP、EBP、ESI、EDI)入栈或出栈



### 标志寄存器

| 标志 |      |      |      |      | OF   | DF   | IF   | TF   | SF   | ZF   |      | AF   |      | PF   |      | CF   |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| 位号 | 15   | 14   | 13   | 12   | 11   | 10   | 9    | 8    | 7    | 6    | 5    | 4    | 3    | 2    | 1    | 0    |



- **OF（溢出标志，Overflow Flag）**：指示**有符号算术运算**后数据的高位溢出。

  ==双符号位判溢法==

- **DF（方向标志，Direction Flag）**：<u>确定向左或向右移动或比较字符串数据的方向</u>。

  字符串操作对象一般为ESI和EDI

  **DF** 值为 0 时，字符串操作为从左至右的方向，ESI和EDI增加

  **DF** 值为 1 时，字符串操作为从右至左的方向，ESI和EDI减少

  ESI寄存器常用于存放源字符串的地址（即字符串的起始地址），而EDI寄存器则常用于存放目标字符串的地址（即字符串将要复制到的位置的起始地址）

- **IF（中断标志，Interrupt Flag）**：确定是否忽略或处理<u>外部中断</u>（例如键盘输入等）。

  为 0 时，它禁用外部中断

  为 1 时，它使能中断。

- **TF（单步标志，Trap Flag）**：允许在单步模式下设置处理器的操作。我们使用的 `DEBUG` 程序设置了陷阱标志，因此我们可以一次逐步执行一条指令。

- **SF（符号标志，Sign Flag）**：反应**运算**结果的符号位，与<u>运算结果的最高位</u>相同。结果为正，置0，结果为负，置1。

- **ZF（零标志，Zero Flag）**：指示**算术或比较运算**的结果。结果为0则置1，结果非零则置0。

- **AF（辅助进位标志，Auxiliary Carry Flag）**：**<span style="color:red">半进位标志</span>**

- **PF（奇偶标志，Parity Flag）**：指示从**算术运算**获得的结果（<u>*实际参与运算的有效部分*</u>）中 1 位的总数。

  偶数个 1 将奇偶校验标志为 1

  奇数个 1 将奇偶校验标志为 0

- **CF（进位标志，Carry Flag）**：在**算术运算**后，最高位产生了借位或进位，则置1，否则置0。<u>它还存储移位或旋转操作的最后一位的内容。</u>

  例如：

  1. add al, 0xff	如果运算后的结果大于al的最大范围(0xff)，则溢出
  2. add ax, 0xff	如果运算后的结果大于ax最大范围(0xffff)，则溢出



>  CF与OF的区别
>
> 无符号运算看OF，有符号运算看CF
>
> 有符号数，最高位为符号位，0为正，1为负。
>
> 例如，32位，则正数范围为0x00000000(0)\~0x7FFFFFFF(2,147,483,647)；
>
> 负数范围为0x80000000(-2,147,483,648)\~0xFFFFFFFF(-1)。
>
> 在计算机中，通常采用补码表示**负数**，运算采用补码运算







### 堆栈调用

例子：

![堆栈调用](pictures/堆栈调用.png)

1. ==call指令调用之前，参数已经压入栈了==

2. 调用函数时，将EIP压栈，之后划分堆栈，将寄存器数据压栈，用于恢复现场

3. **EBP-4是局部变量，+8是参数,+4是返回地址**

4. ==堆栈平衡==

   - 内平衡

     内平衡是指被调用函数会自行管理自己使用的堆栈空间，<u>确保在函数退出之前，堆栈的状态与进入函数时相同</u>。

   - 外平衡

     外平衡是指调用函数前<u>由调用者将堆栈指针调整到最初的状态</u>，亦即保证调用函数前后栈的大小不变。

5. PUSHAD: 将所有寄存器的值存入堆栈（保护现场）

   POPAD：恢复现场



### ==裸函数==

不会生成汇编代码，自己可以在C中写汇编代码。

**调用裸函数，不会返回，直接int 3终止，需自己写ret**

功能：用来hook

```c
void __declspec(naked) 函数名(){}

#有返回的裸函数
void __declspec(naked) hook(){

	__asm{
        retn
    }

}
```



例子：

1. 两个数的加法

   ```c
   int __declspec(naked) ADD(int x, int y){
       
      __asm{
   		//栈底入栈
           push ebp
           //提升栈底
           mov ebp,esp
           //提升栈顶
           sub esp,0x40
           //保留现场
           push ebx
           push esi
           push edi
           //填充缓冲区
           mov eax,0xcccccccc
           mov ecx,0x10
           lea edi,dword ptr ds:[ebp-0x40]
           rep stosd
           //函数主要功能
           mov eax,ds:[ebp+0x8]
           add eax,ds:[ebp+0xc]
           //恢复现场
           pop edi
           pop esi
           pop ebx
   		//降低栈顶
           mov esp,ebp
           //栈底出栈
           pop ebp
           //返回主函数
           retn
      } 
       
   }
   
   
   ```

2. 含临时变量的6位数相加

   以下代码有缺陷！负数算不对！！！

   ```c
   int __declspec(naked) ADD(int x, int y, int z){
       
      __asm{
           //保留栈底
           push ebp
           //提升堆栈
           mov ebp,esp
           sub esp,0x40
           //保留现场
           push ebx
           push esi
           push edi
           //填充缓冲区
           mov eax,0xcccccccc
           mov c×,0x10
           lea edi,dword ptr ds:[ebp-0x40]
           rep stosd
           //函数功能
           	//存储临时变量
           mov dword ptr [ebp-0x4],0x2
           mov dword ptr [ebp-0x8],0x3
           mov dword ptr [ebp-0xc],0x4
           	//提取参数相加
           	//需要用到adc,故需将标志位CF,0F清零
   		clc//cF清零
           ×or ea×,eax//OF清零
           mov eax,[ebp+0x8]
           add eax,[ebp+0xc]
           adc ea×,[ebp+0x10]
           	//提取临时变量相加
           adc eax,[ebp-0x4]
           adc eax,[ebp-0x8]
           adc eax,[ebp-0xc]
           adc eax,0
           //恢复现场
           pop edi
           pop esi
           pop ebx
           //降低堆栈
           mov esp,ebp
           pop ebp
           //返回主函数
           retn
      } 
       
   }
   
   
   ```

   





### ==调用约定==

__cdecl : 参数从右至左压栈，外平衡堆栈

__stdcall : 参数从右至左压栈，内平衡堆栈

__fastcall : 参数从右至左看，**最后两个参数放在ECX/EDX中，剩下参数从右至左压栈**，内平衡堆栈

```c
void __cdecl add(){}

void __stdcall add(){}	//ret+数字来平衡

void __fastcall add(){}
```



### 函数参数个数的确定

1. 观察调用处代码

   ```c
   //例：
   push 3
   call 0x40100F
   ```

2. 找到平衡堆栈的代码

   ```c
   //函数外部
   call 0x40100F
   add esp,0x4
   或函数内部
   ret 0x4
   ```

综合分析，确定参数个数。**注意__fastcall 的特殊情况**



### 程序入口

程序开始执行的地方



==如何寻找程序入口？==

查看**调用堆栈窗口（call stack）**,如图所示：

![](pictures/Call Stack.png)

- 调用顺序为：KERNEL32 —> mainCRTStartup —> main

- 我们写的代码的入口是 main()

- 真正的程序入口是 mainCRTStartup()

  此函数会做初始化工作：

  1. GetVersion()
  2. \_heap\_init()
  3. GetCommandLineA()
  4. \_crtGetEnvironmentStringsA()
  5. \_setargv()
  6. \_setenvp()
  7. \_cinit()

  **会调用多个函数，可以根据main函数与某一确定函数名之间的call的个数大致知道main函数的入口地址。**

  例如：

  GetCommandLineA()与main()之间会有4次call

  ![](pictures/如何确定程序入口.png)





### ==内存图==

![](pictures/内存图.png)

从上至下：

- 代码区
- 堆栈 ：参数、局部变量、临时数据
- 堆 ：==动态申请==
- 全局变量区（可读可写）
- 常量区（可读不可写）

1. 基址（base address）一般用来指示一个内存区域的起始地址，它可以是一个全局变量、数组、结构体或者其他数据结构的地址。基址指向的是一个内存区域的起始地址，通过加上偏移量（offset）可以访问到该内存区域中的不同元素
2. 全局变量用地址表示，局部变量用[ebp-num]表示



### IF语句反汇编

一般是一条影响标志位的指令和一个条件跳转指令搭配使用



### 扩展和截取

- 扩展

  小的转大的

- 截取

  大的转小的

  因为window采用小端模式，数据在内存中存放是低位在前，所以截取是从低位开始。

  （其实就是从变量地址开始，截取目标长度）

### 参数传递、返回值传递

按本机尺寸分配内存

1. 参数传递

   对于32位机，小于32bit的参数传递时，都是按32位传入，大于32bit（64bit）的参数传递时，分成多个32bit的参数压入栈传入，对于数组参数，则存入数组的首地址。

   

2. 返回值传递

   对于32位机，小于等于32bit的返回值通过寄存器eax传递，大于32bit（64bit）的返回值通过eax和ebx组合传递，数组通过寄存器eax返回数组首地址传递（如果数组是临时变量，则在下一次函数调用后会被覆盖）。

   



### 数组

给数组分配内存，从低地址向高地址填写

所占内存则根据类型长度来判定。（==地址连续分配==）

例如：

​	对于32位机

​	int型数组，则按dword分配，也就是一个内存单元存放一个元素

​	char型数组，则按byte分配，也就是一个内存单元存放四个元素

​	short型数组，则按word分配，也就是一个内存单元存放两个元素





数组寻址的反汇编代码

```c
//假设数组首地址为ebp-34h
//对于已知下标,如arr[1]
mov eax, dword ptr [ebp-30h]
//对于变量或表达式下标,如arr[x],x=1,x的地址为ebp-4
mov ecx,dword ptr [ebp-4]
mov edx,dword ptr [ebp+ecx*4-34h]

```



==**多维数组**==

例如：arr\[3][4]，在编译器看来是arr\[3*4]。即汇编中分配内存空间的方式一样。



### ==**结构体**==

 结构体首地址+相对偏移量就是结构体中元素的地址

在汇编中，结构体存入通过堆栈，先将esp减去总长度，再用rep movs将结构体传入

作为返回值时，调用前将结构体压入栈，然后返回时将值存入对应位置。

### 字节对齐/结构体对齐

对齐参数有1byte、2byte、4byte、8byte对齐。

默认8byte对齐。

如果使用其他对齐参数，则如下所示

```c
#pragma pack(1)
struct S{
    
}
或
#pragma pack(2)
struct S{
    
}
或
#pragma pack(4)
struct S{
    
}
```

对齐原则：

1. 对于结构体中的每一个成员，其偏移量取**min{对齐参数，自身大小}**的整数倍
2. **结构体的总大小必须是其内部最大成员的整数倍**，不足则补齐
3. 如果结构体中含有结构体，则该结构体成员要从其内部最大元素大小的整数倍地址开始存储

​	

### switch语句反汇编

1. 分支少的时候，用switch与if...else...无异，反汇编代码相似。

2. 如果各分支中常量的大小连续，分支比较多的时候，会生成各个分支地址的大表，将所有分支的跳转指令集成一个带表达式的jmp指令

3. 分支中常量的大小顺序并不会影响大表的生成。（各分支的地址在大表中按顺序填写）

4. 分支中常量的值不会影响大表的生成。

5. 各分支中的常量相差太大会影响大表的生成！！！

6. 各分支中常量连续但缺少某几个（如1，3，4，5，6）不会影响大表生成，缺少的分支在大表中填default分支的地址，使其是真正的连续分支。

   但如果缺少个数较多的时候，会额外生成小表（其实就是缺少分支太多，在大表中所填的default分支的地址太多，占内存，所以额外用小表存储各分支在大表中的位置，先从小表中找到分支入口地址的偏移量，然后再在大表中找）

7. 如果各分支的常量毫不连续，与if...else...无异





### while语句反汇编

```
Label1:
    cmp ...
    jcc ...
    ...
    ...
    jmp Label1
```



### do...while语句反汇编

```
Label1:
    ...
    ...
    cmp ...
    jcc Label1
```



### for循环语句反汇编

```
例如
for(int i=0;i<5;i++){

}
对应反汇编代码如下：
	mov dword ptr [ebp-4],0 //int i=0
	jmp Label1 // 跳转到比较语句i<5
Label3:
	mov eax, dword ptr [ebp-4]//取出i
	add eax, 1//i++
	mov dword ptr [ebp-4],eax//i存入内存
Label1:
	cmp dowrd ptr [ebp-4],5//i<5
	jge Label2//i>=5循环结束
	...//循环所做的事情
	jmp Label3//循环
Label2:
	...

```

### 指针

地址

宽度：4字节(指针数组宽度也是4)（32位机，地址索引宽度）

- ++、-- 

  值为 去掉一个星号的类型宽度（如果是结构体的一级指针，则是结构体大小；如果是指针数组，则是类型数组的大小）

- 加减整数

  去掉一个星号的类型宽度*加减的值

- 同类型相加减

  数值相加减，然后再除以宽度（去掉一个星号的类型宽度）,得到的是数字

- 同类型相比较

  实质是同类型的相加减

==数组指针==

例如

```c
int main(){
	
	char ini[5]={'a','b','c','d','\0'};
	printf("%s\n",ini);
	char* s= ini;
	for(int j=0;j<5;j++){
		printf("%c",*(s+j));
	}
	printf("\n");
	for(int i=0;i<4;i++){
		*(s+i) = *(s+i)+i;
	}
	printf("%s\n",s);
	return 0;
} 
```

==指针数组==

int* a[5]和int (*a)[5]是不一样的定义：

- `int* a[5]`（==指针数组==）表示a是一个数组，包含5个元素，每个元素都是 `int*`类型，指向一个int（或int数组）。
- `int (*a)[5]`（==数组指针==）表示a是一个指针，指向一个int类型的数组，这个数组有5个元素。

```c
//指针数组
int* a[5];
printf("%d\n",sizeof(a));//20，因为是数组
a = (int*)10;
a++;
printf("%d\n",sizeof(a));//+5*4


//数组指针
int (*a)[5];
printf("%d\n",sizeof(a));//4,因为是指针,指向一个大小为5的数组
a = (int(*)[5])10;//强制转换使用(int (*)[5])
a++;
printf("%d\n",a);//砍掉一个*之后是int[5]，所以+5*4(数组大小)
printf("%d\n",*a);//值与a相同！！！
printf("%d\n",*a+1);//+4，*a是数组的首地址，+1等于加数组类型大小
printf("%d\n",*(*a));//*(*a)才是数组的第一个值

//充分理解
int arr[10] = {1,2,3,4,5,6,7,8,9,10};
int (*px)[2];
px = (int(*)[2])arr;
printf("%d\n",*(*(px+1)+1));
//1、px+1，实际上加的是8个字节(因为px去掉一个*是int[2])
//2、在(1)基础上取*加1，实际上是加4个字节(因为取*之后是int[2]，类型大小为4字节)
//3、综上，相对px偏移了12个字节，指向的是arr[3]
```

- 由上述可知，\*()和\[]是可以互换的，也就是说
  int (\*px)[2]等价于二维数组int px\[][2]?
  但是\*(p+1)[2]不一定等于p\[1][2],前者会多加一个类型长度

  例如：

  ```c
  int main(){
      int arr[][2] = {1,2,3,4,5,6,7,8,9,10};
      int (*p)[2] = (int(*)[2])arr;
      prinf("%d\n",arr[1][1]);
      prinf("%d\n",*(p+1)[1]);
      prinf("%d\n",*(*(p+1)+1));
      return 0;
  }
  ```

  发现输出的结果为

  ```
  4
  5
  4
  ```

  显然输出结果有些意外。我尝试反汇编分析，分析三种不同取值的汇编代码

  - arr\[1]\[1]的汇编代码
    mov	ecx,dword ptr [ebp-Ch]

  - \*(p+1)\[1]的汇编代码

    mov edx,dword ptr [ebp-2Ch]

    mov eax,dword ptr [edx+10h]

  - \*(\*(p+1)+1)的汇编代码

    mov ecx,dword ptr [ebp-2Ch]

    mov edx,dword ptr [ecx+0Ch]

  \*(p+1)\[1]的汇编代码中，偏移量为什么是10h呢？



==字符串与字符数组==

char* 与 char []

字符串常量是存储在程序的静态存储区（也叫做只读数据区、常量区）中的。

char* s所定义的s存放的是字符串常量的首地址，因此不可通过s修改字符串。

但 char s[]所定义的，则是将字符串常量复制一份到堆栈中，因此可以通过s[i]修改字符串。



### 内存搜索

#### 数字搜索

例子：

![](pictures/精确数值搜索.png)

```c
char s[] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x07,0x09,
	0x00,0x20,0x10,0x03,0x03,0x0C,0x00,0x00,0x44,0x00,
	0x00,0x33,0x00,0x47,0x0C,0x0E,0x00,0x0D,0x00,0x11,
	0x00,0x00,0x00,0x02,0x64,0x00,0x00,0x00,0xAA,0x00,
	0x00,0x00,0x64,0x10,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x02,0x00,0x74,0x0F,0x41,0x00,0x00,0x00,
	0x01,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x0A,0x00,
	0x00,0x02,0x74,0x0F,0x41,0x00,0x06,0x08,0x00,0x00,
	0x00,0x00,0x00,0x64,0x00,0x0F,0x00,0x00,0x0D,0x00,
	0x00,0x00,0x23,0x00,0x00,0x64,0x00,0x00,0x64,0x00,	
};
```



##### 1byte数字搜索

```c
void Search_ByteNum(int num){
	char* p = s;
	int len = sizeof(s);
	for(int i=0;i<len;i++){
		if(*(p+i) == num){
			printf("%x\n",p+i);
		}
	}
}
```



##### 2bytes数字

```c
void Search_WordNum(int num){
	short* p;
	int len = sizeof(s) - sizeof(short) + 1;
	for(int i=0;i<len;i++){
		p = (short*)(s+i);
		if(*p == num){
			printf("%x, %d, %d\n", p, *p, i);
		}	
	}
}
```



##### 4bytes搜索

```c
void Search_DWordNum(int num){
	int* p;
	int len = sizeof(s) - sizeof(int) + 1;
	for(int i=0;i<len;i++){
		p = (int*)(s+i);
		if(*p == num){
			printf("%x, %d, %d\n", p, *p, i);
		}
	}
}
```



#### 字符串搜索

搜索“WOW”

```c
char s[] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x07,0x09,
	0x00,0x20,0x10,0x03,0x03,0x0C,0x00,0x00,0x44,0x00,
	0x00,0x33,0x00,0x47,0x0C,0x0E,0x00,0x0D,0x00,0x11,
	0x00,0x00,0x00,0x02,0x64,0x00,0x00,0x00,0xAA,0x00,
	0x00,0x00,0x64,0x10,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x02,0x00,0x74,0x0F,0x41,0x00,0x00,0x00,
	0x01,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x0A,0x00,
	0x00,0x02,0x57,0x4F,0x57,0x00,0x06,0x08,0x00,0x00,
	0x00,0x00,0x00,0x64,0x00,0x0F,0x00,0x00,0x0D,0x00,
	0x00,0x00,0x23,0x00,0x00,0x64,0x00,0x00,0x64,0x00,
};

#define N 1010
void Search_String(char* src){
	
	char dest[N]={0};
	int srclen = strlen(src);//待寻找的字符串的长度
	int len = sizeof(s) - srclen + 1;
	
	for(int i=0;i<len;i++){
		//从内存中取相应长度的字符串
		strncpy(dest,s+i,stlen);//从s+i到s+i+stlen,正向走 
		if(strcmp(dest,src) == 0){
			printf("%x, %s, %d\n", dest, dest, i);
		}
	}
	
}
```



#### 浮点数搜索



#### 结构体搜索

查找id=1,level=8的结构体信息。

```c
//结构体搜索
char s[] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x07,0x09,
	0x00,0x20,0x10,0x03,0x03,0x0C,0x00,0x00,0x44,0x00,
	0x00,0x33,0x01,0x00,0x00,0x08,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x02,0x64,0x00,0x00,0x00,0xAA,0x00,
	0x00,0x00,0x64,0x01,0x00,0x00,0x00,0x08,0x00,0x00, 
	0x00,0x00,0x02,0x00,0x74,0x0F,0x41,0x00,0x00,0x00,
	0x01,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x0A,0x00,
	0x00,0x02,0x57,0x4f,0x57,0x00,0x06,0x08,0x00,0x00,
	0x00,0x00,0x00,0x64,0x00,0x0F,0x00,0x00,0x0D,0x00,
	0x00,0x00,0x23,0x00,0x00,0x64,0x00,0x00,0x64,0x00
};
```



```c
typedef struct  TagPlayer{
	int id;
	int level;
}Player;

void Search_Struct(Player* player){
	int *s_id, *s_level;
	int len = sizeof(s) - sizeof(Player) + 1;
//	printf("%d %d %d",sizeof(s),sizeof(Player),len);
	for(int i=0;i<len;i++){
		s_id = (int*)(s+i);
		s_level = s_id + 1;
		if(*s_id == player->id && *s_level == player->level){
			printf("%x %d %d\n",s_id,*s_id,*s_level);
		}
	}
}
```





### 隐式函数

可以用来==将代码隐藏到数据区。加载DLL文件==

```c
//显示函数
void Function(){}

//隐式函数
int px = function();//无参数的函数,返回值为int类型的值
void px = function(int,int);//带两个Int类型参数的函数,返回值为void类型
```



一些基本特性：

```c
int (*pFun)(int,int);
printf("%d\n",sizeof(pFun))//4,因为是指针类型
pFun = (int(*)(int,int))10;//强制转换 
//也可以写成(int(__cdecl *)(int,int)),因为默认的调用约定是__cdecl，所以可以省略

pFun++;//有的编程器上允许，有的不允许。去掉一个*，是函数，函数的大小是无法确定的
printf("%d\n",pFun);//11
```



代替函数

```c
int ADD(int x, int y) {
	return x+y;
}

int main(){
	int (*pFun)(int,int);
	pFun = ADD;//参数、返回类型一致，可以不用强制转换
	printf("%d\n",pFun(1,2));//执行ADD函数，输出3
}
```



例子：

将代码隐藏到数据区

```c
char arr[]={
	0x55,
    0x8B,0XEC,
    0X83,0XEC,0X40,
    0X53,
    0X56,
    0X57,
    0X8D,0X7D,0XC0,
    0XB9,0X10,0X00,0X00,0X00,
	0XB8,0XCC,0XCC,0XCC,0XCC,
    0XF3,0XAB,
    0X8B,0X45,0X08,
    0X03,0X45,0X0C,
    0X5F,
    0X5E,
    0X5B,
    0X8B,0XE5,
    0X5D,
    0XC3
};//ADD函数的硬编码(32位机)

int main(){
	int* px = function(int, int);
    px = (int(*)(int,int))&arr;
    printf("%d\n",px(1,2))//1+2=3
	return 0;
}
```



### PE

PE文件从磁盘存入内存中，并不能直接运行，还需要”拉伸“（PE Loader）。

所谓内核重载就是指不用window的内核，自己手动拉伸，于是可以不通过内核直接运行。



#### PE加载

1. 找到SizeOfImage，开辟相应大小的内存，并初始化0
2. 找到SizeOfHeaders，直接将头部复制过来
3. 循环复制节的内容，从PointerToRawData开始复制SizeOfRawData大小，复制到VirtualAddress开始的内存





>1. 需要注意的是，用什么编译器生成的就用对应位数的winhex分析。
>
>2. 经分析，发现手动PE加载之后的PE与实际运行的PE仍有不同的地方！
>
>   比如：扩展头部分不同（不知道是不是自己的原因造成的）
>
>   ​			.rdata节信息不同
>
>   ​			.data节信息不同
>
>   ​			.didat节信息不同
>
>   ​			但都是相同的二进制数变成了另一种相同的二进制数，怀疑是编码的问题？

![](pictures/手动PELoading对比.png)

<img src="pictures/系统PELoad对比.png" style="zoom:60%;" />



#### ==代码节空白区添加代码==

> 实际上任何空白区都可添加代码，但如果不是代码节，需要修改属性Characteristics
>
> 同样也可以新增节，添加大量的代码

修改程序入口地址，利用call指令跳到目标代码，执行完后利用jmp跳到原程序入口地址



**==插入代码的前提是，该块区的剩余空间足够目标代码的插入==**



**但call指令和jmp指令后面并不是直接跟随目标地址**，而是符合下述公式的值（令为x）：

​			真正要跳转的地址 = 跳转指令的下个指令的地址 + x

​			而 跳转指令的下个指令的地址 = call指令地址 + call指令长度(x32机为5)

​			故 x = 真正要跳转的地址 - (call指令地址 + call指令长度)

<span style="color:red">然而，实际上call指令和jmp指令的地址都是PE加载后的地址，PE文件中给出的是文件偏移地址，所以要进行换算：</span>

指令地址 = ImageBase + 所在节的VirtualAddress + (指令在文件中的偏移量 -  所在节的PointerToRawData)



针对于jmp指令，因为是要跳转到原程序入口地址AddressOfEntryPoint，该值已经经过处理（即在PE加载后可直接用），所以真正的原程序入口地址 = ImageBase+AddressOfEntryPoint

倘若jmp后面填写的是自己代码的指令，则跟上述公式一样。



程序入口地址OEP修改

​	(目标代码在文件中的偏移量 -  所在节的PointerToRawData) +  所在节的VirtualAddress



**例子：**

```
ImageBase: 0x400000
MessageBox Address: 0x75AA0C10
AddressOfEntryPoint: 0x14e0
PointerToRawData: 0x400
shellcode代码的起始偏移量: 0x1ad0
```

1. MessageBox的地址用如下代码可计算出：

   ```c
   #include <stdio.h>
   #include <windows.h>
    
   typedef void (*FuncPointer)(LPTSTR);  // 函数指针  
   
   //不同位的编译器运行的结果不一样，可能是 MessageBox 在不同位的计算机上的地址不一样
   int main()
   {   
       HINSTANCE LibHandle;
       FuncPointer GetAddr;
       // 加载成功后返回库模块的句柄
       LibHandle = LoadLibrary("user32"); 
       printf("user32 LibHandle = 0x%X\n", LibHandle);
       // 返回动态链接库(DLL)中的输出库函数地址
       GetAddr=(FuncPointer)GetProcAddress(LibHandle,"MessageBoxA");   
       printf("MessageBoxA = 0x%X\n", GetAddr);
       return 0;
   }
   ```

   0x75AA0C10

2. 得到所要插入的代码的硬编码

   ```
   6A 00
   6A 00
   6A 00
   6A 00
   E8 xx xx xx xx//xx代填，为地址
   E9 xx xx xx xx 
   ```

3. 计算call指令(E8)要跳转的地址

   ```
   文件中，E8下一行地址相对PointerToRawData偏移量：(0x1ad8 + 0x5) - 0x400 = 0x16dd
    
   映射到内存中，E8下一行地址:ImageBase + VirtualAddress + 0x16dd = 0x4026DD 
    
   E8后边的值：MessageBox - 0x4026DD = 0x7569E533
   ```

4. 计算jmp指令(E9)要跳转的地址

   ```
   原来OEP(真正要跳转的地址)：ImageBase + AddressOfEntryPoint = 0x4014E0
    
   文件中，E9下一行地址相对PointerToRawData偏移量：(0x1add + 0x5) - 0x400 = 0x16E2
    
   映射到内存中，E9下一行地址:ImageBase + VirtualAddress + 0x16E2 = 0x4026E2
    
   E9后边的值：0x4014E0 - 0x4026E2 = 0xFFFFEDFE
   ```

5. 修改AddressOfEntryPoint

   ```
   文件中shellcode的起始地址相对PointerToRawData偏移量：0x1ad0 - 0x400 = 0x16d0 
    
   映射到内存中，相对ImageBase偏移：VirtualAddress + 0x16d0 = 0x26D0
   
   将原来的OEP修改为映射到内存后的shellcode起始地址(0x26D0)。
   ```

   至此，完成简单的代码插入。

#### ==新增节，添加代码==

节表剩余空间是否大于80bytes，一个用于添加自己的节，一个是40个0的节

修改节的数量

修改sizeofimgae

修改节的属性

​	偏移 = 上一个节的偏移 + max{SizeofRowData,VirtualSize}并对齐

==最后一个节扩展，添加代码==

​	直接修改最后一个节的大小

> ​	以下是我实践中的思路和困惑：
>
> ​	对于要扩大的大小，首先要进行内存对齐，之后就是修改最后一个VirtualSize和SizeOfRawData，将它们都修改成扩展后内存对齐的大小，这样省事，然后修改SizeOfImage。
>
> ​	至于手动修改的时候，因为最后一个节后面是还有其他空间的（假令为空间S），如果这个空间足够的话，按理说是不需要在手动在文件末尾填充零，实际上也是这样的。所以这段空间的数据到底有没有用呢？
>
> ​	但用代码修改的时候，因为我进行PE加载之后再去加载，导致后面那一段空间(空间S)并没有在里面，这样的话，我们就需要开辟更大的空间，至于要不要再追加空间S，我想应该是不用的，因为他没什么用吧？
>
> ​	然而在实际操作中，按照流程进行，但生成的文件不能运行！！？我导致我怀疑后面那段空间(空间S)是有用的，但又与之前的实操结果又不符合。所以应该是我的代码有问题！！！
>
> ​	在回到VirtualSize和SizeOfRawData这两个属性的修改，我认为SizeOfRawData不修改也行（在不添加代码或添加的代码长度不超过原SizeOfRawData的情况下，如果超过则需要修改该），VirtualSize则可以是任意符合条件的值（即该值内存对齐后的大小等于我们扩展后的大小）。
>
> ​	这样一梳理，发现明朗许多，不像之前太混乱了！！
>
> ​	理论存在，实践开始！！！
>
> ​	通过与手动扩大节的文件对比，并修改，终于找到问题了！！原因是最后存储成文件的时候，并没有存储扩展的部分，导致节表SizeOfRawData修改了但文件中节的大小还是原来的大小。通过修改SizeOfRawData成原来的大小，可以运行，这说明了SizeOfRawData在不添加代码或添加的代码长度不超过原SizeOfRawData的情况下，是可以不需要修改的！！！
>
> ​	所以那些从文件末尾追加空间的，好像并没什么用。因为节是连续的，而最后一个节并不在文件末尾，所以这样追加毫无意义！！！
>
> ​	当然，以上的前提是扩展节是扩展节在内存的大小，如果是扩展节在文件的大小，很显然是要修改SizeOfRawData，显然单独扩展节在内存的大小似乎没什么用，所以……
>
> ​	OK！！！成功了！！！



#### ==合并节==

​	

> ​	分析一波合并成一个节表的情况
>
> ​	显然要先进行PE加载，涉及到要修改第一个节表的属性为VirtualSize、SizeOfRawData、Characteristics。
>
> ​	显然VirtualSize==SizeOfRawData=所有节表在内存中拉申后的长度（假令为S）（也许针对SizeOfRawData还需要文件对齐），因为如果VirtualSize是所有节表的SizeOfRawData之和的话，肯定是不对的，这样计算出来的VirtualSize是小于S的，存储的时候存在信息丢失问题。而且，我们存储的时候，应该是直接存储所有节拉伸后的大小，因为合并节表会使得节表信息缺失，如果仍存储去加载之后的节，则生成的exe并不能运行（缺失节表，对应的节无法进行拉伸，造成寻址异常），所以应存储所有节拉伸后的大小，这样一来，因为第一个节表保存，所以拉伸第一个节表后，后续的节表就会对应正常的内存地址。综上，VirtualSize就必须为S
>
> ​	也就是要注意去加载过程中，节这部分不要去加载，直接存储为文件就行。



#### 静态链接 / 动态链接

lib文件	/	dll文件



dll文件可以有导出表和导入表

exe一般只有导入表



#### 导出表

​	IMAGE_DATA_DIRECTORY[0]

​	提供函数和函数地址供其他应用程序使用

> ​	关于导出表中三个小表的思考：
>
> ​	根据导出函数有自定义的序号，所以导出表就会有一个Base属性来记录导出序号的起始值，这样就可以将函数序号转成索引下标。
>
> ​	因为函数地址表存储的是所有函数的函数地址（包括无名函数），对于那些有名函数，就可以利用函数名地址表和函数序号表来查询（先不讨论如何查询）。但对于那些无名函数，是无法通过这两个表查询的，唯一能依靠的只有函数地址表，所以通过函数导出序号来查找函数地址就顺理成章的用函数导出序号 - 基序号Base作为下标来存取了。
>
> ​	接下来讨论那些有名函数了。既然是有名函数，肯定需要一个存储函数名或函数名地址的表，于是就有了函数名地址表。然而并不是所有的函数都有函数名（可以人为使其没有函数名），所以又需要有一个存储有函数名的函数在函数地址表的下标的表，于是就有了函数序号表。于是这两个表的个数是相同的，以简化的思想看待，是不是应该同一下标对应的空间应存储同一个函数的相关信息（因为没有其他表作为字典进行下标转换了），所以这两个表的下标是对应的。
>
> ​	显然，通过函数名就将这三个表联系在一起。那么就讨论一下怎么通过函数名找函数地址。从函数名地址表中找到对应函数名的下标，通过下标从函数序号表中找到序号，最后通过序号最为下标从函数地址表中找到函数地址。
>



#### 重定位表

​	IMAGE_DATA_DIRECTORY[5]

​	多个dll导入的时候，会存在地址冲突（基址相同）。	

​	模块间地址对齐大小为0x10000（64Kb）



#### IAT表

​	IMAGE_DATA_DIRECTORY[12]

​	exe内部调用自身函数，指令一般是call 固定地址，对应call硬编码是E8，意思类似near ptr

​	exe中调用dll函数，指令一般是call [地址]，对应地址存的是dll函数名，对应call硬编码是FF，意思类似far ptr

​	如果dll加载时的ImageBase发生变动，则需要IAT表**修正exe中的调用dll部分的指令**	



> 总结了一下IAT表和 重定向表的区别
> 重定向表：如果dll没站住原来的位置，就需要把dll里面的函数调用和跳转的地址等需要改正的通过重定向表修复指向真正的地址。
> IAT表：如果dll没站住原来的位置，程序调用DLL的函数的时候，程序就需要修正程序调用dll函数的地址。
> 总结：重定向表修dll的地址，IAT表修复程序的地址！





#### 导入表

​	IMAGE_DATA_DIRECTORY[1]

​	一个exe可以导入多个dll，故导入表有多个



#### 绑定导入表

​	绑定 导入表 就是提前将dll中的函数地址写入到IAT中

​	绑定导入 表IMAGE_DATA_DIRECTORY[11]			_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT

​	PE文件中的位置在节表后面

​	导入表的TimeDateStamp的值，0表示没有绑定，-1表示绑定了



#### 导入表注入

​	先将导入表移到新增节中，然后在节末尾新增一个导入表。

​	在移动过程中，INT表可以移动，但IAT表不能移动，因为call [地址(固定的)]，详见IAT表

​	



### C++ 

==对象 this指针==

每个对象都有一个特殊的指针：this指针，指向对象本身

在调用对象的方法时，对应汇编代码会多出一个参数压入栈，就是this指针，但无需对此进行堆栈平衡，因为是用寄存器ecx存入的

结构体内包含参数并不影响结构体的大小

空结构体的大小为1，这是因为如果为0的话，在内存上无法区分该结构体实例化出的实体



==构造函数==

对象初始化时使用



==析构函数==

对象存在动态分派空间时，用于清除动态空间

```
创建格式：
	~对象名(){}
```



==继承==

实际上就是将父类的东西复制给子类

汇编代码上，与子类中全部定义而不继承的情况相同



当子类对象的空间大小 > 父类...

子类转父类可以直接转

父类转子类不能，但可以强转



变量重名并不会替换，如

```
struct x{
	int a;
	int b;
}
struct y:x{
	int a;
	int c;
}
struct z:y{
	int d;
	int e;
}
```

z的大小仍为24bytes，想要对重名变量赋值需如下

```
z:x.a = 
z:y.a = 
```



==private==

即使是私有变量，汇编代码都一样，所以可以用指针访问

**private、public等限制的只是编译器**



==class==

在c++中，变量默认private

想要申明函数或变量权限，如下

```c++
class A{
	public:
		int x;
	
}
```

用class继承时，不会继承父类的变量权限，需要手动声明，如下

```c++
class A:public B{
	
}
```



**在子类初始化时会自动调用父类的<span style="color:red">无参</span>构造方法**





==虚函数==

> 直接调用 ： call(E8)  地址 
>
> 间接调用 ： call(FF)  [...] 

虚函数用virtual修饰，使用的时间接调用！

虚函数占4字节，无论多少个虚函数，对类的大小只增加4字节，多出来的4字节在内存中的首地址，存放的时**虚函数表的地址**，虚函数表里存放的时虚函数地址

1. 继承不重写虚函数时，子类仍然只有一个虚函数表（地址与父类虚表不同），存储虚函数的顺序为父->子

2. 继承重写虚函数时，子类的虚函数表中无父类的虚函数地址

3. 多继承不重写虚函数时，有多少个父类就有多少个虚函数表，子类的虚函数放在第一个虚表中

   ![](pictures/多重继承无重写.png)

4. 多继承重写虚函数时，哪个虚函数重写了就覆盖哪个

5. 多重继承，一个虚表，顺序一次为祖->父->子

==多态(动态绑定)==

多态：同一个对象，执行某个函数是表现不同的行为

绑定：调用的代码跟函数的地址关联到一起的过程

前期绑定 / 编译器绑定 ：普通的变量成员和函数，编译完地址是写死的

运行期绑定 / 动态绑定  / 晚绑定 ： 虚函数，编译完地址不是写死的

动态绑定又叫多态

c++中动态绑定通过虚表实现



==模板==

某段代码对所有类型的变量都可以适用

```c++
template <class int T_ELE>//class，int类型都可以用
void A(T_ELE ob){}
```

==引用类型==

解决指针被修改造成的错误。

引用和指针在汇编代码上无区别！但是编译器不允许引用再指向别的地方！

```c++
//int类型的引用 : 
void A(int& x){
	//直接赋值
	x=1;
}
//使用时 : 
int a=1;
A(x);

```



==友元==

函数访问类的私有成员时，是不能访问的， 但是将访问者函数在类中声明为友元，就可以访问了。

```c++
class A{
private:
	int a;
	int b;
public:
	friend void Print(A p); 

}
void Print(A p){}
```



==运算符重载==

用函数重新定义运算符的含义。

例如：

```c++
class A{
    private:
    	int a;
    	int b;
    public:
    	A operator++(){//对++运算符重定义，返回类型是A，如果是比较运算，则返回bool
            this->a++;
            this->b++;
        }
}
```



==new==

new的本质是malloc，创建的对象（变量）放在堆里

delete的本质是free

```c++
//new、malloc两者本质相同
int* p = new int;
int* p = (int*)malloc(sizeof(int));
//赋初值
int* p = new int(5);

//数组
int* p = new int[5];
delete[] p;
```

==vector==

本质是动态数组



### win32

#### 宽字符

多字节字符：char[]

中文由于使用两个字节表示，而char无法表示，故有wchar_t

```c++
wchar_t x = L'中';//L表示使用unicode编码
wprintf(L"%s\n",x);//使用wprintf输出宽字节字符
```

然而会输出乱码，需

```c++
#include <locale.h>
setlocale(LC_ALL,"");//地域，默认操作系统的编码
```

其他操作

```c++
//1、测长度
wcslen();//长度=字符个数
//而
strlen();//长度=字节个数
//2、复制
wcspy();
//3、等等……
```

然而为了能够使用不同类型时调用内核函数能正常执行，统一用宏定义TCHAR，根据项目编码使用不同类型。

```
TCHAR s[] = TEXT("中国");
```



#### API

==接口==

win32 API主要是存放在c:\windows\system32下的所有dll

几个比较重要的dll

```
Kernel32.dl1:最核心的功能模块，比如管理内存、进程和线程相关的函数等。
User32.d1:是indows用户界面相关应用程序接口，如创建窗口和发送消息等.
GDI32.dll:全称是Graphical Device Interface(图形设备接口)，包含用于画图和显示文本的函数
比如要显示一个程序窗口，就调用了其中的函数来画这个窗口
```

这些都是服务层（如javaweb中的server），本身不实现功能，只是调用==**内核函数**==实现

==WinMain的三个参数==

```c++
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR 1pCmdLine,
    int nCmdShow);

//hInstance : 应用运行时的基址(ImageBase)。是实例的句柄或模块的句柄。当可执行文件加载到内存中时，操作系统使用此值来标识可执行文件或 EXE。某些 Windows 函数需要实例句柄，例如加载图标或位图。
//hPrevInstance : 没有任何意义。它在 16 位 Windows 中使用，但现在始终为零。
//1pCmdLine : 以 Unicode 字符串的形式包含命令行参数。
//nCmdShow : 是一个标志，指示主应用程序窗口是最小化、最大化还是正常显示。
```



==消息==

事件所产生的信息

```
typedef struct tagMSG{
	HWND hwnd;//消息所在的窗口
	UINT message;//事件所产生的消息
	WPARAM wParam;//对消息的进一步描述
	LPARAM lParam;//对消息的进一步描述
	DWORD time;//消息产生的时间
	POINT pt;//鼠标点击位置
}MSSG, *PMSG;
```

![](pictures/windows消息处理.png)

窗口自创建开始，就会一直发送各种消息，

- **用户的事件** 包括某人可以与程序交互的所有方式：鼠标单击、键划、触摸屏手势等。
- **来自操作系统的事件** 包括程序“外部”的任何内容，这些事件可能会影响程序的行为方式。 例如，用户可以插入新的硬件设备，或者Windows可能进入低功率状态， (睡眠或休眠) 。

但很多消息并不是我们所关注的，这部分消息可以让windows处理。



==win32应用程序入口==



==ESP寻址的特点==

ebp是固定的，esp会随push、pop变，故esp寻址比较复杂

执行带参数call指令后，一般[esp]是返回地址，[esp+4]是第一个参数

==窗口回调函数的定位==

窗口类创建时会指定回调函数的地址，而且注册窗口类的时候会引用窗口类的地址，故此时可以跟踪到回调函数

==具体事件的处理的定位==



==子窗口==

- 子窗口如按钮、单选框等，是系统定义好的。

  那我们怎么获取其WNDCLASS？

  ```
  //获取窗口类名
  TCHAR　szBuffer[0x20];
  GetClassName(所查看的窗口的类名,szBuffer,0x20);
  
  //获取完整信息
  WNDCLASS wc;
  GetClassInfo(应用程序的句柄,szBuffer,&wc);
  ```

- 子窗口

  ![](pictures/按钮事件处理.png)

- 对子窗口定位

  在ESP寻址的情况下，WM_COMMAND（[ESP+8]）可以对所有子窗口定位，如果再加上WPLARM（[ESP+0xC]）（子窗口ID）可以对指定子窗口定位。

==资源文件==

通过资源文件创建窗口更方便。



====

dbug菜单栏中的W，可以看到各种窗口句柄

消息断点就是带条件的断点（条件断点）

根据下图可知

![](pictures/按钮事件处理.png)

**我们可以跟踪系统提供的WinProc，进而找到程序员写的消息处理函数。**

> 对于按钮，显然条件断点是leftbuttondown或leftbuttonup，至于是哪一种，直接尝试就知道了（左键点击不放，如果弹出窗口等，就是leftbuttondown了 ）

当找到系统提供的消息函数后，如何找到程序员写的消息函数呢？

dbug菜单栏中的M（Memory），对代码段（一般是.text）下**访问断点**，因为最终还是要调用程序员写的消息函数。

**但是并不是人为操作才会触发消息，而是窗口一旦存在，就会不断产生消息，那如何判断是否是人为操作触发的消息，导致调用程序员写的消息函数呢？**

因为对于每个行为产生的消息，都有对应的唯一ID，当调用父窗口的WinProc时，是需要传参的

```
hwnd//句柄
uMsg//消息类型
wParam
lParam
```

所以当调用到父窗口的消息处理函数后，查看栈，[esp+8]就是消息类型，判断是否与目标消息类型一致（比如按钮给父窗口提供的是WM_COMMAND (0x111)），如果是则进行跟踪。



==资源存放在exe的哪里？==

> 汉化、山寨等的需要

PE文件中的资源表（第3个）

资源类型分系统资源和用户自定义



![](pictures/PE资源表结构.png)

资源目录（上图中绿色部分）：

```c++
typedef struct _IMAGE_RESOURCE_DIRECTORY
{
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    WORD    NumberOfNamedEntries;//资源用字符串命名的数量
    WORD    NumberOfIdEntries;//资源用ID命名的数量
} IMAGE_RESOURCE_DIRECTORY, *PIMAGE_RESOURCE_DIRECTORY;
```

资源目录项（上图中黄色部分）：

```c++
typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY
{
    //该资源类型的属性的描述
    union {//占4个字节，即双字
        struct {
            DWORD NameOffset:31;//位域。代表第0~31位
            DWORD NameIsString:1;//位域。代表第32位
        } DUMMYSTRUCTNAME;
        DWORD   Name;
        WORD    Id;
    } DUMMYUNIONNAME;
    //该资源类型所拥有的资源的目录位置
    union {//占4个字节，即双字
        DWORD   OffsetToData;//目录项指针
        struct {
            DWORD   OffsetToDirectory:31;
            DWORD   DataIsDirectory:1;
        } DUMMYSTRUCTNAME2;
    } DUMMYUNIONNAME2;
} IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;
 
参数说明: 仅供参考, 详细说明请参考原文.
DUMMYUNIONNAME
    第一层: 资源类型标识, 第二层: 资源标识, 第三层: 国家地区语言标识
    DUMMYUNIONNAME.DUMMYSTRUCTNAME.NameOffset
        DUMMYUNIONNAME 的低31位值.
            当 NameIsString == 0 时表示资源目录类型. 也就是 DUMMYUNIONNAME.Id.
            当 NameIsString == 1 时表示资源目录名称的偏移. IMAGE_RESOURCE_DIR_STRING_U 结构. 相对于资源目录表(IMAGE_RESOURCE_DIRECTORY)的起始位置 + NameOffset 的偏移.
    DUMMYUNIONNAME.DUMMYSTRUCTNAME.NameIsString
        DUMMYUNIONNAME 的最高位值.用于判断 NameOffset 为名称还是资源类型.
    DUMMYUNIONNAME.Name
        DUMMYUNIONNAME 的值.
    DUMMYUNIONNAME.Id
        资源类型的ID.
 
DUMMYUNIONNAME2
    第一层: 资源偏移, 第二层: 资源数据偏移, 第三层: 资源数据的偏移
    DUMMYUNIONNAME2.OffsetToData
        DUMMYUNIONNAME2 的值
    DUMMYUNIONNAME2.DUMMYSTRUCTNAME2.OffsetToDirectory
        DUMMYUNIONNAME2 值的低31位值.
            当 DataIsDirectory == 0 时, 表示资源数据的偏移, IMAGE_RESOURCE_DATA_ENTRY 结构.
            当 DataIsDirectory == 1 时, 表示资源目录的偏移, IMAGE_RESOURCE_DIRECTORY 结构. 相对资源目录表(IMAGE_RESOURCE_DIRECTORY)的起始位置 + OffsetToDirectory偏移.
    DUMMYUNIONNAME2.DUMMYSTRUCTNAME2.DataIsDirectory
        DUMMYUNIONNAME2 值的最高位值
```

其中第一个联合体的Name属性在不同层代表不同的名字

![](pictures/PE资源表结构-例子.png)

当最高位（NameIsString）是1时，低31位是一个unicode指针，NameOffset低31位 + **第一层**的资源表（**绿色块**）的起始地址     才是结构体所在地址

```c++
typedef struct _IMAGE_RESOURCE_DIR_STRING_U
{
    WORD    Length;//字符串资源目录项名称的长度
    WCHAR   NameString[ 1 ];//资源目录项名称. 该字符串为Unicode字符串
} IMAGE_RESOURCE_DIR_STRING_U, *PIMAGE_RESOURCE_DIR_STRING_U;
```

当最高位（NameIsString）是0时，低31位是一个ID



其中第二个联合体是用于指向下一层目录

当最高位（DataIsDirectory）为1时，OffsetToData低31位 + **第一层**的资源表（**绿色块**）的起始地址     才是下一层目录地址

当最高位（DataIsDirectory）为0时，指向 _IMAGE_RESOURCE_DATA_ENTRY 结构体



第三层：代码页，下一层指向的结构体为（数据项）

```c++
typedef struct _IMAGE_RESOURCE_DATA_ENTRY
{
    DWORD   OffsetToData;
    DWORD   Size;
    DWORD   CodePage;
    DWORD   Reserved;
} IMAGE_RESOURCE_DATA_ENTRY, *PIMAGE_RESOURCE_DATA_ENTRY;
 
参数说明: 仅供参考, 详细说明请参考原文.
    OffsetToData
        资源数据的RVA
    Size
        资源数据大小
    CodePage
        代码页
    Reserved
        保留
```

