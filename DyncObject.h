/**************************************************************************
* Copyright (c) 2012-2022, Tencent Tech. Co., Ltd. All rights reserved.
*
* File name:          DyncObject.h
* Author:             linfengchen    
* Version:            1.0  
* Date:               2016.11.17
* Description:        动态协议基类
* History:    
*                     2016.11.17             首次创建
**************************************************************************/
#ifndef _DYNC_OBJECT_H_
#define _DYNC_OBJECT_H_

#include <iostream>
#include <assert.h>
#include <string>
#include <cstring>
using namespace std;

namespace bm
{
/**
 * 精简版动态创建:
 * 
 */
//.虚类声明
#define DECLARE_PROTODYNAMIC(class_name) \
public: \
	static RuntimeObject class##class_name;

//.虚类定义
#define IMPLEMENT_PROTODYNAMIC(class_name) \
	RuntimeObject class_name::class##class_name = {#class_name, NULL, NULL}; \
	ObjectList __initcmd_##class_name(&class_name::class##class_name);

//.实类声明
#define DECLARE_PROTODYNCREATE(class_name) \
public: \
	static RuntimeObject class##class_name; \
	static DyncObject* CreateObject(); 

//.实类定义
#define IMPLEMENT_PROTODYNCREATE(class_name) \
	RuntimeObject class_name::class##class_name = { #class_name, class_name::CreateObject, NULL}; \
	ObjectList __initcmd_##class_name(&class_name::class##class_name); \
	DyncObject* class_name::CreateObject(void) { return new class_name; }

struct DyncObject;

/**
 * 执行期对象结构
 */
struct RuntimeObject
{
	const char* _szClassName;

	DyncObject* (*_pfnCreateObject)(void);

	RuntimeObject* _pNextClass;
};

/**
 * 执行期对象
 */
struct DyncObject
{ 
public:
    virtual ~DyncObject() {};

	static RuntimeObject classDyncObject; 

	void setName(const string& name) { _className = name; }

	const std::string& name() const { return _className; }

private:
	std::string _className;
};

/**
 * 构建执行期对象全局链
 */
struct ObjectList 
{ 
	ObjectList(RuntimeObject *pNewClass); 
};

/**
 * 执行期对象创建接口
 */
DyncObject* TCreateObject(const char* class_name);
};
/////////////////////////////////////////////////////////
#endif
