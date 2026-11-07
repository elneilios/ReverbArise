static void directoryReparseProbe()
{
    TestDirectory parent,outside;REQUIRE(parent.file.createDirectory().wasOk() && outside.file.createDirectory().wasOk());
    REQUIRE(parent.file.getChildFile("original.txt").replaceWithText("owned native probe"));bool converted=false;
    try{junction(parent.file,outside.file);converted=true;}catch(const std::exception&){}
    if(converted){
        const auto handle=CreateFileW(parent.file.getFullPathName().toWideCharPointer(),GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS,nullptr);
        REQUIRE(handle!=INVALID_HANDLE_VALUE);struct Header{DWORD tag;USHORT length,reserved;} data{IO_REPARSE_TAG_MOUNT_POINT,0,0};DWORD actual=0;
        const bool removed=DeviceIoControl(handle,FSCTL_DELETE_REPARSE_POINT,&data,sizeof(data),nullptr,0,&actual,nullptr)!=0;CloseHandle(handle);REQUIRE(removed);
    }
    REQUIRE(parent.file.getChildFile("original.txt").loadFileAsString()=="owned native probe");
    REQUIRE(!converted);
    std::cout<<"Native directory guard probe: nonempty directory junction conversion "<<(converted?"allowed":"rejected")<<"; original owned data preserved.\n";
}
