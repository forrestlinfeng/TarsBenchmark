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
#include "DyncObject.h"
#include <stdio.h>
namespace bm
{
    //执行期对象全局链表初始节点
    RuntimeObject DyncObject::classDyncObject = {"DyncObject", NULL, NULL};

    static RuntimeObject* __pFirstCmdClass = &DyncObject::classDyncObject; 

    ObjectList::ObjectList(RuntimeObject *pNewClass)
    {
        assert(pNewClass);
        pNewClass->_pNextClass = __pFirstCmdClass;
        __pFirstCmdClass = pNewClass;
    }

    DyncObject* TCreateObject(const char* class_name)
    {
        RuntimeObject *pHead = __pFirstCmdClass;
        while(pHead)
        {
            if (!strcmp(pHead->_szClassName, class_name))
            {
                if (pHead->_pfnCreateObject)
                {
                    return (*(pHead->_pfnCreateObject))();
                }
            }
            pHead = pHead->_pNextClass;
        }
        return NULL;
    }
};
