#include "ContentHash.h"
#define NOMINMAX
#include <Windows.h>
#include <bcrypt.h>
#include <array>
#include <cmath>
#include <stdexcept>

namespace m3
{
ContentHash::ContentHash()
{
    BCRYPT_ALG_HANDLE alg = nullptr;
    if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0) throw std::runtime_error("SHA-256 initialization failed");
    algorithm = alg;
    ULONG length=0, written=0;
    if (BCryptGetProperty(alg, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&length), sizeof(length), &written, 0) < 0)
    { BCryptCloseAlgorithmProvider(alg,0); algorithm=nullptr; throw std::runtime_error("SHA-256 properties failed"); }
    scratch.resize(length); BCRYPT_HASH_HANDLE h=nullptr;
    if (BCryptCreateHash(alg,&h,scratch.data(),length,nullptr,0,0) < 0)
    { BCryptCloseAlgorithmProvider(alg,0); algorithm=nullptr; throw std::runtime_error("SHA-256 creation failed"); }
    hash=h;
}
ContentHash::~ContentHash() { if (hash) BCryptDestroyHash(hash); if (algorithm) BCryptCloseAlgorithmProvider(algorithm,0); }
void ContentHash::update(const void* data, size_t bytes)
{
    auto p=static_cast<const unsigned char*>(data);
    while (bytes)
    {
        const auto count=static_cast<ULONG>(std::min(bytes,size_t{65536}));
        if (BCryptHashData(hash,const_cast<PUCHAR>(p),count,0)<0) throw std::runtime_error("SHA-256 update failed");
        p+=count; bytes-=count;
    }
}
void ContentHash::samples(const float* input, size_t count)
{
    std::array<float,4096> normalized;
    while (count)
    {
        const auto n=std::min(count,normalized.size());
        for (size_t i=0;i<n;++i)
        { if (! std::isfinite(input[i])) throw std::runtime_error("Non-finite sample in content identity"); normalized[i]=input[i]==0 ? 0.0f : input[i]; }
        update(normalized.data(),n*sizeof(float)); input+=n; count-=n;
    }
}
juce::String ContentHash::finish()
{
    std::array<unsigned char,32> digest{};
    if (BCryptFinishHash(hash,digest.data(),static_cast<ULONG>(digest.size()),0)<0) throw std::runtime_error("SHA-256 finalization failed");
    return juce::String::toHexString(digest.data(),static_cast<int>(digest.size()),0);
}
juce::String fileHash(const juce::File& file)
{
    auto stream=file.createInputStream(); if (!stream) throw std::runtime_error("Missing managed audio file");
    ContentHash hash; std::array<char,65536> block;
    while (!stream->isExhausted()) { const auto count=stream->read(block.data(),static_cast<int>(block.size())); if(count<=0) throw std::runtime_error("Cannot read managed audio"); hash.update(block.data(),static_cast<size_t>(count)); }
    return hash.finish();
}
juce::String sampleHash(const std::vector<float>& samples, double rate, int channels)
{
    ContentHash hash; hash.update(&rate,sizeof(rate)); hash.update(&channels,sizeof(channels));
    const uint64_t frames=samples.size()/static_cast<size_t>(channels); hash.update(&frames,sizeof(frames));
    hash.samples(samples.data(),samples.size()); return hash.finish();
}
}
