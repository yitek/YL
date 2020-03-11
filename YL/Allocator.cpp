#include "Allocator.h"
#include "malloc.h"
#include <string.h>


YL_Variable* YL_MM_Chunk::aquire() {
	YL_MM_ChunkBufferHeader* pBuffer = this->buffer;
	YL_MM_ChunkBufferHeader* pPrevBuffer = pBuffer;
	while (pBuffer != 0) {
		YL_MM_ChunkBufferHeader* pNextBuffer = pBuffer->nextBuffer;
		//跳过头信息，就是Unit开始的地址
		YL_Variable* pUnit = (YL_Variable*)((char*)pBuffer + sizeof(YL_MM_ChunkBufferHeader*));
		int unitSize = this->unitSize;
		for (int i = 0, j = this->maxUnitNum; i < j; i++) {
			
			//引用计数为0，表示没有人用，返回该内存
			if (pUnit->refCount == 0) {
				//清空内存
				memset(pUnit, 0, this->unitSize);
				// 引用计数+1，不再被别人占用
				pUnit->refCount++;
				//返回
				return pUnit;
			}
			//引用计数不为0，查看下一个单元
			pUnit = (YL_Variable*)((char*)pUnit + unitSize);
		}
		//找完了都没找到，全部都被占用了，看下一片内存
		pPrevBuffer = pBuffer;
		pBuffer = pNextBuffer;
	}
	//buffer链表上的都找了，没有空单元
	pBuffer = (YL_MM_ChunkBufferHeader*)malloc(this->bufferSize);
	memset(pBuffer, 0, this->bufferSize);
	pPrevBuffer->nextBuffer = pBuffer;
	
}

void YL_MM_Chunk::swap(unsigned int mark) {
	YL_MM_ChunkBufferHeader* pBuffer = this->buffer;
	while (pBuffer) {
		YL_Variable* pUnit = (YL_Variable*)((char*)pBuffer + sizeof(YL_MM_ChunkBufferHeader*));
		int unitSize = this->unitSize;
		for (int i = 0, j = this->maxUnitNum; i < j; i++) {
			if (pUnit->refCount != 0 || pUnit->refCount < mark) pUnit->refCount = 0;
			pUnit = (YL_Variable*)((char*)pUnit + unitSize);
		}
	}
}
void mark(int mask, YL_Variable* p_variable) {
	
	if (p_variable->refCount > 0) {
		//变量本身标记为引用中
		p_variable->refCount |= mask;
		if (YL_Type::isProperty(p_variable)) {

			mark(mask,((YL_Property*)p_variable)->propertyKey);
			mark(mask, ((YL_Property*)p_variable)->propertyValue);
		}
		else if (YL_Type::isObject(p_variable)) {
			YL_Property* prop = p_variable->dynamicProperty;
			while (prop) {
				mark(mask,prop);
				prop = prop->nextProperty;
			}
		}
	}
}



YL_Heap::YL_Heap() {
	//4M的栈
	this->maxStackNum = 1024 * 1024;
	this->stack = this->stackTop = (YL_Variable**)malloc(this->maxStackNum*sizeof(YL_Variable*));
	this->stackMaxPos = this->stack + this->maxStackNum;

	// 动态堆
	int chunkUnitInnerSizes[] =  { 0,2,4,8,12,16,20,24,28,32,40,48,56,64,72,80 };
	for (int i = 0; i < 16; i++) {
		this->chunkUnitInnerCounts[i] = chunkUnitInnerSizes[i];
		this->initChunk(this->chunks[i],chunkUnitInnerSizes[i]);
	}

	this->mask = 1 << (sizeof(unsigned int) * 8-1);
}

void YL_Heap::initChunk(YL_MM_Chunk& chunk, int count) {
	chunk.bufferSize = this->memeryPageSize;
	chunk.unitSize = sizeof(YL_Variable) + count + sizeof(YL_MM_Word);
	chunk.maxUnitNum = (chunk.bufferSize - sizeof(YL_MM_ChunkBufferHeader)) / chunk.unitSize;

}

YL_Variable* YL_Heap::aquire(rsize_t size) {
	int wordCount = size / (sizeof(YL_MM_Word));
	if (size % sizeof(YL_MM_Word) > 0) wordCount++;
	YL_MM_Chunk* pChunk;
	for (int i = 0; i < 15; i++) {
		if (wordCount <= this->chunkUnitInnerCounts[i]) {
			pChunk = &(this->chunks[i]);
			break;
		}
	}
	if (pChunk == 0) pChunk = &(this->chunks[15]);
	/*if (wordSize == 0) pChunk = &(this->chunks[0]);
	else if (wordSize <= 2) pChunk = &(this->chunks[1]);
	else if (wordSize <= 4) pChunk = &(this->chunks[2]);
	else if (wordSize <= 8) pChunk = &(this->chunks[3]);
	else if (wordSize <= 12) pChunk = &(this->chunks[4]);
	else if (wordSize <= 16) pChunk = &(this->chunks[5]);
	else if (wordSize <= 20) pChunk = &(this->chunks[6]);
	else if (wordSize <= 24) pChunk = &(this->chunks[7]);
	else if (wordSize <= 28) pChunk = &(this->chunks[8]);
	else if (wordSize <= 32) pChunk = &(this->chunks[9]);
	else if (wordSize <= 40) pChunk = &(this->chunks[10]);
	else if (wordSize <= 48) pChunk = &(this->chunks[11]);
	else if (wordSize <= 56) pChunk = &(this->chunks[11]);
	else if (wordSize <= 64) pChunk = &(this->chunks[12]);
	else if (wordSize <= 72) pChunk = &(this->chunks[13]);
	else if (wordSize <= 80) pChunk = &(this->chunks[14]);
	else pChunk = &(this->chunks[15]);*/

	return pChunk->aquire();
}

void YL_Heap::release(YL_Variable* p) {
	p->refCount = 0;
}

void YL_Heap::push(YL_Variable* p) {
	if (++this->stackTop != this->stackMaxPos) {
		*this->stackTop = p;
		this->stackTop++;
	}
	else {
		throw "stack overflow";
	}
}

YL_Variable* YL_Heap::pop(int num=0) {
	if (num <= 1) {
		if (this->stack !=this->stackTop) {
			this->stackTop--;
			return *this->stackTop;
		}
	}
	else {
		this->stackTop -= num;
		if (this->stackTop < this->stack) this->stackTop = this->stack;
	}

}

void YL_Heap::garbageCollect() {
	YL_Variable** p = this->stack;
	YL_Variable** end = this->stackTop;
	for (; p < end; p++) {
		mark(this->mask,*p);
	}
	for (int i = 0; i < 16; i++) {
		this->chunks[i].swap(this->mask);
	}
}