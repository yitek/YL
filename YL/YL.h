#pragma once


//�������ڴ��еĲ���
struct  stut_YL_Variable
{
	//����ָ��
	void* pType;
	union {
		int value;
		float fValue;
		double dValue;
		bool bValue;
		wchar_t charValue;
		struct {
			//ָ������ָ��
			void* pObject;
			//���ü���
			int32_t refCount;
			//���������õ��ֶ�
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


