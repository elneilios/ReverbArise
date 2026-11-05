#pragma once
#include <BuildInformationData.h>
#include <string>
namespace m3::build
{
inline std::string report()
{
    return std::string(productName)+"\nVersion: "+std::string(version)
        +"\nBuild: "+std::string(identifier)+"\nGit commit: "+std::string(gitSha)
        +"\nSource status: "+(gitSha=="unavailable"?"unavailable":worktreeDirty?"modified":"unmodified")+"\n";
}
}
