#include "GameData.h"
#include <wchar.h>
#include "Xml.h"

#ifdef ANDROID
bool GameData::load(const char* path, AAssetManager* assman)
#else
bool GameData::load(const char* path)
#endif
{
    Xml datafile;

#ifdef ANDROID
    if (!datafile.load(path, assman))
#else
        if (!datafile.load(path))
#endif
    {
        return false;
    }

    XmlNode* main = datafile.root.getNode(L"games");

    char buf[256];

    if (main)
    {
        for (unsigned i = 0; i < main->childrenCount(); ++i)
        {
            XmlNode* game = main->getNode(i);

            GameDescription gd;

            if (game)
            {
                gd.effect1[0] = 0;
                gd.description1[0] = 0;

                for (unsigned a = 0; a < game->childrenCount(); ++a)
                {
                    XmlNode* node = game->getNode(a);


                    if (wcscmp(node->getName(), L"name") == 0)
                    {
                        const wchar_t * val = node->getValue();

                        if (val)
                        {
                            sprintf(gd.name, "%ls", val);
                        }
                    }
                    else if (wcscmp(node->getName(), L"description") == 0)
                    {
                        const wchar_t * val = node->getValue();

                        if (val)
                        {
                            sprintf(gd.description, "%ls", node->getValue());
                        }

                    }
                    else if (wcscmp(node->getName(), L"description1") == 0)
                    {
                        const wchar_t * val = node->getValue();

                        if (val)
                        {
                            sprintf(gd.description1, "%ls", node->getValue());
                        }

                    }
                    else if (wcscmp(node->getName(), L"effect") == 0)
                    {

                        const wchar_t * val = node->getValue();

                        if (val)
                        {
                            sprintf(gd.effect, "%ls", val);
                        }
                    }
                    else if (wcscmp(node->getName(), L"effect1") == 0)
                    {

                        const wchar_t * val = node->getValue();

                        if (val)
                        {
                            sprintf(gd.effect1, "%ls", val);
                        }
                    }
                    else if (wcscmp(node->getName(), L"skins") == 0)
                    {

                        for (unsigned j = 0; j < node->childrenCount(); ++j)
                        {
                            XmlNode* skin = node->getNode(j);
                            sprintf(buf, "%ls", skin->getValue());
                            gd.skins[j] = atoi(buf);
                        }
                    }


                }

                games.add(gd);

            }
        }
    }

    datafile.destroy();

    return true;

}

GameDescription* GameData::getGame(unsigned index)
{
    if (index >= games.count())
    {
        return nullptr;
    }

    return &games[index];
}

