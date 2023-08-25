//#include <windows.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include "OwnPE.h"

#define FilePath "ipmsg.exe"
//#define NewFilePath "TestDll++.dll"
#define MaxSectionNum 96


//IMAGE_DOS_HEADER* DOS = NULL;//DOS头
//IMAGE_NT_HEADERS32* NT32 = NULL;//32位NT头 
//IMAGE_NT_HEADERS64* NT64 = NULL;//64位NT头 
//IMAGE_SECTION_HEADER* SectionHeader[MaxSectionNum];//节表数组指针 


int GetFunctionAddrByName(char* FunctionName, char* FileBuffer);//通过导出函数名获取函数地址FOA 
int GetFunctionAddrByOrdinal(int FunctionOrdinal, char* FileBuffer);//通过导出函数序号获取函数地址FOA 

int RVA_To_FOA(int RVA,char* FileBuffer);//内存偏移转文件偏移 
int FOA_To_RVA(int FOA, char* FileBuffer);//文件偏移转内存偏移

IMAGE_DATA_DIRECTORY* GetDirectory(char* FileBuffer, int pos);//获取指定目录表 
IMAGE_EXPORT_DIRECTORY* GetExportDirctory(char* FileBuffer);//获得导出表(FOA)
_IMAGE_BASE_RELOCATION* GetRelocationDirctory(char* FileBuffer);//获得重定位表(FOA)
_IMAGE_BOUND_IMPORT_DESCRIPTOR* GetBoundImportDirctory(char* FileBuffer);//获取绑定导入表 
_IMAGE_IMPORT_DESCRIPTOR* GetImportDirctory(char* FileBuffer);//获取导入表 
_IMAGE_RESOURCE_DIRECTORY* GetResourceDirctory(char* FileBuffer);//获取资源表 

void PrintRelocationBlock(char* FileBuffer);//打印重定位块
void PrintImportDirctory(char* FileBuffer);//打印导入表 
void PrintBoundImportDirctory(char* FileBuffer);//打印绑定导入表 
void PrintResourceDirctory(char* FileBuffer);//打印资源表


//递归打印资源表 
void PrintResourceDirectoryByRecursive(_IMAGE_RESOURCE_DIRECTORY* ResourceTable, char* FileBuffer,int level,int ResourceTableFOA);


void MovExportDirctory(char* FileBuffer);//移动导出表到新的节中 
void MovRelocationDirctory(char* FileBuffer);//移动重定位表到新的节中
void FixRelocationBlock(char* FileBuffer,int NewImageBase);//根据给出的新基址，修正重定位块 
void MovImportDirctory(char* FileBuffer);//移动导入表 (未完成)
void InsertImportDirctory(char* FileBuffer);//dll注入，导入表注入 

//获取目录项 
IMAGE_DATA_DIRECTORY* GetDirectory(char* FileBuffer, int pos){
	
	//获取必要参数
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)FileBuffer;
	//第一个是导出表的目录表 
	IMAGE_DATA_DIRECTORY* DATA_DIRECTORY_pos;
	if(NT32 != NULL){
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(FileBuffer + NewDOS->e_lfanew);
		DATA_DIRECTORY_pos = &NT->OptionalHeader.DataDirectory[pos];
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(FileBuffer + NewDOS->e_lfanew);
		DATA_DIRECTORY_pos = &NT->OptionalHeader.DataDirectory[pos];
	}
	
	return DATA_DIRECTORY_pos;
	
}
//获取重定位表 
_IMAGE_BASE_RELOCATION* GetRelocationDirctory(char* FileBuffer){
	
	_IMAGE_BASE_RELOCATION* RelocationTable = NULL;
	
	IMAGE_DATA_DIRECTORY* DATA_DIRECTORY5 = GetDirectory(FileBuffer,5);
	
	//将虚拟偏移量转换成文件偏移量 
	int FOA = RVA_To_FOA(DATA_DIRECTORY5->VirtualAddress,FileBuffer);

	//找到重定位表地址 
	RelocationTable = (_IMAGE_BASE_RELOCATION*)(FileBuffer + FOA);
	
	return  RelocationTable;
}
//获取导出表 
IMAGE_EXPORT_DIRECTORY* GetExportDirctory(char* FileBuffer){
	
	IMAGE_EXPORT_DIRECTORY* ExportTable = NULL;
	
	IMAGE_DATA_DIRECTORY* DATA_DIRECTORY0 = GetDirectory(FileBuffer,0);
	
	//将虚拟偏移量转换成文件偏移量 
	int FOA = RVA_To_FOA(DATA_DIRECTORY0->VirtualAddress,FileBuffer);
	
	//找到导出表结构体 
	ExportTable = (IMAGE_EXPORT_DIRECTORY*)(FileBuffer + FOA);
	
	return  ExportTable;
	
}
//获取绑定导入表 
_IMAGE_BOUND_IMPORT_DESCRIPTOR* GetBoundImportDirctory(char* FileBuffer){
	
	_IMAGE_BOUND_IMPORT_DESCRIPTOR* BoundImportTable = NULL;
	
	IMAGE_DATA_DIRECTORY* DATA_DIRECTORY11 = GetDirectory(FileBuffer,11);
	
	int FOA = RVA_To_FOA(DATA_DIRECTORY11->VirtualAddress,FileBuffer);
	
	BoundImportTable = (_IMAGE_BOUND_IMPORT_DESCRIPTOR*)(FileBuffer + FOA);
	
	return BoundImportTable;
	
}
//获取导入表 
_IMAGE_IMPORT_DESCRIPTOR* GetImportDirctory(char* FileBuffer){
	
	_IMAGE_IMPORT_DESCRIPTOR* ImportTable = NULL;
	
	IMAGE_DATA_DIRECTORY* DATA_DIRECTORY1 = GetDirectory(FileBuffer,1);
	
	int FOA = RVA_To_FOA(DATA_DIRECTORY1->VirtualAddress,FileBuffer);
	
	ImportTable = (_IMAGE_IMPORT_DESCRIPTOR*)(FileBuffer + FOA);
	
	return ImportTable;
	
}
//获取资源表 
_IMAGE_RESOURCE_DIRECTORY* GetResourceDirctory(char* FileBuffer){
		
	_IMAGE_RESOURCE_DIRECTORY* ResourceTable = NULL;
	
	IMAGE_DATA_DIRECTORY* DATA_DIRECTORY2 = GetDirectory(FileBuffer,2);
	
	int FOA = RVA_To_FOA(DATA_DIRECTORY2->VirtualAddress,FileBuffer);
	
	ResourceTable = (_IMAGE_RESOURCE_DIRECTORY*)(FileBuffer + FOA);
	
	return ResourceTable;

}

int RVA_To_FOA(int RVA,char* FileBuffer){
	
	int FOA = -1;
	
	//获取必要参数
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)FileBuffer;
	int HeaderSize = 0;
	int SectionNum = 0;
	int OptionSize = 0;
	int SectionAlign = 0;
	if(NT32 != NULL){
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		HeaderSize = NT->OptionalHeader.SizeOfHeaders;
		SectionAlign = NT->OptionalHeader.SectionAlignment;
		
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		HeaderSize = NT->OptionalHeader.SizeOfHeaders;
		SectionAlign = NT->OptionalHeader.SectionAlignment;
	}
	
	//在内存中找到第一个节表地址
	IMAGE_SECTION_HEADER* FirstSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize);
	
	
	//判断RVA是否在文件头(DOS+NT+节表)中
	if(RVA <= HeaderSize){
		FOA = RVA;
	}
	else{
		//查找在哪个节中 
		for(int i=0;i<SectionNum;i++) {
			
			int start = (FirstSection+i)->VirtualAddress;
			int end = (FirstSection+i)->VirtualAddress + (FirstSection+i)->SizeOfRawData;
			//判断是否在当前节中 
			if(RVA >= start && RVA <= end){
				//计算相对当前节的偏移量 
				int Offset = RVA - (FirstSection+i)->VirtualAddress;
				//节在文件中的偏移量+相对节的偏移量 
				FOA = Offset + (FirstSection+i)->PointerToRawData;
			}
			
		}
		
	}
	
	if(FOA == -1){
		printf("RVA_TO_FOA转换失败！或许你的RVA对应的空间时PE加载时扩充的地方，在文件中找不到相应空间！\n");
	}
	
	return FOA; 
	
}

int FOA_To_RVA(int FOA, char* FileBuffer){
	
	int RVA = -1;
	
	//获取必要参数
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)FileBuffer;
	int HeaderSize = 0;
	int SectionNum = 0;
	int OptionSize = 0; 
	if(NT32 != NULL){
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		HeaderSize = NT->OptionalHeader.SizeOfHeaders;
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		HeaderSize = NT->OptionalHeader.SizeOfHeaders;
	}
	
	//在内存中找到第一个节表地址
	IMAGE_SECTION_HEADER* FirstSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize);
	
	
	//判断FOA是否在文件头(DOS+NT+节表)中
	if(FOA <= HeaderSize){
		RVA = FOA;
	}
	else{
		//查找在哪个节中 
		int pos = 0;
		for(int i=0;i<SectionNum;i++) {
			int start = (FirstSection+i)->PointerToRawData;
			int end = (FirstSection+i)->PointerToRawData + (FirstSection+i)->SizeOfRawData;
			//判断是否在当前节中 
			if(FOA >= start && FOA <= end){
				//计算相对当前节的偏移量 
				int Offset = FOA - (FirstSection+i)->PointerToRawData;
				//节在文件中的偏移量+相对节的偏移量 
				RVA = Offset + (FirstSection+i)->VirtualAddress;
			}
		}
	}
	
	if(RVA == -1){
		printf("FOA_TO_RVA转换失败！\n");
	}
	
	return RVA;
	
}
//在导出表中通过函数名获取函数地址 
int GetFunctionAddrByName(char* FunctionName, char* FileBuffer){
	
	int FunctionFOA = -1;
	
	//获取导出表
	IMAGE_EXPORT_DIRECTORY* ExportTable = GetExportDirctory(FileBuffer);
	//找到导出表的名字表,对比函数名找到函数序号 
	int NameAddress_FOA = RVA_To_FOA(ExportTable->AddressOfNames,FileBuffer);
	int pos = -1; //下标
	for(int i=0;i<ExportTable->NumberOfNames;i++){
		//函数名地址RVA
		int NameAddr = *(DWORD*)(FileBuffer + NameAddress_FOA+4*i);
		NameAddr = RVA_To_FOA(NameAddr,FileBuffer);
		//获取函数名
		char* fname = (char*)(FileBuffer + NameAddr);
		if(strcmp(fname,FunctionName) == 0){
			printf("Find it by name\n");
			pos = i; 
			break;
		}
	}
	
	//没有找到函数 
	if(pos == -1){
		printf("Cann't get function's address by name!\n");
		return FunctionFOA;
	}
	
	//通过下标获得函数序号
	short FunctionOrdinal = 0;
	int NameOrdinalAddress_FOA = RVA_To_FOA(ExportTable->AddressOfNameOrdinals,FileBuffer);
	FunctionOrdinal = *(short*)(FileBuffer + NameOrdinalAddress_FOA + 2*pos);
	printf("%s的函数序号为%d\n",FunctionName,FunctionOrdinal+ExportTable->Base);
	//函数序号作为下标从函数地址表中找到对应的函数地址
	int FunctionAddress_FOA = RVA_To_FOA(ExportTable->AddressOfFunctions,FileBuffer);
	int FunctionRVA = *(int*)(FileBuffer + FunctionAddress_FOA + 4*FunctionOrdinal);
	
	FunctionFOA = RVA_To_FOA(FunctionRVA,FileBuffer);
	
	return FunctionFOA; 
	
}
//在导出表中通过函数导出序号获取函数地址 
int GetFunctionAddrByOrdinal(int FunctionOrdinal, char* FileBuffer){
	
	int FunctionFOA = -1;
	//获取导出表
	IMAGE_EXPORT_DIRECTORY* ExportTable = GetExportDirctory(FileBuffer);
	
	//下标 = 导出序号-基序号
	int pos =  FunctionOrdinal - ExportTable->Base;
	
	if(pos < 0 || pos >= ExportTable->NumberOfFunctions){
		printf("函数导出序号无效！\n");
		return FunctionFOA;
	} 
	
	//根据下标从函数地址表中找到对应函数地址 
	int FunctionAddress_FOA = RVA_To_FOA(ExportTable->AddressOfFunctions,FileBuffer);
	int FunctionRVA = *(int*)(FileBuffer + FunctionAddress_FOA + 4*pos);
	
	FunctionFOA = RVA_To_FOA(FunctionRVA,FileBuffer);
	
	return FunctionFOA;
	
}
//移动导出表 
void MovExportDirctory(char* FileBuffer){
	
	//获取导出表 
	IMAGE_EXPORT_DIRECTORY* ExportTable = GetExportDirctory(FileBuffer);
	
	int SectionNum = 0;
	int OptionSize = 0;
	int FileAlign = 0; 
	
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)FileBuffer;
	
	if(NT32 != NULL){
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		FileAlign = NT->OptionalHeader.FileAlignment;
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		FileAlign = NT->OptionalHeader.FileAlignment;
	}
	
	//获得最后一个节表
	IMAGE_SECTION_HEADER* LastSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*(SectionNum - 1));
	
	//获取新增节的文件偏移地址,作为新的函数地址表的地址 
	int New_AddressOfFunctions_FOA = LastSection->PointerToRawData + LastSection->Misc.VirtualSize;

	//导出表不在头(SizeOfHeader之内)中
	//复制函数地址表
	int AddressOfFunctions_FOA = RVA_To_FOA(ExportTable->AddressOfFunctions, FileBuffer);
	memcpy(FileBuffer + New_AddressOfFunctions_FOA, FileBuffer + AddressOfFunctions_FOA, 4 * ExportTable->NumberOfFunctions);
	//新的函数导出序号表的文件偏移量 
	int New_AddressOfNameOrdinals_FOA = New_AddressOfFunctions_FOA + 4 * ExportTable->NumberOfFunctions;
	
	//复制函数导出序号表
	int AddressOfNameOrdinals_FOA = RVA_To_FOA(ExportTable->AddressOfNameOrdinals, FileBuffer);
	memcpy(FileBuffer + New_AddressOfNameOrdinals_FOA, FileBuffer + AddressOfNameOrdinals_FOA, 2*ExportTable->NumberOfNames);
	//新的函数名地址表的文件偏移量
	int New_AddressOfNames_FOA = New_AddressOfNameOrdinals_FOA + 2 * ExportTable->NumberOfNames;
	
	//复制函数名地址表
	int AddressOfNames_FOA = RVA_To_FOA(ExportTable->AddressOfNames, FileBuffer);
	memcpy(FileBuffer + New_AddressOfNames_FOA, FileBuffer + AddressOfNames_FOA, 4*ExportTable->NumberOfNames);
	//函数名地址表中第一个函数名所在的新的文件偏移量
	int New_EachAddressOfName_FOA = New_AddressOfNames_FOA + 4 * ExportTable->NumberOfNames;
	
	//根据函数名地址表复制函数名
	for(int i=0;i<ExportTable->NumberOfNames;i++){
		
		//获取函数名所在地址
		int NameAddr = *(DWORD*)(FileBuffer + AddressOfNames_FOA + 4*i);
		//转成FOA
		NameAddr = RVA_To_FOA(NameAddr,FileBuffer); 
		//获取函数名大小
		char* fname = (char*)(FileBuffer + NameAddr);
		int fnlen = strlen(fname) + 1; //要把'\0'包括进来 
		//复制函数名
		memcpy(FileBuffer + New_EachAddressOfName_FOA, fname, fnlen);
		//修正函数名地址表
		*(int*)(FileBuffer + New_AddressOfNames_FOA + 4*i)  = FOA_To_RVA(New_EachAddressOfName_FOA, FileBuffer);
		//下一个函数名所在的新地址
		New_EachAddressOfName_FOA += fnlen;
		
	}
	
	//复制导出表结构体
	//最后移动结构体，方便将其他表继续移动到后面空闲的地方 
	IMAGE_DATA_DIRECTORY* directory0 = GetDirectory(FileBuffer,0);
	int Old_ExportTable_FOA = RVA_To_FOA(directory0->VirtualAddress, FileBuffer);
	int New_AddressOfExportTable_FOA = New_EachAddressOfName_FOA;
	memcpy(FileBuffer + New_AddressOfExportTable_FOA, FileBuffer + Old_ExportTable_FOA, sizeof(IMAGE_EXPORT_DIRECTORY));
	
	//修正导出表结构体的属性
	IMAGE_EXPORT_DIRECTORY* NewExportTable = (IMAGE_EXPORT_DIRECTORY*)(FileBuffer + New_AddressOfExportTable_FOA);
	NewExportTable->AddressOfFunctions = FOA_To_RVA(New_AddressOfFunctions_FOA, FileBuffer);
	NewExportTable->AddressOfNameOrdinals = FOA_To_RVA(New_AddressOfNameOrdinals_FOA, FileBuffer);
	NewExportTable->AddressOfNames = FOA_To_RVA(New_AddressOfNames_FOA, FileBuffer);
	
	//修正导出表所在的目录表 
	directory0->VirtualAddress = FOA_To_RVA(New_AddressOfExportTable_FOA, FileBuffer);
	
	//修改对应节表的属性,方便后续添加其他数据时能定位
	//要使得能复用，需要在新增节表的时候修改这些属性
	//计算相对节的偏移量！！！ 
	int NextBlankArea = New_AddressOfExportTable_FOA + sizeof(IMAGE_EXPORT_DIRECTORY) - LastSection->PointerToRawData;
	//其实不用修改 SizeOfRawData，修改这个会使得节表变小，只要修改VirtualSize就行，根据它找到空白区 
//	LastSection->SizeOfRawData = ((NextBlankArea - 1)/FileAlign + 1)*FileAlign;
	LastSection->Misc.VirtualSize = NextBlankArea;
	
}
//移动重定位表 
void MovRelocationDirctory(char* FileBuffer){
	
	//获取重定位块
	_IMAGE_BASE_RELOCATION* RelocationBlock = GetRelocationDirctory(FileBuffer); 
	
	//获取必要参数
	int SectionNum = 0;
	int OptionSize = 0;
	int FileAlign = 0; 
	
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)FileBuffer;
	
	if(NT32 != NULL){
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		FileAlign = NT->OptionalHeader.FileAlignment;
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		FileAlign = NT->OptionalHeader.FileAlignment;
	} 
	
	//根据上述参数获取最后一个节表
	IMAGE_SECTION_HEADER* LastSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*(SectionNum - 1));
	
	//根据节表找到节，并定位空白区
	int New_AddressOfRelocation_FOA = LastSection->PointerToRawData + LastSection->Misc.VirtualSize;
	int New_Block_FOA = New_AddressOfRelocation_FOA;
	
	//获取重定位目录表
	IMAGE_DATA_DIRECTORY* Relocation = GetDirectory(FileBuffer, 5);  
	//循环复制重定位块
	int Old_Block_FOA = RVA_To_FOA(Relocation->VirtualAddress,FileBuffer); 
	while(1){
		//重定位表数组结束标志 
		if(RelocationBlock->VirtualAddress == 0 && RelocationBlock->SizeOfBlock == 0){
			break;
		}
		//直接复制 SizeOfBlock大小 
		memcpy(FileBuffer + New_Block_FOA, FileBuffer + Old_Block_FOA, RelocationBlock->SizeOfBlock);                         
		//下一个重定位块的文件偏移 
		RelocationBlock = (_IMAGE_BASE_RELOCATION*)((char*)RelocationBlock + RelocationBlock->SizeOfBlock); 
		New_Block_FOA += RelocationBlock->SizeOfBlock;
		Old_Block_FOA += RelocationBlock->SizeOfBlock;

	}
	
	//对后续8个字节赋0,作为结束标志
	memset(FileBuffer + New_Block_FOA, 0, 8);
	
	//修改重定位目录表的 VirtualAddress
	Relocation->VirtualAddress = FOA_To_RVA(New_AddressOfRelocation_FOA, FileBuffer);
	
	//修改相应节的属性，方便后续添加 
	int NextBlankArea = (New_Block_FOA + 8) - LastSection->PointerToRawData;
	LastSection->Misc.VirtualSize = NextBlankArea;
	
}

//移动导入表 (未完成)
void MovImportDirctory(char* FileBuffer){
	
	//获取导入表 
	_IMAGE_IMPORT_DESCRIPTOR* ImportTable = GetImportDirctory(FileBuffer);
	//获取导入目录表
	IMAGE_DATA_DIRECTORY* Import = GetDirectory(FileBuffer, 1);
	//获取旧的导入表初始FOA 
	int Old_Imoprt_FOA = RVA_To_FOA(Import->VirtualAddress, FileBuffer);
	
	//获取必要参数
	int SectionNum = 0;
	int OptionSize = 0;
	int FileAlign = 0; 
	
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)FileBuffer;
	
	if(NT32 != NULL){
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		FileAlign = NT->OptionalHeader.FileAlignment;
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		FileAlign = NT->OptionalHeader.FileAlignment;
	} 
	
	//根据上述参数获取最后一个节表
	IMAGE_SECTION_HEADER* LastSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*(SectionNum - 1));
	
	//根据节表找到节，并定位空白区，作为新的导入表初始FOA 
	int New_AddressOfImport_FOA = LastSection->PointerToRawData + LastSection->Misc.VirtualSize;
	int New_Import_FOA = New_AddressOfImport_FOA;
	
	//创建一个全零结构体
	_IMAGE_IMPORT_DESCRIPTOR* ZeroImportTable = (_IMAGE_IMPORT_DESCRIPTOR*)malloc(sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	memset(ZeroImportTable, 0, sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	
	//先移动所有导入表
	while(1){
		
		//判断结束标志,全零结构体作比较 
		if(memcmp(ImportTable, ZeroImportTable, sizeof(_IMAGE_IMPORT_DESCRIPTOR)) == 0){
			break;
		}
		
		//先只考虑x32机 
		//将导入表移动到新的节中 
		memcpy(FileBuffer + New_Import_FOA, FileBuffer + Old_Imoprt_FOA, sizeof(_IMAGE_IMPORT_DESCRIPTOR));
		
		//更新下一个导入表的初始FOA 
		New_Import_FOA += sizeof(_IMAGE_IMPORT_DESCRIPTOR); 
		Old_Imoprt_FOA += sizeof(_IMAGE_IMPORT_DESCRIPTOR);
		
		//下一个导入表的地址 
		ImportTable += 1;
		
	}
	
	//留出空白区供导入表注入 
	//要不要将INT表移动？？？以及dll名称和dll中函数名称？？？
	//未完！待续！！！ 
	
}

//修复重定位表 
void FixRelocationBlock(char* FileBuffer,int NewImageBase){
	
	//获取重定位块
	_IMAGE_BASE_RELOCATION* RelocationBlock = GetRelocationDirctory(FileBuffer);
	
	//获取必要参数
	int SectionNum = 0;
	int OptionSize = 0;
	int ImageBase = 0;
	
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)FileBuffer;
	
	if(NT32 != NULL){
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		ImageBase = NT->OptionalHeader.SizeOfImage;
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		ImageBase = NT->OptionalHeader.SizeOfImage;
	}
	
	//计算修改基址后的差值 
	int diff = NewImageBase - ImageBase;
	//遍历重定位块，根据重定位块给出的地址，判断是否要修改，如要修改，根据地址找到要修改的地址 
	while(1){
		//重定位表数组结束标志 
		if(RelocationBlock->VirtualAddress == 0 && RelocationBlock->SizeOfBlock == 0){
			break;
		}
		//获得块存储内容的地址 
		char* addr = (char*)RelocationBlock + 8;
		int num = (RelocationBlock->SizeOfBlock - 8)/2;
		for(int i=0;i<num;i++){
			WORD data= *(WORD*)(addr+2*i);
			WORD att = (data & 0xf000)>>12;
			//判断是否需要修改
			if(att == 3) {//要修改 
				DWORD RVA = RelocationBlock->VirtualAddress + (data&0x0fff);
				DWORD FOA = RVA_To_FOA(RVA,FileBuffer);
				//根据FOA找到要修改的内容
				DWORD* AlterAddr = (DWORD*)(FileBuffer + FOA);
				*AlterAddr += diff;
			}
		}
		//下一个重定位块的起始地址 
		RelocationBlock = (_IMAGE_BASE_RELOCATION*)((char*)RelocationBlock + RelocationBlock->SizeOfBlock); 

	}
	
	
}
//打印重定位表 
void PrintRelocationBlock(char* FileBuffer){
	
	_IMAGE_BASE_RELOCATION* RelocationArray = GetRelocationDirctory(FileBuffer);
	
	while(1){
		//重定位表数组结束标志 
		if(RelocationArray->VirtualAddress == 0 && RelocationArray->SizeOfBlock == 0){
			break;
		}
		//打印每块的内容
		printf("---------------------------\n");
		printf("VirtualAddress:0x%X\n",RelocationArray->VirtualAddress);
		printf("SizeOfBlock:%d\n",RelocationArray->SizeOfBlock);
		
		//获得块存储内容的地址 
		char* addr = (char*)RelocationArray + 8;
		int num = (RelocationArray->SizeOfBlock - 8)/2;
		for(int i=0;i<num;i++){
			
			WORD data= *(WORD*)(addr+2*i);
			DWORD RVA = RelocationArray->VirtualAddress + (data&0x0fff);
			WORD att = (data & 0xf000)>>12;
			printf("第%d项:\t地址(RVA)=0x%X\t属性=%d\n",i,RVA,att);
			
		}
		//下一个重定位块的起始地址 
		RelocationArray = (_IMAGE_BASE_RELOCATION*)((char*)RelocationArray + RelocationArray->SizeOfBlock); 
		printf("---------------------------\n");

	}
	
}
//打印导入表 
void PrintImportDirctory(char* FileBuffer){
	
	//找到导入表
	_IMAGE_IMPORT_DESCRIPTOR* ImportTable = GetImportDirctory(FileBuffer);
	
	//创建一个全零结构体
	_IMAGE_IMPORT_DESCRIPTOR* ZeroImportTable = (_IMAGE_IMPORT_DESCRIPTOR*)malloc(sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	memset(ZeroImportTable, 0, sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	
	//循环遍历所有导入表，打印相关信息 
	while(1){
		
		//判断结束标志,全零结构体作比较 
		if(memcmp(ImportTable, ZeroImportTable, sizeof(_IMAGE_IMPORT_DESCRIPTOR)) == 0){
			break;
		}
		
		//打印dll名字
		int Name_FOA = RVA_To_FOA(ImportTable->Name,FileBuffer);
		char* dname = (char*)(FileBuffer + Name_FOA);
		printf("%s--------------------\n",dname); 

		//先只考虑x32机 
		//将OriginalFirstThunk (RVA)转成FOA，根据FOA找到INT 
//		int OriginalFirstThunk_FOA = RVA_To_FOA(ImportTable->DUMMYUNIONNAME.OriginalFirstThunk,FileBuffer);
		int OriginalFirstThunk_FOA = RVA_To_FOA(*(DWORD*)ImportTable,FileBuffer);
		IMAGE_THUNK_DATA32* INT = (IMAGE_THUNK_DATA32*)(FileBuffer + OriginalFirstThunk_FOA);
		
		//将FirstThunk (RVA)转成FOA，根据FOA找到IAT
		int FirstThunk_FOA = RVA_To_FOA(ImportTable->FirstThunk,FileBuffer);
		//看看哪个简单，或者哪个明了 
//		IMAGE_THUNK_DATA32* IAT = (IMAGE_THUNK_DATA32*)(FileBuffer + FirstThunk_FOA);
		DWORD* IAT = (DWORD*)(FileBuffer + FirstThunk_FOA);
		//INT和IAT初始状态下是相同的


		//循环遍历所有INT,打印函数名 
		while(1){
			
			//判断结束标志
			if(*IAT == 0){
				break; 
			} 
			
			//根据最后位是否为1判断是根据名字(0)还是导出序号(1)查找函数名， 
			//名字(0,所有位的值为指向名字所在的地址)	导出序号(1,除最高位其余为的值为导出序号) 
			if(((*IAT) & 0x8000) > 0){//最高位为1 
				int Ordinal = (*IAT) & 0x7FFF;
				printf("按序号导入，函数导入序号:0x%x\n",Ordinal);
			}
			else{//最高位为0
				//又一个表 
				int ImportByNameAddr = (*IAT);
				int ImportByNameAddr_FOA = RVA_To_FOA(ImportByNameAddr,FileBuffer);
				char* ImportByName  = FileBuffer + ImportByNameAddr_FOA;
				printf("按名字导入, hint=0x%X, 函数名:%s\n",*(WORD*)ImportByName,ImportByName + 2);
			}
			
			//下一项 
			IAT += 1;
			
		} 
		
		//下一个导入表的地址 
		ImportTable += 1;
		
	}
	
}
//功能未经测试 ！！！ 
//打印绑定导入表 
void PrintBoundImportDirctory(char* FileBuffer){
	
	//获取绑定导入表
	_IMAGE_BOUND_IMPORT_DESCRIPTOR* BoundImportTable = GetBoundImportDirctory(FileBuffer);
	//记录第一个绑定导入表的地址，后续找dll名字需要使用 
	char* First_FOA = (char*)BoundImportTable;
	
	//创建一个空结构体 
	_IMAGE_BOUND_IMPORT_DESCRIPTOR* ZeroBoundImportTable = (_IMAGE_BOUND_IMPORT_DESCRIPTOR*)malloc(sizeof(_IMAGE_BOUND_IMPORT_DESCRIPTOR));
	memset(ZeroBoundImportTable, 0, sizeof(_IMAGE_BOUND_IMPORT_DESCRIPTOR));
	
	//循环遍历每个绑定导入表 
	while(1){
		
		//判断是否结束
		if(memcmp(BoundImportTable, ZeroBoundImportTable,sizeof(_IMAGE_BOUND_IMPORT_DESCRIPTOR)) == 0){
			break;
		}
		
		//输出
//		int DllName_FOA = 
//		int DllName_FOA = RVA_To_FOA(DllName_RVA, FileBuffer);
		char* DllName = First_FOA + BoundImportTable->OffsetModuleName;;
		printf("当前绑定导入表的关联的dll: %s\n",DllName);
		printf("TimeDateStamp:0x%X\n",BoundImportTable->TimeDateStamp);
		printf("NumberOfModuleForwarderRefs:0x%X\n",BoundImportTable->NumberOfModuleForwarderRefs); 
		
		//NumberOfModuleForwarderRefs 如果为不为零，则存在当前dll调用其他dll的，需要循环打印 
		//根据 NumberOfModuleForwarderRefs 输出 _IMAGE_BOUND_FORWARDER_REF
		int cnt = BoundImportTable->NumberOfModuleForwarderRefs;
		for(int i=0;i<cnt;i++) {
			 
			//转成结构体
			_IMAGE_BOUND_FORWARDER_REF* ref = (_IMAGE_BOUND_FORWARDER_REF*)((char*)BoundImportTable + sizeof(_IMAGE_BOUND_IMPORT_DESCRIPTOR) + i*sizeof(_IMAGE_BOUND_FORWARDER_REF));
			
			//获取被调用的dll的名字
			char* CalledDllName = First_FOA + ref->OffsetModuleName;
			printf("被调用的dll名字: %s\n",CalledDllName);
			printf("TimeDateStamp:0x%X\n",ref->TimeDateStamp);
			printf("Reserved:0x%X\n",ref->Reserved); 
			
		}
		//下一个绑定导入表
		//_IMAGE_BOUND_IMPORT_DESCRIPTOR 和 _IMAGE_BOUND_FORWARDER_REF 大小相同
		BoundImportTable = (_IMAGE_BOUND_IMPORT_DESCRIPTOR*)((char*)BoundImportTable + sizeof(_IMAGE_BOUND_IMPORT_DESCRIPTOR)*(cnt + 1)); 
		
	}
	
}
//dll注入，导入表注入 
void InsertImportDirctory(char* FileBuffer){
	
	//获取导入表 
	_IMAGE_IMPORT_DESCRIPTOR* ImportTable = GetImportDirctory(FileBuffer);
	//获取导入目录表
	IMAGE_DATA_DIRECTORY* Import = GetDirectory(FileBuffer, 1);
	//获取旧的导入表初始FOA 
	int Old_Imoprt_FOA = RVA_To_FOA(Import->VirtualAddress, FileBuffer);
	
	//获取必要参数
	int SectionNum = 0;
	int OptionSize = 0;
	int FileAlign = 0; 
	
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)FileBuffer;
	
	if(NT32 != NULL){
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		FileAlign = NT->OptionalHeader.FileAlignment;
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(FileBuffer + NewDOS->e_lfanew);
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		FileAlign = NT->OptionalHeader.FileAlignment;
	} 
	
	//根据上述参数获取最后一个节表
	IMAGE_SECTION_HEADER* LastSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*(SectionNum - 1));
	
	//根据节表找到节，并定位空白区，作为新的导入表初始FOA 
	int New_AddressOfImport_FOA = LastSection->PointerToRawData + LastSection->Misc.VirtualSize;
	int New_Import_FOA = New_AddressOfImport_FOA;
	
	//创建一个全零结构体
	_IMAGE_IMPORT_DESCRIPTOR* ZeroImportTable = (_IMAGE_IMPORT_DESCRIPTOR*)malloc(sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	memset(ZeroImportTable, 0, sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	
	//移动所有导入表
	while(1){
		
		//判断结束标志,全零结构体作比较 
		if(memcmp(ImportTable, ZeroImportTable, sizeof(_IMAGE_IMPORT_DESCRIPTOR)) == 0){
			break;
		}
		
		//先只考虑x32机 
		//将导入表移动到新的节中 
		memcpy(FileBuffer + New_Import_FOA, FileBuffer + Old_Imoprt_FOA, sizeof(_IMAGE_IMPORT_DESCRIPTOR));
		
		//更新下一个导入表的初始FOA 
		New_Import_FOA += sizeof(_IMAGE_IMPORT_DESCRIPTOR); 
		Old_Imoprt_FOA += sizeof(_IMAGE_IMPORT_DESCRIPTOR);
		
		//下一个导入表的地址 
		ImportTable += 1;
		
	}
	
	//末尾新增导入表
	_IMAGE_IMPORT_DESCRIPTOR* NewImportTable = (_IMAGE_IMPORT_DESCRIPTOR*)(FileBuffer + New_Import_FOA);
	New_Import_FOA += sizeof(_IMAGE_IMPORT_DESCRIPTOR);
	//添零作为结束标志 
	memset(FileBuffer + New_Import_FOA, 0, sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	
	//开辟新空间给INT和IAT,留空间用于新增其他导入表 
	//懒得留 
	//增加8字节INT表，一个双字使INT表生效，一个双字全零作为结束标志 
	int New_INT_FOA = New_Import_FOA + 0x20;
	IMAGE_THUNK_DATA32 * New_INT = (IMAGE_THUNK_DATA32*)(FileBuffer + New_INT_FOA);

	//同理增加8字节的IAT表 
	int New_IAT_FOA  = New_INT_FOA + 0x10;
	IMAGE_THUNK_DATA32* New_IAT = (IMAGE_THUNK_DATA32*)(FileBuffer + New_IAT_FOA);
	
	//开辟空间给dll名字 
	int New_DllName_FOA = New_IAT_FOA + 0x10;
	char* DllName = "InjectDll.dll";
	memcpy(FileBuffer + New_DllName_FOA, DllName, strlen(DllName) + 1);
	
	//开辟空间给dll函数名字 
	int New_DllFunName_FOA = New_DllName_FOA + 0x20;
	IMAGE_IMPORT_BY_NAME* New_DllFunc = (IMAGE_IMPORT_BY_NAME*)(FileBuffer + New_DllFunName_FOA);
	char* FunName = "ExportFunction";//待修改 
	memcpy(FileBuffer + New_DllFunName_FOA + 2, FunName, strlen(FunName) + 1);
	
	//修改INT和IAT表 ,将 New_DllFunc 的RVA赋值给INT和IAT
	New_INT->u1.AddressOfData = (DWORD)FOA_To_RVA(New_DllFunName_FOA,FileBuffer);
	New_IAT->u1.AddressOfData = (DWORD)FOA_To_RVA(New_DllFunName_FOA,FileBuffer);
	
	//修正新增导入表Name、OriginalFirstThunk、FirstThunk
	NewImportTable->FirstThunk = (DWORD)FOA_To_RVA(New_IAT_FOA,FileBuffer);
	NewImportTable->Name = (DWORD)FOA_To_RVA(New_DllName_FOA,FileBuffer); 
	*(DWORD*)NewImportTable = (DWORD)FOA_To_RVA(New_INT_FOA,FileBuffer);
	
	//修复导入目录项
	Import->VirtualAddress = (DWORD)FOA_To_RVA(New_AddressOfImport_FOA,FileBuffer);
	printf("New_AddressOfImport_FOA=0x%X\n",New_AddressOfImport_FOA);
	
	//修改节的属性,否则启动不了 
	LastSection->Characteristics = 0xc0000040; 
	
}

//打印资源表
void PrintResourceDirctory(char* FileBuffer){
	
	//获取资源表 ,也就是第一层 
	_IMAGE_RESOURCE_DIRECTORY* FirstResourceTable = GetResourceDirctory(FileBuffer);
	//获取资源目录表
	 IMAGE_DATA_DIRECTORY* Resource = GetDirectory(FileBuffer,2);
	//FOA
	int FOA = RVA_To_FOA(Resource->VirtualAddress,FileBuffer);
	//递归打印 
	PrintResourceDirectoryByRecursive(FirstResourceTable,FileBuffer,1,FOA);
	
}

//递归打印资源表 
void PrintResourceDirectoryByRecursive(_IMAGE_RESOURCE_DIRECTORY* ResourceTable, char* FileBuffer,int level,int ResourceTableFOA){
	
	for(int k=1;k<level;k++){
		printf("\t");
	}
	printf("当前目录的FOA = 0x%X\n",ResourceTableFOA);
	//获取当前层的目录项的个数 
	int Level_Num = ResourceTable->NumberOfIdEntries + ResourceTable->NumberOfNamedEntries;
	
	for(int k=1;k<level;k++){
		printf("\t");
	}
	printf("当前层中有%d个资源项\n",Level_Num);
	
	//找到第一个目录项
	char* Level_Entry_Start = (char*)(ResourceTable + 1);
	
	//根据资源目录遍历当前层的目录项 
	for(int i=0;i<Level_Num;i++){
		
		//第i个目录项 
		_IMAGE_RESOURCE_DIRECTORY_ENTRY* Dir_Entry_i = (_IMAGE_RESOURCE_DIRECTORY_ENTRY*)(Level_Entry_Start + i*sizeof(_IMAGE_RESOURCE_DIRECTORY_ENTRY));
		
		for(int k=1;k<level;k++){
			printf("\t");
		}
		printf("当前目录项的FOA = 0x%X\n",ResourceTableFOA + sizeof(_IMAGE_RESOURCE_DIRECTORY) + i*sizeof(_IMAGE_RESOURCE_DIRECTORY_ENTRY));
		
		//第一个联合体，打印目录项信息 
		if(Dir_Entry_i->NameIsString == 1){//最高位为1，指向结构体 
			//指向名字结构体的RVA???FOA???
			//???
			int Name_Struct_FOA = Dir_Entry_i->NameOffset + ResourceTableFOA;
			_IMAGE_RESOURCE_DIR_STRING_U* Name_Struct = (_IMAGE_RESOURCE_DIR_STRING_U*)(FileBuffer + Name_Struct_FOA); 
			
			char* Pname;
			memcpy(Pname,Name_Struct->NameString,Name_Struct->Length);
			strcat(Pname,'\0');
			for(int k=1;k<level;k++){
				printf("\t");
			}
			printf("目录项名称：%s",Pname);
		
		}
		else{//最高位为0，值为资源编号 
			int id = Dir_Entry_i->NameOffset;
			for(int k=1;k<level;k++){
				printf("\t");
			}
			printf("目录项id：%d\n",id);
		}
		//第二个联合体，指向下一层目录
		if(Dir_Entry_i->DataIsDirectory == 1){//最高位为1，指向下一层目录 
			//下一层的资源目录
			//能进行递归并找到正确的结构体！！！ 
			int NextResourceFOA = Dir_Entry_i->OffsetToDirectory + ResourceTableFOA;
			_IMAGE_RESOURCE_DIRECTORY* NextResourceTable = (_IMAGE_RESOURCE_DIRECTORY*)(FileBuffer + NextResourceFOA);
			
			//进行递归
			PrintResourceDirectoryByRecursive(NextResourceTable,FileBuffer,level+1,ResourceTableFOA);
			
		}
		else{//最高位为0，指向 _IMAGE_DATA_DIRECTORY 结构体
			//最高位为0， OffsetToDirectory和OffsetToData都一样 
			int ResourceDataTableFOA = Dir_Entry_i->OffsetToDirectory + ResourceTableFOA;
			_IMAGE_RESOURCE_DATA_ENTRY* ResourceDataTable = (_IMAGE_RESOURCE_DATA_ENTRY*)(FileBuffer + ResourceDataTableFOA);
			
			for(int k=1;k<level;k++){
				printf("\t");
			}
			printf("Size: 0x%X , RVA: 0x%X\n",ResourceDataTable->Size,ResourceDataTable->OffsetToData);
			
			return;
			
		}
	
	}
	
}

int main(){
	
	FILE* fp = OpenFile(FilePath);//打开文件 
	
	char* FileBuffer = ReadFileIntoMemory(fp);//读入内存 

	PEAnalyze(FileBuffer);//PE分析
	
//	PrintImportDirctory(FileBuffer);
	
//	//新增节 
//	char* ImageBuffer = PELoading(FileBuffer);
//	char* AddedBuffer = AddSection(ImageBuffer);
//	char* NewFileBuffer = PEUnLoad(AddedBuffer);
	
	//将导出表的信息移动到新增节中
//	MovExportDirctory(NewFileBuffer);
	//将重定位表的信息移动后新的导出表后面 
//	MovRelocationDirctory(NewFileBuffer);
	//修改基址，然后修正重定位块中的地址所指向的固定地址 

	
//	FixRelocationBlock(FileBuffer,0x6C490000);//内存中一个页的大小是0x10000 64Kb 

	//导入表注入 
//	InsertImportDirctory(NewFileBuffer);
	
	//写入文件
//	WriteIntoFile(FileBuffer,"TestDll_After_Insert_ImportTable.dll"); 

	//打印资源表
	PrintResourceDirctory(FileBuffer);
	
	return 0;
	
}
