#pragma once


//变量在内存中的布局
struct  stut_YL_Variable
{
	//类型指针
	void* pType;
	union {
		int value;
		float fValue;
		double dValue;
		bool bValue;
		wchar_t charValue;
		struct {
			//指向对象的指针
			void* pObject;
			//引用计数
			int32_t refCount;
			//垃圾回收用的字段
			int32_t GCInfo;
		};
		 
	};
	stut_YL_Variable operator =(stut_YL_Variable& rightValue) {
		int* pRefCount = (int*)rightValue.pObject;
		(*pRefCount)++;
		return rightValue;
	}
	~stut_YL_Variable() {
		int* pRefCount = (int*)pObject;
		(*pRefCount)--;
	}
};
/*
 a=b; b->refBy = a;
 c=b; b
*/


