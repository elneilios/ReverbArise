#pragma once
#include <Core/ContentHash.h>
#include <Capture/LoopCaptureState.h>

namespace m3::storage
{
// Host state contains exact accepted metadata. Absolute locations remain local,
// non-authoritative registry hints; a saved writer never grants a new lease.
inline constexpr int associationVersion=1,associatedEnvelopeVersion=9;
struct RecoveryLocator
{
    juce::String store,snapshot,writer,registration,project,instance,reference;
    uint64_t revision=0;
    bool operator==(const RecoveryLocator&)const=default;
};
inline bool recoveryReference(const juce::String& value)
{
    return value.startsWith("manifests/") && value.endsWith(".json") && value.length()<=192 && !value.contains("..")
        && value.substring(10).isNotEmpty() && value.substring(10).containsOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_. ");
}
inline bool recoveryLocatorValid(const RecoveryLocator& locator)
{
    const auto uuid=[](const juce::String& s){return s.length()==36 && !juce::Uuid(s).isNull() && juce::Uuid(s).toDashedString()==s;};
    const auto hash=[](const juce::String& s){return s.length()==64 && s.containsOnly("0123456789abcdef");};
    return uuid(locator.project) && uuid(locator.instance) && locator.revision<=static_cast<uint64_t>(INT64_MAX)
        && (locator.reference.isEmpty()?locator.revision==0:recoveryReference(locator.reference))
        && (locator.store.isEmpty()?(locator.snapshot.isEmpty() && locator.writer.isEmpty() && locator.registration.isEmpty())
            :(uuid(locator.store) && hash(locator.snapshot) && uuid(locator.writer) && uuid(locator.registration)));
}

struct Association
{
    juce::String store,snapshot,writer,registration;
    juce::String recoveryRecord;std::vector<RecoveryLocator> recoveryOrigins;
    bool operator==(const Association&)const=default;
    bool present()const{return store.isNotEmpty();}
};
inline bool associationUuid(const juce::String& s)
{return s.length()==36 && !juce::Uuid(s).isNull() && juce::Uuid(s).toDashedString()==s;}
inline bool associationValid(const Association& a)
{return associationUuid(a.store) && associationUuid(a.writer) && associationUuid(a.registration)
        && a.snapshot.length()==64 && a.snapshot.containsOnly("0123456789abcdef")
    && (a.recoveryRecord.isEmpty()?a.recoveryOrigins.empty():a.recoveryRecord.length()==64 && a.recoveryRecord.containsOnly("0123456789abcdef")
        && !a.recoveryOrigins.empty() && a.recoveryOrigins.size()<=16 && std::all_of(a.recoveryOrigins.begin(),a.recoveryOrigins.end(),recoveryLocatorValid));}
inline bool associationXmlShape(const juce::String& value)
{
    if(value.getNumBytesAsUTF8()>97*1024*1024 || value.containsIgnoreCase("<!"))return false;
    int depth=0,tags=0,attributes=0;auto cursor=value.getCharPointer();
    while(!cursor.isEmpty()){
        if(cursor.getAndAdvance()!='<')continue;
        if(*cursor=='?'){bool closed=false;while(!cursor.isEmpty()){if(cursor.getAndAdvance()=='?' && *cursor=='>'){++cursor;closed=true;break;}}if(!closed)return false;continue;}
        const bool closing=*cursor=='/';juce::juce_wchar quote=0,previous=0;bool ended=false;
        while(!cursor.isEmpty()){const auto c=cursor.getAndAdvance();if(quote){if(c==quote)quote=0;continue;}if(c=='\'' || c=='"'){quote=c;continue;}
            if(c=='<')return false;if(c=='=' && ++attributes>256)return false;if(c=='>'){if(++tags>64)return false;if(closing){if(--depth<0)return false;}else{if(++depth>8)return false;if(previous=='/')--depth;}ended=true;break;}previous=c;}
        if(!ended)return false;
    }
    return depth==0;
}
inline bool associationJsonShape(const juce::String& value)
{
    if(value.getNumBytesAsUTF8()>8*1024*1024)return false;
    int depth=0;bool quoted=false,escaped=false;
    for(auto cursor=value.getCharPointer();!cursor.isEmpty();){const auto c=cursor.getAndAdvance();
        if(quoted){if(escaped)escaped=false;else if(c=='\\')escaped=true;else if(c=='"')quoted=false;continue;}
        if(c=='"')quoted=true;else if(c=='{' || c=='['){if(++depth>16)return false;}else if(c=='}' || c==']'){if(--depth<0)return false;}
    }
    return depth==0 && !quoted;
}
inline juce::String associationSnapshot(const juce::String& legacy,const juce::String& manifest)
{
    ContentHash hash;const auto domain=juce::String("ReverbArise working metadata v1\n");
    for(const auto& text:{domain,legacy,juce::String("\n"),manifest})hash.update(text.toRawUTF8(),text.getNumBytesAsUTF8());return hash.finish();
}
inline bool associatedStateChildrenValid(const juce::XmlElement& base)
{
    bool sawSession=false;
    for(const auto* child:base.getChildIterator())
    {
        if(child->hasTagName("SwellSession"))
        {
            if(sawSession)return false;
            sawSession=true;
        }
        else if(!child->hasTagName("NextCapture") && !child->hasTagName("AcceptedLoopSource"))
            return false;
    }
    auto kind=CaptureSourceKind::song;
    LoopCapturePreference preference;
    return sawSession && readLoopState(base,kind,preference);
}
inline bool encodeAssociatedState(const juce::String& legacy,const juce::String& manifest,Association& association,juce::String& output,juce::String& error)
{
    if(!associationXmlShape(legacy)){error="Malformed or oversized accepted metadata";return false;}
    auto xml=juce::XmlDocument::parse(legacy);
    if(!xml || !xml->hasTagName("PreverbState") || xml->getStringAttribute("stateVersion")!="8" || legacy.getNumBytesAsUTF8()>49*1024*1024
        || !associatedStateChildrenValid(*xml) || manifest.getNumBytesAsUTF8()>8*1024*1024)
    {error="Invalid accepted metadata for folder association";return false;}
    auto next=association;next.snapshot=associationSnapshot(xml->toString(),manifest);
    if(!associationValid(next)){error="Invalid working-folder identity";return false;}
    xml->setAttribute("stateVersion",associatedEnvelopeVersion);auto* node=xml->createNewChildElement("WorkingFolder");
    node->setAttribute("version",associationVersion);node->setAttribute("store",next.store);node->setAttribute("snapshot",next.snapshot);
    node->setAttribute("writer",next.writer);node->setAttribute("registration",next.registration);
    node->createNewChildElement("CaptureManifest")->addTextElement(manifest);
    if(next.recoveryRecord.isNotEmpty()){
        node->setAttribute("recovery",next.recoveryRecord);auto* origins=node->createNewChildElement("RecoveryOrigins");origins->setAttribute("version",1);
        for(const auto& saved:next.recoveryOrigins){auto* origin=origins->createNewChildElement("Locator");
            origin->setAttribute("store",saved.store);origin->setAttribute("snapshot",saved.snapshot);origin->setAttribute("writer",saved.writer);origin->setAttribute("registration",saved.registration);
            origin->setAttribute("project",saved.project);origin->setAttribute("instance",saved.instance);origin->setAttribute("reference",saved.reference);origin->setAttribute("revision",juce::String(static_cast<juce::int64>(saved.revision)));}
    }
    const auto encoded=xml->toString();if(!associationXmlShape(encoded)){error="Oversized associated metadata";return false;}
    output=encoded;association=std::move(next);error.clear();return true;
}
inline bool decodeAssociatedState(const juce::String& text,Association& association,juce::String& legacy,juce::String& manifest,juce::String& error)
{
    if(!associationXmlShape(text)){error="Malformed or oversized associated metadata";return false;}
    auto xml=juce::XmlDocument::parse(text);
    if(!xml || !xml->hasTagName("PreverbState") || xml->getStringAttribute("stateVersion")!=juce::String(associatedEnvelopeVersion))
    {error="Unsupported associated processor envelope";return false;}
    auto* node=xml->getChildByName("WorkingFolder");
    if(!node || node->getNumAttributes()!=(node->hasAttribute("recovery")?6:5)
        || node->getStringAttribute("version")!="1" || node->getNumChildElements()!=(node->hasAttribute("recovery")?2:1)
        || !node->getChildByName("CaptureManifest"))
    {error="Malformed working-folder metadata";return false;}
    const auto* payload=node->getChildByName("CaptureManifest");
    if(payload->getNumAttributes()!=0 || payload->getNumChildElements()>1
        || (payload->getFirstChildElement() && !payload->getFirstChildElement()->isTextElement()))
    {error="Unsupported capture payload structure";return false;}
    Association next{node->getStringAttribute("store"),node->getStringAttribute("snapshot"),node->getStringAttribute("writer"),node->getStringAttribute("registration")};
    next.recoveryRecord=node->getStringAttribute("recovery");
    if(node->hasAttribute("recovery")){
        const auto* origins=node->getChildByName("RecoveryOrigins");
        if(!origins || origins->getStringAttribute("version")!="1" || origins->getNumAttributes()!=1 || origins->getNumChildElements()<1 || origins->getNumChildElements()>16){error="Invalid recovery-origin metadata";return false;}
        for(const auto* saved:origins->getChildIterator()){
            if(!saved->hasTagName("Locator") || saved->getNumAttributes()!=8 || saved->getNumChildElements()!=0){error="Malformed recovery locator";return false;}
            const auto revision=saved->getStringAttribute("revision");const auto number=revision.getLargeIntValue();
            RecoveryLocator origin{saved->getStringAttribute("store"),saved->getStringAttribute("snapshot"),saved->getStringAttribute("writer"),saved->getStringAttribute("registration"),
                saved->getStringAttribute("project"),saved->getStringAttribute("instance"),saved->getStringAttribute("reference"),static_cast<uint64_t>(number)};
            if(number<0 || juce::String(number)!=revision || !recoveryLocatorValid(origin)){error="Invalid saved recovery locator";return false;}
            next.recoveryOrigins.push_back(std::move(origin));
        }
    }
    const auto captured=node->getChildByName("CaptureManifest")->getAllSubText();
    if(!associationValid(next) || !associationJsonShape(captured))
    {error="Unsupported or oversized working-folder identity";return false;}
    xml->removeChildElement(node,true);xml->setAttribute("stateVersion",8);
    if(!associatedStateChildrenValid(*xml)){error="Malformed associated capture or session metadata";return false;}
    const auto base=xml->toString();
    if(associationSnapshot(base,captured)!=next.snapshot){error="Working snapshot does not match accepted metadata";return false;}
    association=std::move(next);legacy=base;manifest=captured;error.clear();return true;
}
}
