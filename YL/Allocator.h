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
	//类型指针，指向变量类型
	YL_Type* type;
	//引用计数
	unsigned int refCount;
};
//对象的内存布局
struct YL_Variable :YL_VariableHeader{
	union {
		int intValue;
		bool boolValue;
		int enumValue;
		int datetimeValue;
		long longValue;
		double doubleValue;
		//字符串，第一个就是长度
		int length;
		YL_Variable* propertyValue;
		//如果是引用类型，用这个域,表示第一个动态属性,动态属性是以key/value方式存放
		YL_Property* dynamicProperty;
	};	
	//后面就是编译好的定义过的成员列表或字符串的值
};

struct YL_Property :YL_Variable {
	YL_Variable* propertyKey;
	YL_Property* nextProperty;
	
};

struct YL_Type {
	//字段个数
	int fieldCount;
	//方法个数
	int methodCount;
	//指向堆管理的指针，可以直接用该chunk分配堆空间
	YL_MM_Chunk* chunk;
	static bool isProperty(YL_Variable* p);
	static bool isObject(YL_Variable* p);
};




//内存块的头
struct YL_MM_Chunk
{
	//单元大小
	int unitSize;
	// buffer的大小,一般来说就是一个页的大小
	int bufferSize;
	//单元个数 ,bufferSize-sizeof(ChunkBufferHeader)/bufferSize
	int maxUnitNum;
	//它是一个链表，如果不够了会再分配
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
	//栈顶指针
	YL_Variable** stackTop;
	YL_Variable** stackMaxPos;
	unsigned int mask;
public:
	YL_Heap();
	//申请堆空间
	YL_Variable* aquire(size_t size);
	//释放堆空间
	void release(YL_Variable* p);
	void push(YL_Variable* variable);
	YL_Variable* pop(int num=0);
	void garbageCollect();
private:
	void initChunk(YL_MM_Chunk& chunk, int count);
};






