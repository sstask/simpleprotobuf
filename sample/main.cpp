#include "my.h"
#include <iostream>
/*
 void pbdtest()
{
    ProtoMsg::mymsg1 pmsg1;
    pmsg1.set_len(123);
    pmsg1.add_typ("123");
    pmsg1.add_typ("456");
    pmsg1.add_typ("789");
    ProtoMsg::mymsg1_my* pmy=pmsg1.add_msg();
    pmy->set_len(456);
    pmy->mutable_temp()->set_len(789);
    pmy=pmsg1.add_msg();
    pmy->set_len(123);
    pmy->mutable_temp()->set_len(456);
    pmsg1.mutable_mes()->set_len(999999999);
    pmsg1.mutable_mes()->mutable_temp()->set_len(888888);

    mymsg1 msg1;
    std::string out = pmsg1.SerializeAsString();
    msg1.decode((const gpb::uint8*)out.c_str(), out.length());
    gpb::uint8 buf[1024]={0};
    int len = msg1.encode(buf);
    pmsg1.Clear();
    pmsg1.ParseFromArray(buf, len);
    mymsg1 msg11;
    std::string out1 = pmsg1.SerializeAsString();
    msg11.decode((const gpb::uint8*)out1.c_str(), out1.length());
    std::cout<<msg11.tostr();

    ProtoMsg::mymsg2 pmsg2;
    pmsg2.set_len(34234567);
    pmsg2.set_typ(12121);
    ::google::protobuf::Map< ::google::protobuf::uint32, ::std::string >*
        msg_map_map = pmsg2.mutable_str();
    (*msg_map_map)[1]="11";
    (*msg_map_map)[2222]="2222";

    mymsg2 msg2;
    std::string out2 = pmsg2.SerializeAsString();
    msg2.decode((const gpb::uint8*)out2.c_str(), out2.length());
    len = msg2.encode(buf);
    pmsg2.Clear();
    pmsg2.ParseFromArray(buf, len);
    mymsg2 msg22;
    std::string out22 = pmsg2.SerializeAsString();
    msg22.decode((const gpb::uint8*)out22.c_str(), out22.length());
    std::cout<<msg22.tostr();

    ProtoMsg::mymsg3 pmsg3;
    pmsg3.mutable_tt()->set_a(11324345);
    pmsg3.mutable_tt()->add_b("hello");
    pmsg3.mutable_tt()->add_b("world");
    ::google::protobuf::Map< ::google::protobuf::int32, ::google::protobuf::int32 >*
        pmsg3map = pmsg3.mutable_tt()->mutable_d();
    (*pmsg3map)[1]=11;
    (*pmsg3map)[3]=3;
    (*pmsg3map)[2222]=222222;

    mymsg3 msg3;
    std::string out3 = pmsg3.SerializeAsString();
    msg3.decode((const gpb::uint8*)out3.c_str(), out3.length());
    len = msg3.encode(buf);
    pmsg3.Clear();
    pmsg3.ParseFromArray(buf, len);
    mymsg3 msg33;
    std::string out33 = pmsg3.SerializeAsString();
    msg33.decode((const gpb::uint8*)out33.c_str(), out33.length());
    std::cout<<msg33.tostr();
}
 */
int main()
{
    mymsg1 msg1;
    msg1.setlen(1);
    vector<string> strs;
    strs.push_back("hello");
    strs.push_back("world");
    msg1.settyp(strs);
    mymsg1_my msg2;
    msg2.setlen(22222);
    mymsg1_my_mymy msg3;
    msg3.setlen(333);
    msg2.settemp(msg3);
    vector<mymsg1_my> vecmsg;
    vecmsg.push_back(msg2);
    vecmsg.push_back(msg2);
    msg1.setmsg(vecmsg);
    msg1.setmes(msg2);
    gpb::uint8 buf[1024]={0};
    int len = msg1.encode(buf);
    mymsg1 msg11;
    msg11.decode(buf, len);
    std::cout<<msg11.tostr();
    return 0;
}
