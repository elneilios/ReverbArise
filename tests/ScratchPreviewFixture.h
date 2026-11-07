#pragma once

#include "PortableFixture.h"
#include <Storage/PortableAssets.h>

struct ScratchPreviewFixture
{
    Fixture original;
    TestDirectory store;
    juce::String operation,path,storeId,sha;
    uint64_t bytes=0;

    ScratchPreviewFixture()
    {
        using namespace m3::storage::portable;
        REQUIRE(store.file.createDirectory().wasOk());
        juce::String error;
        std::shared_ptr<Prepared> prepared;
        REQUIRE(prepare({original.root,original.xml,original.manifest},prepared,error));
        bool stop=false;
        Hooks hooks;
        hooks.cancelled=[&]{return stop;};
        hooks.checkpoint=[&](const auto& point,const auto&){if(point=="sealed-payload")stop=true;return true;};
        const auto result=backup(prepared,store.file,hooks);
        REQUIRE(result.cancelled && !result.success);
        operation=result.operation;
        const auto proof=juce::JSON::parse(journal().loadFileAsString());
        REQUIRE(proof.isObject());
        path="transactions/"+operation+"/staging/"+proof["part"].toString();
        storeId=proof["store"].toString();
        sha=proof["sha256"].toString();
        bytes=static_cast<uint64_t>(store.file.getChildFile(path).getSize());
        REQUIRE(bytes>0 && m3::fileHash(store.file.getChildFile(path))==sha);
    }

    juce::File journal()const{return store.file.getChildFile("transactions/"+operation+"/scratch-cancelled.json");}
};
