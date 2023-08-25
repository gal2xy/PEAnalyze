#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MaxSectionNum 96

typedef void (*FuncPointer)(LPTSTR); //����ָ��  

IMAGE_DOS_HEADER* DOS = NULL;//DOSͷ 
IMAGE_NT_HEADERS32* NT32 = NULL;//32λNTͷ 
IMAGE_NT_HEADERS64* NT64 = NULL;//64λNTͷ 
IMAGE_SECTION_HEADER* SectionHeader[MaxSectionNum];//�ڱ�����

char ShellCode[] = {
	0x6A,0x00,0x6A,0x00,0x6A,0x00,0x6A,0x00,
	0xE8,0x00,0x00,0x00,0x00,
	0xE9,0x00,0x00,0x00,0x00
};


FILE* OpenFile(char* FileName);//���ļ��������ļ�ָ�� 
int getFileSize(FILE* fp);//��ȡ�ļ���С 
char* ReadFileIntoMemory(FILE* fp);//���ļ���ȡ���ڴ��У��������ڴ��е��׵�ַ(ָ��)
void WriteIntoFile(char* FileBuffer,char* FileName);//���ڴ�����д���ļ� ,��ֹPE���غ�д���ļ� 
void PEAnalyze(char* FileBuffer);//PE�ļ��ṹ���� 
char* PELoading(char* FileBuffer);//PE���� 
char* PEUnLoad(char* ImageBuffer);//PEȥ����(�ָ����ļ�) 
void InsertCodeAfterLoading(char* ImageBuffer);//PE���غ�������
int GetMessageBoxAddress();//��ȡMessageBox�����ĵ�ַ
char* ExpendSection(char* ImageBuffer);//����ڣ����һ���ڣ� 
char* AddSection(char* ImageBuffer);//������
void CombineSection(char* ImageBuffer);//�ϲ���(PE���غ���У���Ϊ��������Ѱ�������ڱ�������ǿ�������ĵ�ַ���������ļ���ַ)



char* AddSection(char* ImageBuffer){
	
	int SectionNum = 0;
	int SectionAlign = 0; 
	int OptionSize = 0;
	int ImageSize = 0;
	int HeaderSize = 0;
	//��ȡDOSͷ��PE��׼ͷ	
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
	
	//�ȷ���ImageSize��С�Ŀռ䣬����������
	char* NewBuffer = (char*)malloc(sizeof(char)*ImageSize);
	memset(NewBuffer,0,ImageSize);
	//ȫ���ƹ���
	memcpy(NewBuffer,ImageBuffer,ImageSize);
	//�ж�ʣ��ռ��Ƿ��ܹ���ӽڱ�
	int RemainSpace = HeaderSize - (NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*SectionNum);
	int New_e_lfanew = 0;//�޸ĺ��e_lfanew 
	New_e_lfanew = NewDOS->e_lfanew;//���ʣ��ռ��㹻���Ͳ��øģ��������if�������޸� 
	if(RemainSpace < 40){//������80��Ҫ��һ��ȫ��Ľڱ���Ϊ����ʶ�� 
		if(RemainSpace + NewDOS->e_lfanew - 64 < 40){
			printf("�ռ䲻�㣬�޷���ӽڱ�\n");
			free(NewBuffer);
			return NULL;
		}
		IMAGE_DOS_HEADER* DOSTemp1 = (IMAGE_DOS_HEADER*)NewBuffer;
		//�޸�e_lfanew
		DOSTemp1->e_lfanew = 64;
		New_e_lfanew = DOSTemp1->e_lfanew;
		//���� 
		memset(NewBuffer + DOSTemp1->e_lfanew, 0, HeaderSize - NewDOS->e_lfanew);
		//�ƶ�NT�ͽڱ���DOS Stub���ǵ�
		memcpy(NewBuffer + DOSTemp1->e_lfanew, ImageBuffer + NewDOS->e_lfanew, HeaderSize - NewDOS->e_lfanew); 
	}
	
	//���ڴ����ҵ���һ���ڱ��ַ
	IMAGE_SECTION_HEADER* FirstSection = (IMAGE_SECTION_HEADER*)(NewBuffer + New_e_lfanew + 4 + 20 +OptionSize);
	//���һ���ڱ����ֹ��ַ��Ҳ���������ڱ����ʼ��ַ 
	IMAGE_SECTION_HEADER* AddSection = FirstSection + SectionNum;
	//Ĭ�ϸ��Ƶ�һ���ڵ���Ϣ���µĽڱ� 
	memcpy(AddSection,FirstSection,40);
	//�޸������ڵ�����
//	AddSection->Name = ;
	AddSection->PointerToRawData = (AddSection-1)->PointerToRawData + (AddSection-1)->SizeOfRawData;
	AddSection->VirtualAddress = ImageSize;
	AddSection->Misc.VirtualSize = 0; 
//	AddSection->Characteristics = ;

	//���·���ռ� 
	int ReallocNewSize = ImageSize + ((FirstSection+1)->VirtualAddress - FirstSection->VirtualAddress);
//	char* ReallocNewBuffer = (char*)realloc(NewBuffer,ReallocNewSize);
	char* ReallocNewBuffer = (char*)malloc(sizeof(char)*ReallocNewSize);
	memset(ReallocNewBuffer, 0, ReallocNewSize);
	memcpy(ReallocNewBuffer, NewBuffer, ImageSize);
	free(NewBuffer);
	
	//��ȡNTͷ 
	if(NT32 != NULL){
		//��Ϊ�ط����Ե�ʣ�ԭ�ȿռ䱻free������ֱ��ʹ�ýṹ��ָ��e_lfanew
		IMAGE_NT_HEADERS32* NT = (IMAGE_NT_HEADERS32*)(ReallocNewBuffer + New_e_lfanew);
		//�޸Ľڵ�����
		NT->FileHeader.NumberOfSections += 1;
		printf("�޸��ڴ�������ɹ���\n"); 
		//�޸��ڴ��СSizeOfImage 
		NT->OptionalHeader.SizeOfImage = ReallocNewSize;
		printf("�޸��ڴ��СSizeOfImage�ɹ���\n"); 
	}
	else{
		IMAGE_NT_HEADERS64* NT = (IMAGE_NT_HEADERS64*)(ReallocNewBuffer + New_e_lfanew);
		//�޸Ľڵ�����
		NT->FileHeader.NumberOfSections += 1;
		//�޸��ڴ��СSizeOfImage 
		NT->OptionalHeader.SizeOfImage = ReallocNewSize;
	}
	
	printf("�����ڱ�ɹ���\n");
	
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
	
	//������������Ĵ�С 
	ExpenSize = ((ExpenSize - 1)/SectionAlign + 1)*SectionAlign;
	//�µ��ڴ��С 
	int NewImageSize = ImageSize + ExpenSize;
	//�����µĿռ�
	char* ExpendImageBuffer = (char*)malloc(sizeof(char)*NewImageSize);
	memset(ExpendImageBuffer,0,ImageSize);
	memcpy(ExpendImageBuffer,ImageBuffer,ImageSize);
	//�������һ���� 
	IMAGE_SECTION_HEADER* LastSection = (IMAGE_SECTION_HEADER*)(ExpendImageBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*(SectionNum - 1));
	//�������һ���ڵ��ڴ��С 
	int LastSize = ImageSize - LastSection->VirtualAddress;
	//�޸����� 
	LastSection->SizeOfRawData = LastSize + ExpenSize;
	LastSection->Misc.VirtualSize = LastSize + ExpenSize;
	//�޸�ImageSize
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
	
	//��ȡ NumberOfSections,FileAlignment
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
	
	//���ڴ����ҵ���һ���ڱ��ַ
	IMAGE_SECTION_HEADER* FirstSection = (IMAGE_SECTION_HEADER*)(ImageBuffer + NewDOS->e_lfanew + 4 + 20 +OptionSize);
	//���һ���ڱ����ʼ��ַ
	IMAGE_SECTION_HEADER* LastSection = FirstSection + SectionNum - 1;
	
	//�ϲ��ڱ����ԣ�����ϲ�����ڴ��С
	int AllSectionSize = 0;
	int AllSectionCharacteristics = 0;
	int AllSizeOfRawData = 0;
	
	//�ϲ���ڱ������ 
	for(int i=0;i<SectionNum;i++) {
		AllSectionCharacteristics |= (FirstSection + i)->Characteristics;
	}
	
	//�ϲ���ڱ���ڴ�����С
	int LastSectionSizeOfRawData = LastSection->SizeOfRawData;
	int LastSectionVirtualSize = LastSection->Misc.VirtualSize;
	int MAX = LastSectionSizeOfRawData > LastSectionVirtualSize ? LastSectionSizeOfRawData : LastSectionVirtualSize;
	
	AllSectionSize = LastSection->VirtualAddress + MAX - FirstSection->VirtualAddress;
	AllSizeOfRawData = ((AllSectionSize-1)/FileAlign+1)*FileAlign;
	
	//�޸Ľڱ�� VirtualSize,SizeOfRawData,Characteristics
	FirstSection->Misc.VirtualSize = AllSectionSize; 
	FirstSection->SizeOfRawData = AllSizeOfRawData;
	FirstSection->Characteristics = AllSectionCharacteristics;
	printf("�ϲ��ں�VirtualSize=%X\n",AllSectionSize);
	printf("�ϲ��ں�SizeOfRawData=%X\n",AllSizeOfRawData);
	
	//����ڱ�����
	for(int i=1;i<SectionNum;i++){
		memset(FirstSection + i,0,sizeof(_IMAGE_SECTION_HEADER));
	}
	
	//�ڱ������Ϊ1
	IMAGE_FILE_HEADER* FileHeader = NULL;
	FileHeader = (IMAGE_FILE_HEADER*)(ImageBuffer + NewDOS->e_lfanew + 4);
	FileHeader->NumberOfSections = 1;
	
	return;

}

int GetMessageBoxAddress(){
	
	HINSTANCE LibHandle;
    FuncPointer GetAddr;
    // ���سɹ��󷵻ؿ�ģ��ľ��
    LibHandle = LoadLibrary("user32"); 
//    printf("user32 LibHandle = 0x%X\n", LibHandle);
    // ���ض�̬���ӿ�(DLL)�е�����⺯����ַ
    GetAddr = (FuncPointer)GetProcAddress(LibHandle,"MessageBoxA");   
//    printf("MessageBoxA = 0x%X\n", GetAddr);
//    printf("(int)MessageBoxA = 0x%X\n", (int)(GetAddr));
    return (int)GetAddr;
    
}

void InsertCodeAfterLoading(char* ImageBuffer,int pos){
	
	//�ڶԽڱ���в��������½ṹ��Ӧʹ��ImageBuffer�еĽṹ�� 
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)ImageBuffer;
	
	//��ȡImageBase��OEP��OPtionSize 
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
		//�޸�OEP,�����ԣ���ΪNTָ��ָ�����FileBuffer��������ͨ������޸�ImageBuffer���ֵ 
//		NT64->OptionalHeader.AddressOfEntryPoint = ShellCodeOffset; 
	}
	
	//��һ���ڱ��ƫ�Ƶ�ַ 
	int SectionOffset = 0;
	SectionOffset = NewDOS->e_lfanew + 4 + 20 + OptionSize;
	//Ŀ��ڵ���ʼ��ַ 
	IMAGE_SECTION_HEADER* Section_pos = (IMAGE_SECTION_HEADER*)(ImageBuffer + SectionOffset + 40*pos);
	
	//�жϵ�position���ڵ�ʣ��ռ��Ƿ��ܹ��������
	int ResumeBytes = Section_pos->SizeOfRawData - Section_pos->Misc.VirtualSize;
	if(ResumeBytes < sizeof(ShellCode)){
		printf("�ýڵ�ʣ��ռ䲻�㣡\n");
		return; 
	}
	
	//��ȡMessageBox�ĵ�ַ
	int MessageBoxAddr = GetMessageBoxAddress();
	printf("MessageBox��ַ=0x%x\n",MessageBoxAddr);

	//����ShellCode���ڴ��е�ƫ��
	int ShellCodeOffset = Section_pos->VirtualAddress + Section_pos->Misc.VirtualSize;
	printf("����ƫ����=0x%x\n",ShellCodeOffset);
	
	//����callָ�����ת��ַ,�޸�ShellCode
	int CallAddr = MessageBoxAddr - (ImageBase + (ShellCodeOffset + 13));
	*(int*)(ShellCode+9) = CallAddr;
	printf("callָ�����ת��ַΪ��%x\n",CallAddr);
	
	//����jmpָ�����ת��ַ,�޸�ShellCode
	int JmpAddr =  OEP - (ShellCodeOffset + 18);
	*(int*)(ShellCode+14) = JmpAddr;
	printf("jmpָ�����ת��ַΪ��%x\n",JmpAddr);
	
	//�޸� AddressOfEntryPoint,��Ϊ����չPEͷ�У�����ת�ɽṹ�� 
	*(int*)(ImageBuffer + NewDOS->e_lfanew + 4 + 20 + 16) = ShellCodeOffset;//����תint�ˣ����ֻ�޸��˵�һλ������ 
	
	//�޸Ľڵ����ԣ�ʹ����п�ִ�����ԣ� Ĭ�ϵ�һ����Ϊ�����
	//����Ҳ�����ԣ�ͬ�����޸���FileBuffer���ֵ 
//	SectionHeader[pos].Characteristics |= SectionHeader[0].Characteristics;
//	IMAGE_SECTION_HEADER* S1 = (IMAGE_SECTION_HEADER*)(ImageBuffer + SectionOffset + pos*40);
	IMAGE_SECTION_HEADER* SectionFirst = (IMAGE_SECTION_HEADER*)(ImageBuffer + SectionOffset); 
	Section_pos->Characteristics |= SectionFirst->Characteristics;

	//��ShellCodeд���ڴ� 
	memcpy(ImageBuffer+ShellCodeOffset,ShellCode,sizeof(ShellCode));
	
	return;
	
} 

char* PEUnLoad(char* ImageBuffer){
	//���½ṹ��Ӧͨ��ImageBuffer���ڴ����ң�����������ǰPE�����õ��Ľṹ�� 
	int NewSize = 0;
	int HeaderSize = 0;
	int SectionNum = 0;
	int OptionSize = 0;
	
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)ImageBuffer;
	
	if(NT32 != NULL){//x32
		IMAGE_NT_HEADERS32* NT = NULL;
		NT = (IMAGE_NT_HEADERS32*)(ImageBuffer + NewDOS->e_lfanew);//���DOS->e_lfanewҲҪ�ģ�����ӽڵ�ʱ����ܻ��޸�e_lfanew��ֱ��ʹ�ýṹ��ľͻ���� 
		HeaderSize = NT->OptionalHeader.SizeOfHeaders;
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		printf("�ļ����ڴ��еĴ�С0x%X\n",NT->OptionalHeader.SizeOfImage); 
	} 
	else{//x64
		IMAGE_NT_HEADERS64* NT = NULL;
		NT = (IMAGE_NT_HEADERS64*)(ImageBuffer + NewDOS->e_lfanew);
		HeaderSize = NT->OptionalHeader.SizeOfHeaders;
		SectionNum = NT->FileHeader.NumberOfSections;
		OptionSize = NT->FileHeader.SizeOfOptionalHeader;
		printf("�ļ����ڴ��еĴ�С0x%X\n",NT->OptionalHeader.SizeOfImage);
	}
	printf("�ڵ�����=%d\n",SectionNum);
	//���ڴ����ҵ���һ���ڱ��ַ
	IMAGE_SECTION_HEADER* SectionStart = (IMAGE_SECTION_HEADER*)(ImageBuffer + NewDOS->e_lfanew + 4 + 20 +OptionSize);
	//���ݽڱ���ʼ��ַ�ҵ����һ���ڱ� 
	IMAGE_SECTION_HEADER* LastSection = SectionStart + SectionNum - 1;
	//����ļ���С��ʵ�����ǲ�̫׼ȷ�ģ����ǲ�Ӱ������ 
	NewSize = LastSection->PointerToRawData + LastSection->SizeOfRawData;
	
	printf("���һ���ڵĶ����С=%X\n",LastSection->SizeOfRawData);
	printf("���һ�������ļ��е�ƫ����=%X\n",LastSection->PointerToRawData);
	printf("�ļ���С=%X\n",NewSize);
	//���ڴ��п�����Ӧ��С�Ŀռ� 
	char*  NewBuffer= (char*)malloc(sizeof(char)*NewSize);
	memset(NewBuffer,0,NewSize);
	//����PEȥ���� 
	//HEADERֱ���հ�
	memcpy(NewBuffer,ImageBuffer,HeaderSize);
	//�ڱ����ȥ����
	//Ҫ�޸ģ�������SectionӦ���ڴ����� 
	for(int i=0;i<SectionNum;i++){
		//��ǰ�����ļ��еĶ����Ĵ�С 
		int SizeOfRawData = (SectionStart + i)->SizeOfRawData;
		//��ǰ�����ڴ��е�ƫ�Ƶ�ַ
		int VirtualAddress = (SectionStart + i)->VirtualAddress;
		//��ǰ�����ļ��е�ƫ�Ƶ�ַ
		int PointerToRawData = (SectionStart + i)->PointerToRawData;
		
		memcpy(NewBuffer+PointerToRawData,ImageBuffer+VirtualAddress,SizeOfRawData);
	}

	return NewBuffer;
	
}

char* PELoading(char* FileBuffer){
	
	int ImageSize = 0;
	int HeaderSize = 0;
	int SectionNum = 0;
	//��ȡ�ļ����ڴ��еĴ�С
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
	
	//���ڴ��п�����Ӧ��С�Ŀռ� 
	char*  ImageBuffer= (char*)malloc(sizeof(char)*ImageSize);
	printf("\n");
	if(ImageBuffer == NULL){
		printf("PE���ص��ڴ����ʧ�ܣ�\n");
		return NULL;
	}
	
	memset(ImageBuffer,0,ImageSize);
	
	//����PE����
	//HEADERֱ���հ�
	memcpy(ImageBuffer,FileBuffer,HeaderSize);
	
	//�ڱ��������
	for(int i=0;i<SectionNum;i++){
		//��ǰ�����ļ��еĶ����Ĵ�С 
		int SizeOfRawData = SectionHeader[i]->SizeOfRawData;
		//��ǰ�����ڴ��е�ƫ�Ƶ�ַ
		int VirtualAddress = SectionHeader[i]->VirtualAddress;
		//��ǰ�����ļ��е�ƫ�Ƶ�ַ
		int PointerToRawData = SectionHeader[i]->PointerToRawData;
		//�� PointerToRawData��ʼ������SizeOfRawData��С����ImageBuffer����ʼ��ַΪVirtualAddress�ĵط� 
		memcpy(ImageBuffer+VirtualAddress,FileBuffer+PointerToRawData,SizeOfRawData);
	}
	
	return ImageBuffer;
	
}

void PEAnalyze(char* FileBuffer){
	
	char* FM_Addr = FileBuffer;
	
	//DOSͷ
	DOS = (IMAGE_DOS_HEADER*)FM_Addr;
	//NTͷ 
	FM_Addr += DOS->e_lfanew;
	//�ж���x32����x64 
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
		printf("�޷�ʶ���Ƕ���λ�ĳ���\n");
		return;
	}
	//�ڱ����
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
		printf("�ļ���ʧ�ܣ�\n");
		return NULL;	
	}
	
//	printf("�ļ��򿪳ɹ���\n");
	return fp;
		
}

int getFileSize(FILE* fp){
	
	fseek(fp,0,SEEK_END);
	int FileSize =  ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
//	printf("�ļ���СΪ��%d\n",FileSize);
	
	return FileSize;
	
}

char* ReadFileIntoMemory(FILE* fp){
	
	int BufferSize = getFileSize(fp);
	char* FileBuffer = NULL;
	FileBuffer = (char*)malloc(sizeof(char)*BufferSize);
	memset(FileBuffer,0,sizeof(FileBuffer));
	if(FileBuffer == NULL){
		printf("�ڴ����ʧ�ܣ��ļ�δ��ȡ���ڴ��У�\n");
		return NULL;
	}
	
	fread(FileBuffer,BufferSize,1,fp);
//	printf("�ڴ����ɹ����׵�ַΪ��%x\n",FileBuffer);
	
	return FileBuffer;
	
}

void WriteIntoFile(char* FileBuffer,char* FileName){
	
	FILE* fp = NULL;
	fp = fopen(FileName,"wb");
	if(fp == NULL){
		printf("�ļ���ʧ�ܣ�\n");
		return;
	}
	
//	int FileSize =  sizeof(FileBuffer);//�ҵ�ԭ���ˣ� sizeof(FileBuffer)=4��������ʹ��д���ļ�����4���ֽ� 
//	fwrite(FileBuffer,FileSize,1,fp);��ʱ��̫����д�����ļ�
	
	IMAGE_DOS_HEADER* NewDOS = (IMAGE_DOS_HEADER*)FileBuffer;
	int FileAlign = 0;//�ļ������С 
	int FileSize = 0;//�ļ���С
	int OptionSize = 0;//��ѡPEͷ�Ĵ�С 
	int SectionNum = 0;//�ڱ����� 
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
	
	//�ҵ����һ���� 
	IMAGE_SECTION_HEADER* LastSection = (IMAGE_SECTION_HEADER*)(FileBuffer + NewDOS->e_lfanew + 4 + 20 + OptionSize + 40*(SectionNum - 1));
	//�����ļ���С 
	FileSize = LastSection->PointerToRawData + LastSection->SizeOfRawData; 
	//ÿ�ζ����ļ�����Ĵ�С����ΪFileSize��ͨ�����һ���ڱ��ƫ����+�����Ĵ�С�ó��ģ������������� 
	int cnt = FileSize/FileAlign;
	for(int i=0;i<cnt;i++) {
		fwrite(FileBuffer+FileAlign*i,FileAlign,1,fp);
	}

	fclose(fp);
	
	return; 
	
}
