#pragma once
#include <Core/ContentHash.h>

namespace m3::storage
{
// Host state contains exact accepted metadata. Absolute locations remain local,
// non-authoritative registry hints; a saved writer never grants a new lease.
inline constexpr int associationVersion=1,associatedEnvelopeVersion=9;
struct Association
{
    juce::String store,snapshot,writer,registration;
    bool operator==(const Association&)const=default;
    bool present()const{return store.isNotEmpty();}
};
inline bool associationUuid(const juce::String& s)
{return s.length()==36 && !juce::Uuid(s).isNull() && juce::Uuid(s).toDashedString()==s;}
inline bool associationValid(const Association& a)
{return associationUuid(a.store) && associationUuid(a.writer) && associationUuid(a.registration)
    && a.snapshot.length()==64 && a.snapshot.containsOnly("0123456789abcdef");}
inline bool associationXmlShape(const juce::String& value)
{
    if(value.getNumBytesAsUTF8()>49*1024*1024 || value.containsIgnoreCase("<!"))return false;
    int depth=0,tags=0,attributes=0;auto cursor=value.getCharPointer();
    while(!cursor.isEmpty()){
        if(cursor.getAndAdvance()!='<')continue;
        if(*cursor=='?'){bool closed=false;while(!cursor.isEmpty()){if(cursor.getAndAdvance()=='?' && *cursor=='>'){++cursor;closed=true;break;}}if(!closed)return false;continue;}
        const bool closing=*cursor=='/';juce::juce_wchar quote=0,previous=0;bool ended=false;
        while(!cursor.isEmpty()){const auto c=cursor.getAndAdvance();if(quote){if(c==quote)quote=0;continue;}if(c=='\'' || c=='"'){quote=c;continue;}
            if(c=='<')return false;if(c=='=' && ++attributes>128)return false;if(c=='>'){if(++tags>64)return false;if(closing){if(--depth<0)return false;}else{if(++depth>8)return false;if(previous=='/')--depth;}ended=true;break;}previous=c;}
        if(!ended)return false;
    }
    return depth==0;
}
inline juce::String associationSnapshot(const juce::String& legacy,const juce::String& manifest)
{
    ContentHash hash;const auto domain=juce::String("ReverbArise working metadata v1\n");
    for(const auto& text:{domain,legacy,juce::String("\n"),manifest})hash.update(text.toRawUTF8(),text.getNumBytesAsUTF8());return hash.finish();
}
inline bool encodeAssociatedState(const juce::String& legacy,const juce::String& manifest,Association& association,juce::String& output,juce::String& error)
{
    if(!associationXmlShape(legacy)){error="Malformed or oversized accepted metadata";return false;}
    auto xml=juce::XmlDocument::parse(legacy);
    if(!xml || !xml->hasTagName("PreverbState") || xml->getStringAttribute("stateVersion")!="8"
        || xml->getChildByName("WorkingFolder") || manifest.getNumBytesAsUTF8()>8*1024*1024)
    {error="Invalid accepted metadata for folder association";return false;}
    auto next=association;next.snapshot=associationSnapshot(xml->toString(),manifest);
    if(!associationValid(next)){error="Invalid working-folder identity";return false;}
    xml->setAttribute("stateVersion",associatedEnvelopeVersion);auto* node=xml->createNewChildElement("WorkingFolder");
    node->setAttribute("version",associationVersion);node->setAttribute("store",next.store);node->setAttribute("snapshot",next.snapshot);
    node->setAttribute("writer",next.writer);node->setAttribute("registration",next.registration);
    node->createNewChildElement("CaptureManifest")->addTextElement(manifest);
    output=xml->toString();association=std::move(next);error.clear();return true;
}
inline bool decodeAssociatedState(const juce::String& text,Association& association,juce::String& legacy,juce::String& manifest,juce::String& error)
{
    if(!associationXmlShape(text)){error="Malformed or oversized associated metadata";return false;}
    auto xml=juce::XmlDocument::parse(text);
    if(!xml || !xml->hasTagName("PreverbState") || xml->getStringAttribute("stateVersion")!=juce::String(associatedEnvelopeVersion))
    {error="Unsupported associated processor envelope";return false;}
    auto* node=xml->getChildByName("WorkingFolder");
    if(!node || xml->getNumChildElements()!=2 || !xml->getChildByName("SwellSession") || node->getNumAttributes()!=5
        || node->getStringAttribute("version")!="1" || node->getNumChildElements()!=1
        || !node->getChildByName("CaptureManifest"))
    {error="Malformed working-folder metadata";return false;}
    const auto* payload=node->getChildByName("CaptureManifest");
    if(payload->getNumAttributes()!=0 || payload->getNumChildElements()>1
        || (payload->getFirstChildElement() && !payload->getFirstChildElement()->isTextElement()))
    {error="Unsupported capture payload structure";return false;}
    Association next{node->getStringAttribute("store"),node->getStringAttribute("snapshot"),node->getStringAttribute("writer"),node->getStringAttribute("registration")};
    const auto captured=node->getChildByName("CaptureManifest")->getAllSubText();
    if(!associationValid(next) || captured.getNumBytesAsUTF8()>8*1024*1024)
    {error="Unsupported or oversized working-folder identity";return false;}
    xml->removeChildElement(node,true);xml->setAttribute("stateVersion",8);const auto base=xml->toString();
    if(associationSnapshot(base,captured)!=next.snapshot){error="Working snapshot does not match accepted metadata";return false;}
    association=std::move(next);legacy=base;manifest=captured;error.clear();return true;
}
}
