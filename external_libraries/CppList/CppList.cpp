//#include "StdAfx.h"
#include "CppList.h"
#include <stdlib.h>
#include <string.h>

CppList::CppList()
{
	_count = 0;
	_maxCount = 1;
	_arr = (void**)malloc(_maxCount*sizeof(void*));
	memset(_arr,0,_maxCount*sizeof(void*));
}


CppList::~CppList()
{
	free(_arr);
}

void CppList::Add(void *item){
	if (!Exists(item)){
		int orig_count = _count;
		int next_count = _count + 1;
		int next_index = next_count - 1;
		PrepareArray(next_count);
		_arr[next_index] = item;
		_count = next_count;
	}
}
void CppList::PrepareArray(int nextCount){
	if (nextCount > _maxCount){
		int new_max_count = _maxCount*2;
		void **old_arr = _arr;
		void **new_arr = (void**)malloc(new_max_count*sizeof(void*));
		memset(new_arr,0,new_max_count*sizeof(void*));
		memcpy(new_arr,old_arr,_maxCount*sizeof(void*));
		free(old_arr);
		_maxCount = new_max_count;
		_arr = new_arr;
	}
}
void CppList::Delete(void *item){
	int idx = GetIndex(item);
	DeleteAt(idx);
}
void CppList::DeleteAt(int index){
	if (index < 0)
		return;
	if (index >= _count)
		return;

	void **tmp_arr = (void**)malloc(_count*sizeof(void*));
	memcpy(tmp_arr,_arr,_count*sizeof(void*));

	//clear
	memset(_arr,0,_count*sizeof(void*));
	// copy start
	memcpy(_arr,tmp_arr,index*sizeof(void*));
	// copy end
	memcpy(_arr + index,tmp_arr + (index + 1),(_count - index - 1)*sizeof(void*));

	free(tmp_arr);

	_count--;
}
int CppList::GetCount(){
	return _count;
}
void* CppList::GetItem(int index){
	if (index < 0)
		return NULL;
	if (index >= _count)
		return NULL;
	return _arr[index];
}
bool CppList::Exists(void *item){
	int idx = GetIndex(item);
	return idx != -1;
}
int CppList::GetIndex(void *item){
	int found_idx = -1;
	for(int i=0;i<_count;++i){
		if (item == _arr[i]){
			found_idx = i;
			break;
		}
	}
	return found_idx;
}