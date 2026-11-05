#include <Capture/CaptureFailure.h>
#include <iostream>
#include <thread>

int main()
{
    CaptureFailureMailbox mailbox;CaptureFailureEvent event;
    for(uint64_t n=1;n<=64;++n){event.pass=n;event.start=static_cast<int64_t>(n*100);if(!mailbox.push(event))return 1;}
    if(mailbox.push(event))return 2;
    for(uint64_t n=1;n<=64;++n)if(!mailbox.pop(event) || event.pass!=n || event.start!=static_cast<int64_t>(n*100))return 3;
    if(mailbox.pop(event))return 4;
    std::atomic<bool> bad{false};
    std::thread producer([&]{for(uint64_t n=1;n<=100000;++n){CaptureFailureEvent next;next.pass=n;next.order=n;next.start=static_cast<int64_t>(n*100);next.timing.ppqPosition=double(n);while(!mailbox.push(next))std::this_thread::yield();}});
    for(uint64_t n=1;n<=100000;++n)
    {
        while(!mailbox.pop(event))std::this_thread::yield();
        if(event.pass!=n || event.order!=n || event.start!=static_cast<int64_t>(n*100) || event.timing.ppqPosition!=double(n))bad=true;
    }
    producer.join();if(bad)return 5;
    std::cout<<"100000 coherent failure publications; full mailbox never overwrites unread evidence\n";
    return 0;
}
