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
