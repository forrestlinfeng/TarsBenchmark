/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */
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
