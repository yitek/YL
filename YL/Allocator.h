#pragma once
#include "YL.h"
typedef int YL_MM_Word;

enum YL_EmbededTypeCodes {
	Int = 1,
	Boolean = 1 << 1,
	Enum = 1<<2,
	Long = 1<<3,
	Datetime = 1 << 4,
	Integer = Int | Long | Boolean | Enum |Datetime,
	Double = 1<<5,	
	Property = 1 << 6,
	ByValue = Integer | Double | Property,
	String = 1<<7,
	
};
struct YL_VariableHeader{
	//����ָ�룬ָ���������
	YL_Type* type;
	//���ü���
	unsigned int refCount;
};
//������ڴ沼��
struct YL_Variable :YL_VariableHeader{
	union {
		int intValue;
		bool boolValue;
		int enumValue;
		int datetimeValue;
		long longValue;
		double doubleValue;
		//�ַ�������һ�����ǳ���
		int length;
		YL_Variable* propertyValue;
		//������������ͣ��������,��ʾ��һ����̬����,��̬��������key/value��ʽ���
		YL_Property* dynamicProperty;
	};	
	//������Ǳ���õĶ�����ĳ�Ա�б���ַ�����ֵ
};

struct YL_Property :YL_Variable {
	YL_Variable* propertyKey;
	YL_Property* nextProperty;
	
};

struct YL_Type {
	//�ֶθ���
	int fieldCount;
	//��������
	int methodCount;
	//ָ��ѹ����ָ�룬����ֱ���ø�chunk����ѿռ�
	YL_MM_Chunk* chunk;
	static bool isProperty(YL_Variable* p);
	static bool isObject(YL_Variable* p);
};




//�ڴ���ͷ
struct YL_MM_Chunk
{
	//��Ԫ��С
	int unitSize;
	// buffer�Ĵ�С,һ����˵����һ��ҳ�Ĵ�С
	int bufferSize;
	//��Ԫ���� ,bufferSize-sizeof(ChunkBufferHeader)/bufferSize
	int maxUnitNum;
	//����һ��������������˻��ٷ���
	YL_MM_ChunkBufferHeader* buffer;
	YL_Variable* aquire();
	void swap(unsigned int mark);

private:
	
	void swap();
};

struct YL_MM_ChunkBufferHeader {
	YL_MM_ChunkBufferHeader* nextBuffer;
};


class YL_Heap {
	size_t memeryPageSize;
	YL_MM_Chunk chunks[16];
	int chunkUnitInnerCounts[16];
	int maxStackNum;
	
	YL_Variable** stack;
	//ջ��ָ��
	YL_Variable** stackTop;
	YL_Variable** stackMaxPos;
	unsigned int mask;
public:
	YL_Heap();
	//����ѿռ�
	YL_Variable* aquire(size_t size);
	//�ͷŶѿռ�
	void release(YL_Variable* p);
	void push(YL_Variable* variable);
	YL_Variable* pop(int num=0);
	void garbageCollect();
private:
	void initChunk(YL_MM_Chunk& chunk, int count);
};






