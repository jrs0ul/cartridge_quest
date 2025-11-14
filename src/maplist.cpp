#include <cstdio>
#include <cstring>
#include <cwchar>
#include "maplist.h"
#include "Xml.h"

//----------------------
#ifdef ANDROID
MapList::MapList(AAssetManager* assman)
#else
MapList::MapList()
#endif
{
    Xml mapList;
#ifdef ANDROID
    bool res = mapList.load("maps/list.xml", assman);
#else
    bool res = mapList.load("maps/list.xml");
#endif

    if (res)
    {
        XmlNode *mainnode = mapList.root.getNode(L"Maps");

        _count = 0;

        if (mainnode)
        {
            _count = mainnode->childrenCount();
            if (_count)
            {
                maps = new char*[_count];
            }
            else
            {
                return;
            }
        }

        for (int i = 0; i < _count; ++i)
        {
            XmlNode *node = mainnode->getNode(i);

            if (node)
            {
                for (int j = 0; j < (int)node->attributeCount(); ++j)
                {
                    XmlAttribute* attr = node->getAttribute(j);

                    if (attr)
                    {
                        if (wcscmp(attr->getName(), L"name") == 0)
                        {
                            wchar_t* val = attr->getValue();

                            if (val)
                            {
                                maps[i] = new char[255];
                                sprintf(maps[i], "%ls", val);
                            }
                        }
                    }
                }
            }
        }
    }

    mapList.destroy();


    current = 0;
}

void MapList::Destroy()
{
    if (_count)
    {
        for (int i = 0; i < _count; ++i)
        {
            if (maps[i])
            {
                delete []maps[i];
            }
        }

        delete []maps;
        maps = 0;

    }

}

void MapList::getMapName(int i, char*name)
{
    if ((i >= 0) && (i < _count))
    {
        strcpy(name,maps[i]);
    }
    else
    {
        name=0;
    }
}
