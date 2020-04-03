/**************************************************************************
* Copyright (c) 2012-2022, Tencent Tech. Co., Ltd. All rights reserved.
*
* File name:          ProtoFactory.h
* Author:             linfengchen    
* Version:            1.0  
* Date:               2016.11.17
* Description:        协议代理工厂，实现协议自动发现
* History:    
*                     2016.11.17             首次创建
**************************************************************************/
#ifndef _PROTO_FACTORY_H_
#define _PROTO_FACTORY_H_

#include <map>
#include <stdexcept>
#include "Protocol.h"
using namespace std;

namespace bm
{
    class ProtoFactory
    {
    public:
        ProtoFactory() { _protos.clear(); }
        virtual ~ProtoFactory() { destroyObject(); }

        Protocol* get(const string& name, int argc, char** argv)
        {
            Protocol* cmd = NULL;
            if (_protos.find(name) == _protos.end())
            {
                DyncObject* obj = TCreateObject(name.c_str());
                if(!obj)
                {
                    throw runtime_error("Command not existed:" + name);
                }

                obj->setName(name);
                if ((cmd = dynamic_cast<Protocol*>(obj)) == NULL)
                {
                    throw runtime_error("NULL command:" + name);
                }

                cmd->initialize(argc, argv);
                _protos[name] = obj;
            }

            if ((cmd = dynamic_cast<Protocol*>(_protos[name])) == NULL)
            {
                throw runtime_error("null command:" + name);
            }

            return cmd;
        }

        void destroyObject(void)
        {
            for (map<string, DyncObject*>::iterator itm =  _protos.begin(); itm != _protos.end(); )
            {
                delete itm->second;
                _protos.erase(itm++);    
            }
        }

    private:
        map<string, DyncObject*>    _protos;
    };
};
/////////////////////////////////////////////////////////////////
#endif
