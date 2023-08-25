#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MaxSectionNum 96

typedef void (*FuncPointer)(LPTSTR); //函数指针  

IMAGE_DOS_HEADER* DOS = NULL;//DOS头 
IMAGE_NT_HEADERS32* NT32 = NULL;//32位NT头 
IMAGE_NT_HEADERS64* NT64 = NULL;//64位NT头 
IMAGE_SECTION_HEADER* SectionHeader[MaxSectionNum];//节表数组

char ShellCode[] = {
	0x6A,0x00,0x6A,0x00,0x6A,0x00,0x6A,0x00,
	0xE8,0x00,0x00,0x00,0x00,
	0xE9,0x00,0x00,0x00,0x00
};


FILE* OpenFile(char* FileName);//打开文件，返回文件指针 
int getFileSize(FILE* fp);//获取文件大小 
char* ReadFileIntoMemory(FILE* fp);//将文件读取到内存中，返回在内存中的首地址(指针)
void WriteIntoFile(char* FileBuffer,char* FileName);//将内存数据写入文件 ,禁止PE加载后写入文件 
void PEAnalyze(char* FileBuffer);//PE文件结构分析 
char* PELoading(char* FileBuffer);//PE加载 
char* PEUnLoad(char* ImageBuffer);//PE去加载(恢复成文件) 
void InsertCodeAfterLoading(char* ImageBuffer);//PE加载后插入代码
int GetMessageBoxAddress();//获取MessageBox函数的地址
char* ExpendSection(char* ImageBuffer);//扩大节（最后一个节） 
char* AddSection(char* ImageBuffer);//新增节
void CombineSection(char* ImageBuffer);//合并节(PE加载后才行，因为程序运行寻找其他节表的数据是靠拉升后的地址，而不是文件地址)



char* AddSection(char* ImageBuffer){
	
	int SectionNum = 0;
	int SectionAlign = 0; 
	int OptionSize = 0;
	int ImageSize = 0;
	int HeaderSize = 0;
	//获取DOS头和PE标准头	
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)ImageBuffer;
	
	if(NT32 != NULL){
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(ImageBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		SectionAlign = NT->OptionalHeader.SectionAlignment;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		ImageSize = NT->OptionalHeader.SizeOfImage;
		HeaderSize = NT->OptionalHeader.SizeOfHeaders;
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(ImageBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		SectionAlign = NT->OptionalHeader.SectionAlignment;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		ImageSize = NT->OptionalHeader.SizeOfImage;
		HeaderSize = NT->OptionalHeader.SizeOfHeaders;
	}
	
	//先分配ImageSize大小的空间，后续在扩大
	char* NewBuffer = (char*)malloc(sizeof(char)*ImageSize);
	memset(NewBuffer,0,ImageSize);
	//全复制过来
	memcpy(NewBuffer,ImageBuffer,ImageSize);
	//判断剩余空间是否能够添加节表
	int RemainSpace = HeaderSize - (NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*SectionNum);
	int New_e_lfanew = 0;//修改后的e_lfanew 
	New_e_lfanew = NewDOS->e_lfanew;//如果剩余空间足够，就不用改，否则进入if语句进行修改 
	if(RemainSpace < 40){//尽可能80，要有一个全零的节表作为结束识别 
		if(RemainSpace + NewDOS->e_lfanew - 64 < 40){
			printf("空间不足，无法添加节表！\n");
			free(NewBuffer);
			return NULL;
		}
		IMAGE_DOS_HEADER* DOSTemp1 = (IMAGE_DOS_HEADER*)NewBuffer;
		//修改e_lfanew
		DOSTemp1->e_lfanew = 64;
		New_e_lfanew = DOSTemp1->e_lfanew;
		//置零 
		memset(NewBuffer + DOSTemp1->e_lfanew, 0, HeaderSize - NewDOS->e_lfanew);
		//移动NT和节表，将DOS Stub覆盖掉
		memcpy(NewBuffer + DOSTemp1->e_lfanew, ImageBuffer + NewDOS->e_lfanew, HeaderSize - NewDOS->e_lfanew); 
	}
	
	//在内存中找到第一个节表地址
	IMAGE_SECTION_HEADER* FirstSection = (IMAGE_SECTION_HEADER*)(NewBuffer + New_e_lfanew + 4 + 20 +OptionSize);
	//最后一个节表的终止地址，也就是新增节表的起始地址 
	IMAGE_SECTION_HEADER* AddSection = FirstSection + SectionNum;
	//默认复制第一个节的信息给新的节表 
	memcpy(AddSection,FirstSection,40);
	//修改新增节的属性
//	AddSection->Name = ;
	AddSection->PointerToRawData = (AddSection-1)->PointerToRawData + (AddSection-1)->SizeOfRawData;
	AddSection->VirtualAddress = ImageSize;
	AddSection->Misc.VirtualSize = 0; 
//	AddSection->Characteristics = ;

	//重新分配空间 
	int ReallocNewSize = ImageSize + ((FirstSection+1)->VirtualAddress - FirstSection->VirtualAddress);
//	char* ReallocNewBuffer = (char*)realloc(NewBuffer,ReallocNewSize);
	char* ReallocNewBuffer = (char*)malloc(sizeof(char)*ReallocNewSize);
	memset(ReallocNewBuffer, 0, ReallocNewSize);
	memcpy(ReallocNewBuffer, NewBuffer, ImageSize);
	free(NewBuffer);
	
	//获取NT头 
	if(NT32 != NULL){
		//因为重分配的缘故，原先空间被free，不能直接使用结构体指向e_lfanew
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(ReallocNewBuffer + New_e_lfanew);
		//修改节的数量
		NT->FileHeader.NumberOfSections += 1;
		printf("修改内存的数量成功！\n"); 
		//修改内存大小SizeOfImage 
		NT->OptionalHeader.SizeOfImage = ReallocNewSize;
		printf("修改内存大小SizeOfImage成功！\n"); 
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(ReallocNewBuffer + New_e_lfanew);
		//修改节的数量
		NT->FileHeader.NumberOfSections += 1;
		//修改内存大小SizeOfImage 
		NT->OptionalHeader.SizeOfImage = ReallocNewSize;
	}
	
	printf("新增节表成功！\n");
	
	return ReallocNewBuffer; 
	
}

char* ExpendSection(char* ImageBuffer,int ExpenSize){
	
	int SectionNum = 0;
	int SectionAlign = 0; 
	int OptionSize = 0;
	int ImageSize = 0;
	
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)ImageBuffer;
	
	if(NT32 != NULL){
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(ImageBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		SectionAlign = NT->OptionalHeader.SectionAlignment;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		ImageSize = NT->OptionalHeader.SizeOfImage;
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(ImageBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		SectionAlign = NT->OptionalHeader.SectionAlignment;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		ImageSize = NT->OptionalHeader.SizeOfImage;
	}
	
	//计算对齐后扩大的大小 
	ExpenSize = ((ExpenSize - 1)/SectionAlign + 1)*SectionAlign;
	//新的内存大小 
	int NewImageSize = ImageSize + ExpenSize;
	//分配新的空间
	char* ExpendImageBuffer = (char*)malloc(sizeof(char)*NewImageSize);
	memset(ExpendImageBuffer,0,ImageSize);
	memcpy(ExpendImageBuffer,ImageBuffer,ImageSize);
	//扩大最后一个节 
	IMAGE_SECTION_HEADER* LastSection = (IMAGE_SECTION_HEADER*)(ExpendImageBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*(SectionNum - 1));
	//计算最后一个节的内存大小 
	int LastSize = ImageSize - LastSection->VirtualAddress;
	//修改属性 
	LastSection->SizeOfRawData = LastSize + ExpenSize;
	LastSection->Misc.VirtualSize = LastSize + ExpenSize;
	//修改ImageSize
	if(NT32 != NULL){
		IMAGE_OPTIONAL_HEADER32* OpHeader = (IMAGE_OPTIONAL_HEADER32*)(ExpendImageBuffer + NewDOS->e_lfanew + 4 + 20);
		OpHeader->SizeOfImage = NewImageSize;
	}
	else{
		IMAGE_OPTIONAL_HEADER64* OpHeader = (IMAGE_OPTIONAL_HEADER64*)(ExpendImageBuffer + NewDOS->e_lfanew + 4 + 20);
		OpHeader->SizeOfImage = NewImageSize;
	}
	
	return ExpendImageBuffer;
	
}

void CombineSection(char* ImageBuffer){
	
	//获取 NumberOfSections,FileAlignment
	int SectionNum = 0;
	int FileAlign = 0;
	int OptionSize = 0;
	
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)ImageBuffer;
	
	if(NT32 != NULL){//x32
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(ImageBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		FileAlign = NT->OptionalHeader.FileAlignment;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader; 
	} 
	else{//x64
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(ImageBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		FileAlign = NT->OptionalHeader.FileAlignment;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
	} 
	
	//在内存中找到第一个节表地址
	IMAGE_SECTION_HEADER* FirstSection = (IMAGE_SECTION_HEADER*)(ImageBuffer + NewDOS->e_lfanew + 4 + 20 +OptionSize);
	//最后一个节表的起始地址
	IMAGE_SECTION_HEADER* LastSection = FirstSection + SectionNum - 1;
	
	//合并节表属性，计算合并后的内存大小
	int AllSectionSize = 0;
	int AllSectionCharacteristics = 0;
	int AllSizeOfRawData = 0;
	
	//合并后节表的属性 
	for(int i=0;i<SectionNum;i++) {
		AllSectionCharacteristics |= (FirstSection + i)->Characteristics;
	}
	
	//合并后节表的内存对齐大小
	int LastSectionSizeOfRawData = LastSection->SizeOfRawData;
	int LastSectionVirtualSize = LastSection->Misc.VirtualSize;
	int MAX = LastSectionSizeOfRawData > LastSectionVirtualSize ? LastSectionSizeOfRawData : LastSectionVirtualSize;
	
	AllSectionSize = LastSection->VirtualAddress + MAX - FirstSection->VirtualAddress;
	AllSizeOfRawData = ((AllSectionSize-1)/FileAlign+1)*FileAlign;
	
	//修改节表的 VirtualSize,SizeOfRawData,Characteristics
	FirstSection->Misc.VirtualSize = AllSectionSize; 
	FirstSection->SizeOfRawData = AllSizeOfRawData;
	FirstSection->Characteristics = AllSectionCharacteristics;
	printf("合并节后，VirtualSize=%X\n",AllSectionSize);
	printf("合并节后，SizeOfRawData=%X\n",AllSizeOfRawData);
	
	//其余节表清零
	for(int i=1;i<SectionNum;i++){
		memset(FirstSection + i,0,sizeof(_IMAGE_SECTION_HEADER));
	}
	
	//节表个数置为1
	IMAGE_FILE_HEADER* FileHeader = NULL;
	FileHeader = (IMAGE_FILE_HEADER*)(ImageBuffer + NewDOS->e_lfanew + 4);
	FileHeader->NumberOfSections = 1;
	
	return;

}

int GetMessageBoxAddress(){
	
	HINSTANCE LibHandle;
    FuncPointer GetAddr;
    // 加载成功后返回库模块的句柄
    LibHandle = LoadLibrary("user32"); 
//    printf("user32 LibHandle = 0x%X\n", LibHandle);
    // 返回动态链接库(DLL)中的输出库函数地址
    GetAddr = (FuncPointer)GetProcAddress(LibHandle,"MessageBoxA");   
//    printf("MessageBoxA = 0x%X\n", GetAddr);
//    printf("(int)MessageBoxA = 0x%X\n", (int)(GetAddr));
    return (int)GetAddr;
    
}

void InsertCodeAfterLoading(char* ImageBuffer,int pos){
	
	//在对节表进行操作后，以下结构体应使用ImageBuffer中的结构体 
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)ImageBuffer;
	
	//获取ImageBase、OEP、OPtionSize 
	int ImageBase = 0;
	int OEP = 0; 
	int OptionSize = 0;
	if(NT32 != NULL){//x32
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(ImageBuffer + NewDOS->e_lfanew);
		ImageBase = NT->OptionalHeader.ImageBase;
		OEP = NT->OptionalHeader.AddressOfEntryPoint;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
	} 
	else{//x64
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(ImageBuffer + NewDOS->e_lfanew);
		ImageBase = NT->OptionalHeader.ImageBase;
		OEP = NT->OptionalHeader.AddressOfEntryPoint;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		//修改OEP,不可以，因为NT指针指向的是FileBuffer，并不能通过这个修改ImageBuffer里的值 
//		NT64->OptionalHeader.AddressOfEntryPoint = ShellCodeOffset; 
	}
	
	//第一个节表的偏移地址 
	int SectionOffset = 0;
	SectionOffset = NewDOS->e_lfanew + 4 + 20 + OptionSize;
	//目标节的起始地址 
	IMAGE_SECTION_HEADER* Section_pos = (IMAGE_SECTION_HEADER*)(ImageBuffer + SectionOffset + 40*pos);
	
	//判断第position个节的剩余空间是否能够插入代码
	int ResumeBytes = Section_pos->SizeOfRawData - Section_pos->Misc.VirtualSize;
	if(ResumeBytes < sizeof(ShellCode)){
		printf("该节的剩余空间不足！\n");
		return; 
	}
	
	//获取MessageBox的地址
	int MessageBoxAddr = GetMessageBoxAddress();
	printf("MessageBox地址=0x%x\n",MessageBoxAddr);

	//计算ShellCode在内存中的偏移
	int ShellCodeOffset = Section_pos->VirtualAddress + Section_pos->Misc.VirtualSize;
	printf("代码偏移量=0x%x\n",ShellCodeOffset);
	
	//计算call指令的跳转地址,修改ShellCode
	int CallAddr = MessageBoxAddr - (ImageBase + (ShellCodeOffset + 13));
	*(int*)(ShellCode+9) = CallAddr;
	printf("call指令的跳转地址为：%x\n",CallAddr);
	
	//计算jmp指令的跳转地址,修改ShellCode
	int JmpAddr =  OEP - (ShellCodeOffset + 18);
	*(int*)(ShellCode+14) = JmpAddr;
	printf("jmp指令的跳转地址为：%x\n",JmpAddr);
	
	//修改 AddressOfEntryPoint,因为在扩展PE头中，不好转成结构体 
	*(int*)(ImageBuffer + NewDOS->e_lfanew + 4 + 20 + 16) = ShellCodeOffset;//忘记转int了，结果只修改了第一位！！！ 
	
	//修改节的属性，使其具有可执行属性， 默认第一个节为代码节
	//这样也不可以，同样是修改了FileBuffer里的值 
//	SectionHeader[pos].Characteristics |= SectionHeader[0].Characteristics;
//	IMAGE_SECTION_HEADER* S1 = (IMAGE_SECTION_HEADER*)(ImageBuffer + SectionOffset + pos*40);
	IMAGE_SECTION_HEADER* SectionFirst = (IMAGE_SECTION_HEADER*)(ImageBuffer + SectionOffset); 
	Section_pos->Characteristics |= SectionFirst->Characteristics;

	//将ShellCode写入内存 
	memcpy(ImageBuffer+ShellCodeOffset,ShellCode,sizeof(ShellCode));
	
	return;
	
} 

char* PEUnLoad(char* ImageBuffer){
	//以下结构体应通过ImageBuffer从内存中找，而不是用先前PE分析得到的结构体 
	int NewSize = 0;
	int HeaderSize = 0;
	int SectionNum = 0;
	int OptionSize = 0;
	
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)ImageBuffer;
	
	if(NT32 != NULL){//x32
		IMAGE_NT_HEADERS32* NT = NULL;
		NT = (IMAGE_NT_HEADERS32*)(ImageBuffer + NewDOS->e_lfanew);//这个DOS->e_lfanew也要改，在添加节的时候可能会修改e_lfanew，直接使用结构体的就会出错 
		HeaderSize = NT->OptionalHeader.SizeOfHeaders;
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		printf("文件在内存中的大小0x%X\n",NT->OptionalHeader.SizeOfImage); 
	} 
	else{//x64
		IMAGE_NT_HEADERS64* NT = NULL;
		NT = (IMAGE_NT_HEADERS64*)(ImageBuffer + NewDOS->e_lfanew);
		HeaderSize = NT->OptionalHeader.SizeOfHeaders;
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		printf("文件在内存中的大小0x%X\n",NT->OptionalHeader.SizeOfImage);
	}
	printf("节的数量=%d\n",SectionNum);
	//在内存中找到第一个节表地址
	IMAGE_SECTION_HEADER* SectionStart = (IMAGE_SECTION_HEADER*)(ImageBuffer + NewDOS->e_lfanew + 4 + 20 +OptionSize);
	//根据节表起始地址找到最后一个节表 
	IMAGE_SECTION_HEADER* LastSection = SectionStart + SectionNum - 1;
	//获得文件大小，实际上是不太准确的，但是不影响运行 
	NewSize = LastSection->PointerToRawData + LastSection->SizeOfRawData;
	
	printf("最后一个节的对齐大小=%X\n",LastSection->SizeOfRawData);
	printf("最后一个节在文件中的偏移量=%X\n",LastSection->PointerToRawData);
	printf("文件大小=%X\n",NewSize);
	//在内存中开辟相应大小的空间 
	char*  NewBuffer= (char*)malloc(sizeof(char)*NewSize);
	memset(NewBuffer,0,NewSize);
	//进行PE去加载 
	//HEADER直接照搬
	memcpy(NewBuffer,ImageBuffer,HeaderSize);
	//节表进行去拉伸
	//要修改！！！，Section应从内存中找 
	for(int i=0;i<SectionNum;i++){
		//当前节在文件中的对齐后的大小 
		int SizeOfRawData = (SectionStart + i)->SizeOfRawData;
		//当前节在内存中的偏移地址
		int VirtualAddress = (SectionStart + i)->VirtualAddress;
		//当前节在文件中的偏移地址
		int PointerToRawData = (SectionStart + i)->PointerToRawData;
		
		memcpy(NewBuffer+PointerToRawData,ImageBuffer+VirtualAddress,SizeOfRawData);
	}

	return NewBuffer;
	
}

char* PELoading(char* FileBuffer){
	
	int ImageSize = 0;
	int HeaderSize = 0;
	int SectionNum = 0;
	//获取文件在内存中的大小
	if(NT32 != NULL){//x32
		ImageSize = NT32->OptionalHeader.SizeOfImage;
		HeaderSize = NT32->OptionalHeader.SizeOfHeaders;
		SectionNum = NT32->FileHeader.NumberOfSections;
	} 
	else{//x64
		ImageSize = NT64->OptionalHeader.SizeOfImage;
		HeaderSize = NT64->OptionalHeader.SizeOfHeaders;
		SectionNum = NT64->FileHeader.NumberOfSections;
	}
	
	printf("ImageSize=%x\n",ImageSize);
	printf("HeaderSize=%x\n",HeaderSize);
	printf("SectionNum=%x\n",SectionNum);
	
	//在内存中开辟相应大小的空间 
	char*  ImageBuffer= (char*)malloc(sizeof(char)*ImageSize);
	printf("\n");
	if(ImageBuffer == NULL){
		printf("PE加载的内存分配失败！\n");
		return NULL;
	}
	
	memset(ImageBuffer,0,ImageSize);
	
	//进行PE加载
	//HEADER直接照搬
	memcpy(ImageBuffer,FileBuffer,HeaderSize);
	
	//节表进行拉伸
	for(int i=0;i<SectionNum;i++){
		//当前节在文件中的对齐后的大小 
		int SizeOfRawData = SectionHeader[i]->SizeOfRawData;
		//当前节在内存中的偏移地址
		int VirtualAddress = SectionHeader[i]->VirtualAddress;
		//当前节在文件中的偏移地址
		int PointerToRawData = SectionHeader[i]->PointerToRawData;
		//从 PointerToRawData开始，复制SizeOfRawData大小，到ImageBuffer的起始地址为VirtualAddress的地方 
		memcpy(ImageBuffer+VirtualAddress,FileBuffer+PointerToRawData,SizeOfRawData);
	}
	
	return ImageBuffer;
	
}

void PEAnalyze(char* FileBuffer){
	
	char* FM_Addr = FileBuffer;
	
	//DOS头
	DOS = (IMAGE_DOS_HEADER*)FM_Addr;
	//NT头 
	FM_Addr += DOS->e_lfanew;
	//判断是x32还是x64 
	int SectionNum = 0;
	WORD ProgramFlag = *(WORD*)(FM_Addr + 4 + 16);
	if(ProgramFlag == IMAGE_SIZEOF_NT_OPTIONAL32_HEADER){//x32
		NT32 = (IMAGE_NT_HEADERS32*)(FM_Addr);
		SectionNum = NT32->FileHeader.NumberOfSections;
		FM_Addr += 4 + 20 + NT32->FileHeader.SizeOfOptionalHeader;
	}
	else if(ProgramFlag == IMAGE_SIZEOF_NT_OPTIONAL64_HEADER){//x64
		NT64 = (IMAGE_NT_HEADERS64*)(FM_Addr);
		SectionNum = NT64->FileHeader.NumberOfSections;
		FM_Addr += 4 + 20 + NT64->FileHeader.SizeOfOptionalHeader;
	}
	else{
		printf("无法识别是多少位的程序！\n");
		return;
	}
	//节表分析
	for(int i=0;i<SectionNum;i++){
		SectionHeader[i] = (IMAGE_SECTION_HEADER*)FM_Addr;
		FM_Addr += 40;
	}
	
	return;
	
}

FILE* OpenFile(char* FileName){
	
	FILE* fp = NULL;
	fp = fopen(FileName,"rb");
	
	if(fp == NULL){
		printf("文件打开失败！\n");
		return NULL;	
	}
	
//	printf("文件打开成功！\n");
	return fp;
		
}

int getFileSize(FILE* fp){
	
	fseek(fp,0,SEEK_END);
	int FileSize =  ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
//	printf("文件大小为：%d\n",FileSize);
	
	return FileSize;
	
}

char* ReadFileIntoMemory(FILE* fp){
	
	int BufferSize = getFileSize(fp);
	char* FileBuffer = NULL;
	FileBuffer = (char*)malloc(sizeof(char)*BufferSize);
	memset(FileBuffer,0,sizeof(FileBuffer));
	if(FileBuffer == NULL){
		printf("内存分配失败，文件未读取到内存中！\n");
		return NULL;
	}
	
	fread(FileBuffer,BufferSize,1,fp);
//	printf("内存分配成功，首地址为：%x\n",FileBuffer);
	
	return FileBuffer;
	
}

void WriteIntoFile(char* FileBuffer,char* FileName){
	
	FILE* fp = NULL;
	fp = fopen(FileName,"wb");
	if(fp == NULL){
		printf("文件打开失败！\n");
		return;
	}
	
//	int FileSize =  sizeof(FileBuffer);//找到原因了， sizeof(FileBuffer)=4，这样就使得写入文件总是4个字节 
//	fwrite(FileBuffer,FileSize,1,fp);有时候太大了写不进文件
	
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)FileBuffer;
	int FileAlign = 0;//文件对齐大小 
	int FileSize = 0;//文件大小
	int OptionSize = 0;//可选PE头的大小 
	int SectionNum = 0;//节表数量 
	if(NT32 != NULL){
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(FileBuffer + NewDOS->e_lfanew);
		FileAlign = NT->OptionalHeader.FileAlignment;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		SectionNum = NT->FileHeader.NumberOfSections;
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(FileBuffer + NewDOS->e_lfanew);
		FileAlign = NT64->OptionalHeader.FileAlignment;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		SectionNum = NT->FileHeader.NumberOfSections;
	}
	
	//找到最后一个节 
	IMAGE_SECTION_HEADER* LastSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*(SectionNum - 1));
	//计算文件大小 
	FileSize = LastSection->PointerToRawData + LastSection->SizeOfRawData; 
	//每次读入文件对齐的大小，因为FileSize是通过最后一个节表的偏移量+对齐后的大小得出的，所以是整数倍 
	int cnt = FileSize/FileAlign;
	for(int i=0;i<cnt;i++) {
		fwrite(FileBuffer+FileAlign*i,FileAlign,1,fp);
	}

	fclose(fp);
	
	return; 
	
}
