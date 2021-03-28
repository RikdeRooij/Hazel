#include "TextureAtlas.h"
#include "../vendor/tinyxml2/tinyxml2.h"
#include "Globals.h"
#include "glm/detail/_noise.hpp"

using namespace Jelly;

TextureAtlas::TextureAtlas()
{}

TextureAtlas::TextureAtlas(std::string path)
{
    this->texure = LoadXML(path, this->rects);
}

TextureAtlas::TextureAtlas(const std::shared_ptr<Hazel::Texture2D>& x)
{
    this->texure = x;
    glm::vec4 rect(0, 0, 1, 1);
    this->rects.push_back(rect);
}

void TextureAtlas::Load(std::string path)
{
    this->texure = LoadXML(path, this->rects);
}

TextureRef TextureAtlas::LoadXML(std::string path, std::vector<glm::vec4>& rects)
{
    tinyxml2::XMLDocument doc;

    doc.LoadFile(path.c_str());
    if (doc.Error())
    {
        DBG_OUTPUT(doc.GetErrorStr1());
        DBG_OUTPUT(doc.GetErrorStr2());
        return TextureRef();
    }

    const tinyxml2::XMLElement* element = doc.FirstChildElement("TextureAtlas");

    if (!element)
    {
        DBG_OUTPUT("element == NULL");
        return TextureRef();
    }

    std::string imagepath = std::string("assets/") + element->Attribute("imagePath");

    auto itex = Hazel::Texture2D::Create(imagepath);

    float width = (float)element->IntAttribute("width");
    float height = (float)element->IntAttribute("height");

    //rects.clear();
    int count = 0;
    for (const tinyxml2::XMLElement* sprite = element->FirstChildElement("sprite");
         sprite; sprite = sprite->NextSiblingElement("sprite"))
    {
        ++count;
        //DBG_OUTPUT("%s %s", sprite->Value(), sprite->Attribute("n"));

        glm::vec4 rect(
            sprite->IntAttribute("x") / width,
            sprite->IntAttribute("y") / height,
            sprite->IntAttribute("w") / width,
            sprite->IntAttribute("h") / height);

        rect.y = 1 - rect.w - rect.y;

        //frameAnim.addFrame(1.f, iREct);
        rects.push_back(rect);
    }

    DBG_WRITE("COUNT: %d", count);

    return itex;
}

const glm::vec4 & TextureAtlas::AnimationRect(const float& t, const int& frame_start, const int& frame_length, const bool& pingpong) const
{
    uint indx = AnimTimeIndex(t, abs(frame_length), pingpong);
    indx = frame_start + (sign(frame_length) * indx);
    return GetRect(indx);
}

