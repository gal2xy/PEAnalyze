//#include <windows.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include "OwnPE.h"

#define FilePath "ipmsg.exe"
//#define NewFilePath "TestDll++.dll"
#define MaxSectionNum 96


//IMAGE_DOS_HEADER* DOS = NULL;//DOSͷ
//IMAGE_NT_HEADERS32* NT32 = NULL;//32λNTͷ 
//IMAGE_NT_HEADERS64* NT64 = NULL;//64λNTͷ 
//IMAGE_SECTION_HEADER* SectionHeader[MaxSectionNum];//�ڱ�����ָ�� 


int GetFunctionAddrByName(char* FunctionName, char* FileBuffer);//ͨ��������������ȡ������ַFOA 
int GetFunctionAddrByOrdinal(int FunctionOrdinal, char* FileBuffer);//ͨ������������Ż�ȡ������ַFOA 

int RVA_To_FOA(int RVA,char* FileBuffer);//�ڴ�ƫ��ת�ļ�ƫ�� 
int FOA_To_RVA(int FOA, char* FileBuffer);//�ļ�ƫ��ת�ڴ�ƫ��

IMAGE_DATA_DIRECTORY* GetDirectory(char* FileBuffer, int pos);//��ȡָ��Ŀ¼�� 
IMAGE_EXPORT_DIRECTORY* GetExportDirctory(char* FileBuffer);//��õ�����(FOA)
_IMAGE_BASE_RELOCATION* GetRelocationDirctory(char* FileBuffer);//����ض�λ��(FOA)
_IMAGE_BOUND_IMPORT_DESCRIPTOR* GetBoundImportDirctory(char* FileBuffer);//��ȡ�󶨵���� 
_IMAGE_IMPORT_DESCRIPTOR* GetImportDirctory(char* FileBuffer);//��ȡ����� 
_IMAGE_RESOURCE_DIRECTORY* GetResourceDirctory(char* FileBuffer);//��ȡ��Դ�� 

void PrintRelocationBlock(char* FileBuffer);//��ӡ�ض�λ��
void PrintImportDirctory(char* FileBuffer);//��ӡ����� 
void PrintBoundImportDirctory(char* FileBuffer);//��ӡ�󶨵���� 
void PrintResourceDirctory(char* FileBuffer);//��ӡ��Դ��


//�ݹ��ӡ��Դ�� 
void PrintResourceDirectoryByRecursive(_IMAGE_RESOURCE_DIRECTORY* ResourceTable, char* FileBuffer,int level,int ResourceTableFOA);


void MovExportDirctory(char* FileBuffer);//�ƶ��������µĽ��� 
void MovRelocationDirctory(char* FileBuffer);//�ƶ��ض�λ���µĽ���
void FixRelocationBlock(char* FileBuffer,int NewImageBase);//���ݸ������»�ַ�������ض�λ�� 
void MovImportDirctory(char* FileBuffer);//�ƶ������ (δ���)
void InsertImportDirctory(char* FileBuffer);//dllע�룬�����ע�� 

//��ȡĿ¼�� 
IMAGE_DATA_DIRECTORY* GetDirectory(char* FileBuffer, int pos){
	
	//��ȡ��Ҫ����
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)FileBuffer;
	//��һ���ǵ������Ŀ¼�� 
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
//��ȡ�ض�λ�� 
_IMAGE_BASE_RELOCATION* GetRelocationDirctory(char* FileBuffer){
	
	_IMAGE_BASE_RELOCATION* RelocationTable = NULL;
	
	IMAGE_DATA_DIRECTORY* DATA_DIRECTORY5 = GetDirectory(FileBuffer,5);
	
	//������ƫ����ת�����ļ�ƫ���� 
	int FOA = RVA_To_FOA(DATA_DIRECTORY5->VirtualAddress,FileBuffer);

	//�ҵ��ض�λ���ַ 
	RelocationTable = (_IMAGE_BASE_RELOCATION*)(FileBuffer + FOA);
	
	return  RelocationTable;
}
//��ȡ������ 
IMAGE_EXPORT_DIRECTORY* GetExportDirctory(char* FileBuffer){
	
	IMAGE_EXPORT_DIRECTORY* ExportTable = NULL;
	
	IMAGE_DATA_DIRECTORY* DATA_DIRECTORY0 = GetDirectory(FileBuffer,0);
	
	//������ƫ����ת�����ļ�ƫ���� 
	int FOA = RVA_To_FOA(DATA_DIRECTORY0->VirtualAddress,FileBuffer);
	
	//�ҵ�������ṹ�� 
	ExportTable = (IMAGE_EXPORT_DIRECTORY*)(FileBuffer + FOA);
	
	return  ExportTable;
	
}
//��ȡ�󶨵���� 
_IMAGE_BOUND_IMPORT_DESCRIPTOR* GetBoundImportDirctory(char* FileBuffer){
	
	_IMAGE_BOUND_IMPORT_DESCRIPTOR* BoundImportTable = NULL;
	
	IMAGE_DATA_DIRECTORY* DATA_DIRECTORY11 = GetDirectory(FileBuffer,11);
	
	int FOA = RVA_To_FOA(DATA_DIRECTORY11->VirtualAddress,FileBuffer);
	
	BoundImportTable = (_IMAGE_BOUND_IMPORT_DESCRIPTOR*)(FileBuffer + FOA);
	
	return BoundImportTable;
	
}
//��ȡ����� 
_IMAGE_IMPORT_DESCRIPTOR* GetImportDirctory(char* FileBuffer){
	
	_IMAGE_IMPORT_DESCRIPTOR* ImportTable = NULL;
	
	IMAGE_DATA_DIRECTORY* DATA_DIRECTORY1 = GetDirectory(FileBuffer,1);
	
	int FOA = RVA_To_FOA(DATA_DIRECTORY1->VirtualAddress,FileBuffer);
	
	ImportTable = (_IMAGE_IMPORT_DESCRIPTOR*)(FileBuffer + FOA);
	
	return ImportTable;
	
}
//��ȡ��Դ�� 
_IMAGE_RESOURCE_DIRECTORY* GetResourceDirctory(char* FileBuffer){
		
	_IMAGE_RESOURCE_DIRECTORY* ResourceTable = NULL;
	
	IMAGE_DATA_DIRECTORY* DATA_DIRECTORY2 = GetDirectory(FileBuffer,2);
	
	int FOA = RVA_To_FOA(DATA_DIRECTORY2->VirtualAddress,FileBuffer);
	
	ResourceTable = (_IMAGE_RESOURCE_DIRECTORY*)(FileBuffer + FOA);
	
	return ResourceTable;

}

int RVA_To_FOA(int RVA,char* FileBuffer){
	
	int FOA = -1;
	
	//��ȡ��Ҫ����
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
	
	//���ڴ����ҵ���һ���ڱ��ַ
	IMAGE_SECTION_HEADER* FirstSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize);
	
	
	//�ж�RVA�Ƿ����ļ�ͷ(DOS+NT+�ڱ�)��
	if(RVA <= HeaderSize){
		FOA = RVA;
	}
	else{
		//�������ĸ����� 
		for(int i=0;i<SectionNum;i++) {
			
			int start = (FirstSection+i)->VirtualAddress;
			int end = (FirstSection+i)->VirtualAddress + (FirstSection+i)->SizeOfRawData;
			//�ж��Ƿ��ڵ�ǰ���� 
			if(RVA >= start && RVA <= end){
				//������Ե�ǰ�ڵ�ƫ���� 
				int Offset = RVA - (FirstSection+i)->VirtualAddress;
				//�����ļ��е�ƫ����+��Խڵ�ƫ���� 
				FOA = Offset + (FirstSection+i)->PointerToRawData;
			}
			
		}
		
	}
	
	if(FOA == -1){
		printf("RVA_TO_FOAת��ʧ�ܣ��������RVA��Ӧ�Ŀռ�ʱPE����ʱ����ĵط������ļ����Ҳ�����Ӧ�ռ䣡\n");
	}
	
	return FOA; 
	
}

int FOA_To_RVA(int FOA, char* FileBuffer){
	
	int RVA = -1;
	
	//��ȡ��Ҫ����
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
	
	//���ڴ����ҵ���һ���ڱ��ַ
	IMAGE_SECTION_HEADER* FirstSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize);
	
	
	//�ж�FOA�Ƿ����ļ�ͷ(DOS+NT+�ڱ�)��
	if(FOA <= HeaderSize){
		RVA = FOA;
	}
	else{
		//�������ĸ����� 
		int pos = 0;
		for(int i=0;i<SectionNum;i++) {
			int start = (FirstSection+i)->PointerToRawData;
			int end = (FirstSection+i)->PointerToRawData + (FirstSection+i)->SizeOfRawData;
			//�ж��Ƿ��ڵ�ǰ���� 
			if(FOA >= start && FOA <= end){
				//������Ե�ǰ�ڵ�ƫ���� 
				int Offset = FOA - (FirstSection+i)->PointerToRawData;
				//�����ļ��е�ƫ����+��Խڵ�ƫ���� 
				RVA = Offset + (FirstSection+i)->VirtualAddress;
			}
		}
	}
	
	if(RVA == -1){
		printf("FOA_TO_RVAת��ʧ�ܣ�\n");
	}
	
	return RVA;
	
}
//�ڵ�������ͨ����������ȡ������ַ 
int GetFunctionAddrByName(char* FunctionName, char* FileBuffer){
	
	int FunctionFOA = -1;
	
	//��ȡ������
	IMAGE_EXPORT_DIRECTORY* ExportTable = GetExportDirctory(FileBuffer);
	//�ҵ�����������ֱ�,�ԱȺ������ҵ�������� 
	int NameAddress_FOA = RVA_To_FOA(ExportTable->AddressOfNames,FileBuffer);
	int pos = -1; //�±�
	for(int i=0;i<ExportTable->NumberOfNames;i++){
		//��������ַRVA
		int NameAddr = *(DWORD*)(FileBuffer + NameAddress_FOA+4*i);
		NameAddr = RVA_To_FOA(NameAddr,FileBuffer);
		//��ȡ������
		char* fname = (char*)(FileBuffer + NameAddr);
		if(strcmp(fname,FunctionName) == 0){
			printf("Find it by name\n");
			pos = i; 
			break;
		}
	}
	
	//û���ҵ����� 
	if(pos == -1){
		printf("Cann't get function's address by name!\n");
		return FunctionFOA;
	}
	
	//ͨ���±��ú������
	short FunctionOrdinal = 0;
	int NameOrdinalAddress_FOA = RVA_To_FOA(ExportTable->AddressOfNameOrdinals,FileBuffer);
	FunctionOrdinal = *(short*)(FileBuffer + NameOrdinalAddress_FOA + 2*pos);
	printf("%s�ĺ������Ϊ%d\n",FunctionName,FunctionOrdinal+ExportTable->Base);
	//���������Ϊ�±�Ӻ�����ַ�����ҵ���Ӧ�ĺ�����ַ
	int FunctionAddress_FOA = RVA_To_FOA(ExportTable->AddressOfFunctions,FileBuffer);
	int FunctionRVA = *(int*)(FileBuffer + FunctionAddress_FOA + 4*FunctionOrdinal);
	
	FunctionFOA = RVA_To_FOA(FunctionRVA,FileBuffer);
	
	return FunctionFOA; 
	
}
//�ڵ�������ͨ������������Ż�ȡ������ַ 
int GetFunctionAddrByOrdinal(int FunctionOrdinal, char* FileBuffer){
	
	int FunctionFOA = -1;
	//��ȡ������
	IMAGE_EXPORT_DIRECTORY* ExportTable = GetExportDirctory(FileBuffer);
	
	//�±� = �������-�����
	int pos =  FunctionOrdinal - ExportTable->Base;
	
	if(pos < 0 || pos >= ExportTable->NumberOfFunctions){
		printf("�������������Ч��\n");
		return FunctionFOA;
	} 
	
	//�����±�Ӻ�����ַ�����ҵ���Ӧ������ַ 
	int FunctionAddress_FOA = RVA_To_FOA(ExportTable->AddressOfFunctions,FileBuffer);
	int FunctionRVA = *(int*)(FileBuffer + FunctionAddress_FOA + 4*pos);
	
	FunctionFOA = RVA_To_FOA(FunctionRVA,FileBuffer);
	
	return FunctionFOA;
	
}
//�ƶ������� 
void MovExportDirctory(char* FileBuffer){
	
	//��ȡ������ 
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
	
	//������һ���ڱ�
	IMAGE_SECTION_HEADER* LastSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*(SectionNum - 1));
	
	//��ȡ�����ڵ��ļ�ƫ�Ƶ�ַ,��Ϊ�µĺ�����ַ��ĵ�ַ 
	int New_AddressOfFunctions_FOA = LastSection->PointerToRawData + LastSection->Misc.VirtualSize;

	//��������ͷ(SizeOfHeader֮��)��
	//���ƺ�����ַ��
	int AddressOfFunctions_FOA = RVA_To_FOA(ExportTable->AddressOfFunctions, FileBuffer);
	memcpy(FileBuffer + New_AddressOfFunctions_FOA, FileBuffer + AddressOfFunctions_FOA, 4 * ExportTable->NumberOfFunctions);
	//�µĺ���������ű���ļ�ƫ���� 
	int New_AddressOfNameOrdinals_FOA = New_AddressOfFunctions_FOA + 4 * ExportTable->NumberOfFunctions;
	
	//���ƺ���������ű�
	int AddressOfNameOrdinals_FOA = RVA_To_FOA(ExportTable->AddressOfNameOrdinals, FileBuffer);
	memcpy(FileBuffer + New_AddressOfNameOrdinals_FOA, FileBuffer + AddressOfNameOrdinals_FOA, 2*ExportTable->NumberOfNames);
	//�µĺ�������ַ����ļ�ƫ����
	int New_AddressOfNames_FOA = New_AddressOfNameOrdinals_FOA + 2 * ExportTable->NumberOfNames;
	
	//���ƺ�������ַ��
	int AddressOfNames_FOA = RVA_To_FOA(ExportTable->AddressOfNames, FileBuffer);
	memcpy(FileBuffer + New_AddressOfNames_FOA, FileBuffer + AddressOfNames_FOA, 4*ExportTable->NumberOfNames);
	//��������ַ���е�һ�����������ڵ��µ��ļ�ƫ����
	int New_EachAddressOfName_FOA = New_AddressOfNames_FOA + 4 * ExportTable->NumberOfNames;
	
	//���ݺ�������ַ���ƺ�����
	for(int i=0;i<ExportTable->NumberOfNames;i++){
		
		//��ȡ���������ڵ�ַ
		int NameAddr = *(DWORD*)(FileBuffer + AddressOfNames_FOA + 4*i);
		//ת��FOA
		NameAddr = RVA_To_FOA(NameAddr,FileBuffer); 
		//��ȡ��������С
		char* fname = (char*)(FileBuffer + NameAddr);
		int fnlen = strlen(fname) + 1; //Ҫ��'\0'�������� 
		//���ƺ�����
		memcpy(FileBuffer + New_EachAddressOfName_FOA, fname, fnlen);
		//������������ַ��
		*(int*)(FileBuffer + New_AddressOfNames_FOA + 4*i)  = FOA_To_RVA(New_EachAddressOfName_FOA, FileBuffer);
		//��һ�����������ڵ��µ�ַ
		New_EachAddressOfName_FOA += fnlen;
		
	}
	
	//���Ƶ�����ṹ��
	//����ƶ��ṹ�壬���㽫����������ƶ���������еĵط� 
	IMAGE_DATA_DIRECTORY* directory0 = GetDirectory(FileBuffer,0);
	int Old_ExportTable_FOA = RVA_To_FOA(directory0->VirtualAddress, FileBuffer);
	int New_AddressOfExportTable_FOA = New_EachAddressOfName_FOA;
	memcpy(FileBuffer + New_AddressOfExportTable_FOA, FileBuffer + Old_ExportTable_FOA, sizeof(IMAGE_EXPORT_DIRECTORY));
	
	//����������ṹ�������
	IMAGE_EXPORT_DIRECTORY* NewExportTable = (IMAGE_EXPORT_DIRECTORY*)(FileBuffer + New_AddressOfExportTable_FOA);
	NewExportTable->AddressOfFunctions = FOA_To_RVA(New_AddressOfFunctions_FOA, FileBuffer);
	NewExportTable->AddressOfNameOrdinals = FOA_To_RVA(New_AddressOfNameOrdinals_FOA, FileBuffer);
	NewExportTable->AddressOfNames = FOA_To_RVA(New_AddressOfNames_FOA, FileBuffer);
	
	//�������������ڵ�Ŀ¼�� 
	directory0->VirtualAddress = FOA_To_RVA(New_AddressOfExportTable_FOA, FileBuffer);
	
	//�޸Ķ�Ӧ�ڱ������,������������������ʱ�ܶ�λ
	//Ҫʹ���ܸ��ã���Ҫ�������ڱ��ʱ���޸���Щ����
	//������Խڵ�ƫ���������� 
	int NextBlankArea = New_AddressOfExportTable_FOA + sizeof(IMAGE_EXPORT_DIRECTORY) - LastSection->PointerToRawData;
	//��ʵ�����޸� SizeOfRawData���޸������ʹ�ýڱ��С��ֻҪ�޸�VirtualSize���У��������ҵ��հ��� 
//	LastSection->SizeOfRawData = ((NextBlankArea - 1)/FileAlign + 1)*FileAlign;
	LastSection->Misc.VirtualSize = NextBlankArea;
	
}
//�ƶ��ض�λ�� 
void MovRelocationDirctory(char* FileBuffer){
	
	//��ȡ�ض�λ��
	_IMAGE_BASE_RELOCATION* RelocationBlock = GetRelocationDirctory(FileBuffer); 
	
	//��ȡ��Ҫ����
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
	
	//��������������ȡ���һ���ڱ�
	IMAGE_SECTION_HEADER* LastSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*(SectionNum - 1));
	
	//���ݽڱ��ҵ��ڣ�����λ�հ���
	int New_AddressOfRelocation_FOA = LastSection->PointerToRawData + LastSection->Misc.VirtualSize;
	int New_Block_FOA = New_AddressOfRelocation_FOA;
	
	//��ȡ�ض�λĿ¼��
	IMAGE_DATA_DIRECTORY* Relocation = GetDirectory(FileBuffer, 5);  
	//ѭ�������ض�λ��
	int Old_Block_FOA = RVA_To_FOA(Relocation->VirtualAddress,FileBuffer); 
	while(1){
		//�ض�λ�����������־ 
		if(RelocationBlock->VirtualAddress == 0 && RelocationBlock->SizeOfBlock == 0){
			break;
		}
		//ֱ�Ӹ��� SizeOfBlock��С 
		memcpy(FileBuffer + New_Block_FOA, FileBuffer + Old_Block_FOA, RelocationBlock->SizeOfBlock);                         
		//��һ���ض�λ����ļ�ƫ�� 
		RelocationBlock = (_IMAGE_BASE_RELOCATION*)((char*)RelocationBlock + RelocationBlock->SizeOfBlock); 
		New_Block_FOA += RelocationBlock->SizeOfBlock;
		Old_Block_FOA += RelocationBlock->SizeOfBlock;

	}
	
	//�Ժ���8���ֽڸ�0,��Ϊ������־
	memset(FileBuffer + New_Block_FOA, 0, 8);
	
	//�޸��ض�λĿ¼��� VirtualAddress
	Relocation->VirtualAddress = FOA_To_RVA(New_AddressOfRelocation_FOA, FileBuffer);
	
	//�޸���Ӧ�ڵ����ԣ����������� 
	int NextBlankArea = (New_Block_FOA + 8) - LastSection->PointerToRawData;
	LastSection->Misc.VirtualSize = NextBlankArea;
	
}

//�ƶ������ (δ���)
void MovImportDirctory(char* FileBuffer){
	
	//��ȡ����� 
	_IMAGE_IMPORT_DESCRIPTOR* ImportTable = GetImportDirctory(FileBuffer);
	//��ȡ����Ŀ¼��
	IMAGE_DATA_DIRECTORY* Import = GetDirectory(FileBuffer, 1);
	//��ȡ�ɵĵ�����ʼFOA 
	int Old_Imoprt_FOA = RVA_To_FOA(Import->VirtualAddress, FileBuffer);
	
	//��ȡ��Ҫ����
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
	
	//��������������ȡ���һ���ڱ�
	IMAGE_SECTION_HEADER* LastSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*(SectionNum - 1));
	
	//���ݽڱ��ҵ��ڣ�����λ�հ�������Ϊ�µĵ�����ʼFOA 
	int New_AddressOfImport_FOA = LastSection->PointerToRawData + LastSection->Misc.VirtualSize;
	int New_Import_FOA = New_AddressOfImport_FOA;
	
	//����һ��ȫ��ṹ��
	_IMAGE_IMPORT_DESCRIPTOR* ZeroImportTable = (_IMAGE_IMPORT_DESCRIPTOR*)malloc(sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	memset(ZeroImportTable, 0, sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	
	//���ƶ����е����
	while(1){
		
		//�жϽ�����־,ȫ��ṹ�����Ƚ� 
		if(memcmp(ImportTable, ZeroImportTable, sizeof(_IMAGE_IMPORT_DESCRIPTOR)) == 0){
			break;
		}
		
		//��ֻ����x32�� 
		//��������ƶ����µĽ��� 
		memcpy(FileBuffer + New_Import_FOA, FileBuffer + Old_Imoprt_FOA, sizeof(_IMAGE_IMPORT_DESCRIPTOR));
		
		//������һ�������ĳ�ʼFOA 
		New_Import_FOA += sizeof(_IMAGE_IMPORT_DESCRIPTOR); 
		Old_Imoprt_FOA += sizeof(_IMAGE_IMPORT_DESCRIPTOR);
		
		//��һ�������ĵ�ַ 
		ImportTable += 1;
		
	}
	
	//�����հ����������ע�� 
	//Ҫ��Ҫ��INT���ƶ��������Լ�dll���ƺ�dll�к������ƣ�����
	//δ�꣡���������� 
	
}

//�޸��ض�λ�� 
void FixRelocationBlock(char* FileBuffer,int NewImageBase){
	
	//��ȡ�ض�λ��
	_IMAGE_BASE_RELOCATION* RelocationBlock = GetRelocationDirctory(FileBuffer);
	
	//��ȡ��Ҫ����
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
	
	//�����޸Ļ�ַ��Ĳ�ֵ 
	int diff = NewImageBase - ImageBase;
	//�����ض�λ�飬�����ض�λ������ĵ�ַ���ж��Ƿ�Ҫ�޸ģ���Ҫ�޸ģ����ݵ�ַ�ҵ�Ҫ�޸ĵĵ�ַ 
	while(1){
		//�ض�λ�����������־ 
		if(RelocationBlock->VirtualAddress == 0 && RelocationBlock->SizeOfBlock == 0){
			break;
		}
		//��ÿ�洢���ݵĵ�ַ 
		char* addr = (char*)RelocationBlock + 8;
		int num = (RelocationBlock->SizeOfBlock - 8)/2;
		for(int i=0;i<num;i++){
			WORD data= *(WORD*)(addr+2*i);
			WORD att = (data & 0xf000)>>12;
			//�ж��Ƿ���Ҫ�޸�
			if(att == 3) {//Ҫ�޸� 
				DWORD RVA = RelocationBlock->VirtualAddress + (data&0x0fff);
				DWORD FOA = RVA_To_FOA(RVA,FileBuffer);
				//����FOA�ҵ�Ҫ�޸ĵ�����
				DWORD* AlterAddr = (DWORD*)(FileBuffer + FOA);
				*AlterAddr += diff;
			}
		}
		//��һ���ض�λ�����ʼ��ַ 
		RelocationBlock = (_IMAGE_BASE_RELOCATION*)((char*)RelocationBlock + RelocationBlock->SizeOfBlock); 

	}
	
	
}
//��ӡ�ض�λ�� 
void PrintRelocationBlock(char* FileBuffer){
	
	_IMAGE_BASE_RELOCATION* RelocationArray = GetRelocationDirctory(FileBuffer);
	
	while(1){
		//�ض�λ�����������־ 
		if(RelocationArray->VirtualAddress == 0 && RelocationArray->SizeOfBlock == 0){
			break;
		}
		//��ӡÿ�������
		printf("---------------------------\n");
		printf("VirtualAddress:0x%X\n",RelocationArray->VirtualAddress);
		printf("SizeOfBlock:%d\n",RelocationArray->SizeOfBlock);
		
		//��ÿ�洢���ݵĵ�ַ 
		char* addr = (char*)RelocationArray + 8;
		int num = (RelocationArray->SizeOfBlock - 8)/2;
		for(int i=0;i<num;i++){
			
			WORD data= *(WORD*)(addr+2*i);
			DWORD RVA = RelocationArray->VirtualAddress + (data&0x0fff);
			WORD att = (data & 0xf000)>>12;
			printf("��%d��:\t��ַ(RVA)=0x%X\t����=%d\n",i,RVA,att);
			
		}
		//��һ���ض�λ�����ʼ��ַ 
		RelocationArray = (_IMAGE_BASE_RELOCATION*)((char*)RelocationArray + RelocationArray->SizeOfBlock); 
		printf("---------------------------\n");

	}
	
}
//��ӡ����� 
void PrintImportDirctory(char* FileBuffer){
	
	//�ҵ������
	_IMAGE_IMPORT_DESCRIPTOR* ImportTable = GetImportDirctory(FileBuffer);
	
	//����һ��ȫ��ṹ��
	_IMAGE_IMPORT_DESCRIPTOR* ZeroImportTable = (_IMAGE_IMPORT_DESCRIPTOR*)malloc(sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	memset(ZeroImportTable, 0, sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	
	//ѭ���������е������ӡ�����Ϣ 
	while(1){
		
		//�жϽ�����־,ȫ��ṹ�����Ƚ� 
		if(memcmp(ImportTable, ZeroImportTable, sizeof(_IMAGE_IMPORT_DESCRIPTOR)) == 0){
			break;
		}
		
		//��ӡdll����
		int Name_FOA = RVA_To_FOA(ImportTable->Name,FileBuffer);
		char* dname = (char*)(FileBuffer + Name_FOA);
		printf("%s--------------------\n",dname); 

		//��ֻ����x32�� 
		//��OriginalFirstThunk (RVA)ת��FOA������FOA�ҵ�INT 
//		int OriginalFirstThunk_FOA = RVA_To_FOA(ImportTable->DUMMYUNIONNAME.OriginalFirstThunk,FileBuffer);
		int OriginalFirstThunk_FOA = RVA_To_FOA(*(DWORD*)ImportTable,FileBuffer);
		IMAGE_THUNK_DATA32* INT = (IMAGE_THUNK_DATA32*)(FileBuffer + OriginalFirstThunk_FOA);
		
		//��FirstThunk (RVA)ת��FOA������FOA�ҵ�IAT
		int FirstThunk_FOA = RVA_To_FOA(ImportTable->FirstThunk,FileBuffer);
		//�����ĸ��򵥣������ĸ����� 
//		IMAGE_THUNK_DATA32* IAT = (IMAGE_THUNK_DATA32*)(FileBuffer + FirstThunk_FOA);
		DWORD* IAT = (DWORD*)(FileBuffer + FirstThunk_FOA);
		//INT��IAT��ʼ״̬������ͬ��


		//ѭ����������INT,��ӡ������ 
		while(1){
			
			//�жϽ�����־
			if(*IAT == 0){
				break; 
			} 
			
			//�������λ�Ƿ�Ϊ1�ж��Ǹ�������(0)���ǵ������(1)���Һ������� 
			//����(0,����λ��ֵΪָ���������ڵĵ�ַ)	�������(1,�����λ����Ϊ��ֵΪ�������) 
			if(((*IAT) & 0x8000) > 0){//���λΪ1 
				int Ordinal = (*IAT) & 0x7FFF;
				printf("����ŵ��룬�����������:0x%x\n",Ordinal);
			}
			else{//���λΪ0
				//��һ���� 
				int ImportByNameAddr = (*IAT);
				int ImportByNameAddr_FOA = RVA_To_FOA(ImportByNameAddr,FileBuffer);
				char* ImportByName  = FileBuffer + ImportByNameAddr_FOA;
				printf("�����ֵ���, hint=0x%X, ������:%s\n",*(WORD*)ImportByName,ImportByName + 2);
			}
			
			//��һ�� 
			IAT += 1;
			
		} 
		
		//��һ�������ĵ�ַ 
		ImportTable += 1;
		
	}
	
}
//����δ������ ������ 
//��ӡ�󶨵���� 
void PrintBoundImportDirctory(char* FileBuffer){
	
	//��ȡ�󶨵����
	_IMAGE_BOUND_IMPORT_DESCRIPTOR* BoundImportTable = GetBoundImportDirctory(FileBuffer);
	//��¼��һ���󶨵����ĵ�ַ��������dll������Ҫʹ�� 
	char* First_FOA = (char*)BoundImportTable;
	
	//����һ���սṹ�� 
	_IMAGE_BOUND_IMPORT_DESCRIPTOR* ZeroBoundImportTable = (_IMAGE_BOUND_IMPORT_DESCRIPTOR*)malloc(sizeof(_IMAGE_BOUND_IMPORT_DESCRIPTOR));
	memset(ZeroBoundImportTable, 0, sizeof(_IMAGE_BOUND_IMPORT_DESCRIPTOR));
	
	//ѭ������ÿ���󶨵���� 
	while(1){
		
		//�ж��Ƿ����
		if(memcmp(BoundImportTable, ZeroBoundImportTable,sizeof(_IMAGE_BOUND_IMPORT_DESCRIPTOR)) == 0){
			break;
		}
		
		//���
//		int DllName_FOA = 
//		int DllName_FOA = RVA_To_FOA(DllName_RVA, FileBuffer);
		char* DllName = First_FOA + BoundImportTable->OffsetModuleName;;
		printf("��ǰ�󶨵����Ĺ�����dll: %s\n",DllName);
		printf("TimeDateStamp:0x%X\n",BoundImportTable->TimeDateStamp);
		printf("NumberOfModuleForwarderRefs:0x%X\n",BoundImportTable->NumberOfModuleForwarderRefs); 
		
		//NumberOfModuleForwarderRefs ���Ϊ��Ϊ�㣬����ڵ�ǰdll��������dll�ģ���Ҫѭ����ӡ 
		//���� NumberOfModuleForwarderRefs ��� _IMAGE_BOUND_FORWARDER_REF
		int cnt = BoundImportTable->NumberOfModuleForwarderRefs;
		for(int i=0;i<cnt;i++) {
			 
			//ת�ɽṹ��
			_IMAGE_BOUND_FORWARDER_REF* ref = (_IMAGE_BOUND_FORWARDER_REF*)((char*)BoundImportTable + sizeof(_IMAGE_BOUND_IMPORT_DESCRIPTOR) + i*sizeof(_IMAGE_BOUND_FORWARDER_REF));
			
			//��ȡ�����õ�dll������
			char* CalledDllName = First_FOA + ref->OffsetModuleName;
			printf("�����õ�dll����: %s\n",CalledDllName);
			printf("TimeDateStamp:0x%X\n",ref->TimeDateStamp);
			printf("Reserved:0x%X\n",ref->Reserved); 
			
		}
		//��һ���󶨵����
		//_IMAGE_BOUND_IMPORT_DESCRIPTOR �� _IMAGE_BOUND_FORWARDER_REF ��С��ͬ
		BoundImportTable = (_IMAGE_BOUND_IMPORT_DESCRIPTOR*)((char*)BoundImportTable + sizeof(_IMAGE_BOUND_IMPORT_DESCRIPTOR)*(cnt + 1)); 
		
	}
	
}
//dllע�룬�����ע�� 
void InsertImportDirctory(char* FileBuffer){
	
	//��ȡ����� 
	_IMAGE_IMPORT_DESCRIPTOR* ImportTable = GetImportDirctory(FileBuffer);
	//��ȡ����Ŀ¼��
	IMAGE_DATA_DIRECTORY* Import = GetDirectory(FileBuffer, 1);
	//��ȡ�ɵĵ�����ʼFOA 
	int Old_Imoprt_FOA = RVA_To_FOA(Import->VirtualAddress, FileBuffer);
	
	//��ȡ��Ҫ����
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
	
	//��������������ȡ���һ���ڱ�
	IMAGE_SECTION_HEADER* LastSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*(SectionNum - 1));
	
	//���ݽڱ��ҵ��ڣ�����λ�հ�������Ϊ�µĵ�����ʼFOA 
	int New_AddressOfImport_FOA = LastSection->PointerToRawData + LastSection->Misc.VirtualSize;
	int New_Import_FOA = New_AddressOfImport_FOA;
	
	//����һ��ȫ��ṹ��
	_IMAGE_IMPORT_DESCRIPTOR* ZeroImportTable = (_IMAGE_IMPORT_DESCRIPTOR*)malloc(sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	memset(ZeroImportTable, 0, sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	
	//�ƶ����е����
	while(1){
		
		//�жϽ�����־,ȫ��ṹ�����Ƚ� 
		if(memcmp(ImportTable, ZeroImportTable, sizeof(_IMAGE_IMPORT_DESCRIPTOR)) == 0){
			break;
		}
		
		//��ֻ����x32�� 
		//��������ƶ����µĽ��� 
		memcpy(FileBuffer + New_Import_FOA, FileBuffer + Old_Imoprt_FOA, sizeof(_IMAGE_IMPORT_DESCRIPTOR));
		
		//������һ�������ĳ�ʼFOA 
		New_Import_FOA += sizeof(_IMAGE_IMPORT_DESCRIPTOR); 
		Old_Imoprt_FOA += sizeof(_IMAGE_IMPORT_DESCRIPTOR);
		
		//��һ�������ĵ�ַ 
		ImportTable += 1;
		
	}
	
	//ĩβ���������
	_IMAGE_IMPORT_DESCRIPTOR* NewImportTable = (_IMAGE_IMPORT_DESCRIPTOR*)(FileBuffer + New_Import_FOA);
	New_Import_FOA += sizeof(_IMAGE_IMPORT_DESCRIPTOR);
	//������Ϊ������־ 
	memset(FileBuffer + New_Import_FOA, 0, sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	
	//�����¿ռ��INT��IAT,���ռ������������������ 
	//������ 
	//����8�ֽ�INT��һ��˫��ʹINT����Ч��һ��˫��ȫ����Ϊ������־ 
	int New_INT_FOA = New_Import_FOA + 0x20;
	IMAGE_THUNK_DATA32 * New_INT = (IMAGE_THUNK_DATA32*)(FileBuffer + New_INT_FOA);

	//ͬ������8�ֽڵ�IAT�� 
	int New_IAT_FOA  = New_INT_FOA + 0x10;
	IMAGE_THUNK_DATA32* New_IAT = (IMAGE_THUNK_DATA32*)(FileBuffer + New_IAT_FOA);
	
	//���ٿռ��dll���� 
	int New_DllName_FOA = New_IAT_FOA + 0x10;
	char* DllName = "InjectDll.dll";
	memcpy(FileBuffer + New_DllName_FOA, DllName, strlen(DllName) + 1);
	
	//���ٿռ��dll�������� 
	int New_DllFunName_FOA = New_DllName_FOA + 0x20;
	IMAGE_IMPORT_BY_NAME* New_DllFunc = (IMAGE_IMPORT_BY_NAME*)(FileBuffer + New_DllFunName_FOA);
	char* FunName = "ExportFunction";//���޸� 
	memcpy(FileBuffer + New_DllFunName_FOA + 2, FunName, strlen(FunName) + 1);
	
	//�޸�INT��IAT�� ,�� New_DllFunc ��RVA��ֵ��INT��IAT
	New_INT->u1.AddressOfData = (DWORD)FOA_To_RVA(New_DllFunName_FOA,FileBuffer);
	New_IAT->u1.AddressOfData = (DWORD)FOA_To_RVA(New_DllFunName_FOA,FileBuffer);
	
	//�������������Name��OriginalFirstThunk��FirstThunk
	NewImportTable->FirstThunk = (DWORD)FOA_To_RVA(New_IAT_FOA,FileBuffer);
	NewImportTable->Name = (DWORD)FOA_To_RVA(New_DllName_FOA,FileBuffer); 
	*(DWORD*)NewImportTable = (DWORD)FOA_To_RVA(New_INT_FOA,FileBuffer);
	
	//�޸�����Ŀ¼��
	Import->VirtualAddress = (DWORD)FOA_To_RVA(New_AddressOfImport_FOA,FileBuffer);
	printf("New_AddressOfImport_FOA=0x%X\n",New_AddressOfImport_FOA);
	
	//�޸Ľڵ�����,������������ 
	LastSection->Characteristics = 0xc0000040; 
	
}

//��ӡ��Դ��
void PrintResourceDirctory(char* FileBuffer){
	
	//��ȡ��Դ�� ,Ҳ���ǵ�һ�� 
	_IMAGE_RESOURCE_DIRECTORY* FirstResourceTable = GetResourceDirctory(FileBuffer);
	//��ȡ��ԴĿ¼��
	 IMAGE_DATA_DIRECTORY* Resource = GetDirectory(FileBuffer,2);
	//FOA
	int FOA = RVA_To_FOA(Resource->VirtualAddress,FileBuffer);
	//�ݹ��ӡ 
	PrintResourceDirectoryByRecursive(FirstResourceTable,FileBuffer,1,FOA);
	
}

//�ݹ��ӡ��Դ�� 
void PrintResourceDirectoryByRecursive(_IMAGE_RESOURCE_DIRECTORY* ResourceTable, char* FileBuffer,int level,int ResourceTableFOA){
	
	for(int k=1;k<level;k++){
		printf("\t");
	}
	printf("��ǰĿ¼��FOA = 0x%X\n",ResourceTableFOA);
	//��ȡ��ǰ���Ŀ¼��ĸ��� 
	int Level_Num = ResourceTable->NumberOfIdEntries + ResourceTable->NumberOfNamedEntries;
	
	for(int k=1;k<level;k++){
		printf("\t");
	}
	printf("��ǰ������%d����Դ��\n",Level_Num);
	
	//�ҵ���һ��Ŀ¼��
	char* Level_Entry_Start = (char*)(ResourceTable + 1);
	
	//������ԴĿ¼������ǰ���Ŀ¼�� 
	for(int i=0;i<Level_Num;i++){
		
		//��i��Ŀ¼�� 
		_IMAGE_RESOURCE_DIRECTORY_ENTRY* Dir_Entry_i = (_IMAGE_RESOURCE_DIRECTORY_ENTRY*)(Level_Entry_Start + i*sizeof(_IMAGE_RESOURCE_DIRECTORY_ENTRY));
		
		for(int k=1;k<level;k++){
			printf("\t");
		}
		printf("��ǰĿ¼���FOA = 0x%X\n",ResourceTableFOA + sizeof(_IMAGE_RESOURCE_DIRECTORY) + i*sizeof(_IMAGE_RESOURCE_DIRECTORY_ENTRY));
		
		//��һ�������壬��ӡĿ¼����Ϣ 
		if(Dir_Entry_i->NameIsString == 1){//���λΪ1��ָ��ṹ�� 
			//ָ�����ֽṹ���RVA???FOA???
			//???
			int Name_Struct_FOA = Dir_Entry_i->NameOffset + ResourceTableFOA;
			_IMAGE_RESOURCE_DIR_STRING_U* Name_Struct = (_IMAGE_RESOURCE_DIR_STRING_U*)(FileBuffer + Name_Struct_FOA); 
			
			char* Pname;
			memcpy(Pname,Name_Struct->NameString,Name_Struct->Length);
			strcat(Pname,'\0');
			for(int k=1;k<level;k++){
				printf("\t");
			}
			printf("Ŀ¼�����ƣ�%s",Pname);
		
		}
		else{//���λΪ0��ֵΪ��Դ��� 
			int id = Dir_Entry_i->NameOffset;
			for(int k=1;k<level;k++){
				printf("\t");
			}
			printf("Ŀ¼��id��%d\n",id);
		}
		//�ڶ��������壬ָ����һ��Ŀ¼
		if(Dir_Entry_i->DataIsDirectory == 1){//���λΪ1��ָ����һ��Ŀ¼ 
			//��һ�����ԴĿ¼
			//�ܽ��еݹ鲢�ҵ���ȷ�Ľṹ�壡���� 
			int NextResourceFOA = Dir_Entry_i->OffsetToDirectory + ResourceTableFOA;
			_IMAGE_RESOURCE_DIRECTORY* NextResourceTable = (_IMAGE_RESOURCE_DIRECTORY*)(FileBuffer + NextResourceFOA);
			
			//���еݹ�
			PrintResourceDirectoryByRecursive(NextResourceTable,FileBuffer,level+1,ResourceTableFOA);
			
		}
		else{//���λΪ0��ָ�� _IMAGE_DATA_DIRECTORY �ṹ��
			//���λΪ0�� OffsetToDirectory��OffsetToData��һ�� 
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
	
	FILE* fp = OpenFile(FilePath);//���ļ� 
	
	char* FileBuffer = ReadFileIntoMemory(fp);//�����ڴ� 

	PEAnalyze(FileBuffer);//PE����
	
//	PrintImportDirctory(FileBuffer);
	
//	//������ 
//	char* ImageBuffer = PELoading(FileBuffer);
//	char* AddedBuffer = AddSection(ImageBuffer);
//	char* NewFileBuffer = PEUnLoad(AddedBuffer);
	
	//�����������Ϣ�ƶ�����������
//	MovExportDirctory(NewFileBuffer);
	//���ض�λ�����Ϣ�ƶ����µĵ�������� 
//	MovRelocationDirctory(NewFileBuffer);
	//�޸Ļ�ַ��Ȼ�������ض�λ���еĵ�ַ��ָ��Ĺ̶���ַ 

	
//	FixRelocationBlock(FileBuffer,0x6C490000);//�ڴ���һ��ҳ�Ĵ�С��0x10000 64Kb 

	//�����ע�� 
//	InsertImportDirctory(NewFileBuffer);
	
	//д���ļ�
//	WriteIntoFile(FileBuffer,"TestDll_After_Insert_ImportTable.dll"); 

	//��ӡ��Դ��
	PrintResourceDirctory(FileBuffer);
	
	return 0;
	
}
