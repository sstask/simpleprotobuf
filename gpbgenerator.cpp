#include "gpbgenerator.h"
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>

bool gpbgenerator::ismessage(const string& msg)
{
	static const char* k[]={"uint32","sint32","uint64","sint64","float","double","bool","string"};
	static std::set<string> keys(k,k+8);
	return keys.find(msg)==keys.end();
}

string gpbgenerator::addgpb(const string& memtp)
{
	string stype = memtp;
	if(memtp.compare("uint32")==0)
		stype = "gpb::uint32";
	else if(memtp.compare("sint32")==0)
		stype = "gpb::int32";
	else if(memtp.compare("uint64")==0)
		stype = "gpb::uint64";
	else if(memtp.compare("sint64")==0)
		stype = "gpb::int64";
	return stype;
}

string gpbgenerator::getmemtype(const parsemsg::parsemem& mem)
{
	string stype=addgpb(mem.type);
	if(mem.symbol.compare("repeated")==0)
		stype = "vector<"+addgpb(mem.type)+">";
	else if(mem.type.compare("map")==0)
		stype = "map<"+addgpb(mem.keytype)+", "+addgpb(mem.valtype)+">";
	return stype;
}

bool gpbgenerator::genclass(const parsemsg& message)
{
	string classes;
	string constructor;
	string meminit;
	string destructor;
	string functions;
	string members;
	string encode="\tint encode(gpb::uint8* buffer)\n\t{\n\t\tgpb::uint8* buf = buffer;\n";
	string decode="\tbool decode(const gpb::uint8* buffer, int len)\n\t{\n\t\tgpb::io::CodedInputStream input(buffer, len);\n\t\tfor(;;)\n\t\t{\n\t\t\tif(input.BufferSize() <= 0) return true;\n\t\t\tgpb::uint32 tag = 0;\n\t\t\tif(!input.ReadVarint32(&tag)) return false;\n"
		"\t\t\tint fieldnumber = gpb::WireFormatLite::GetTagFieldNumber(tag);\n\t\t\tswitch (fieldnumber)\n\t\t\t{\n";

	string msg_name = message.name;
	string tostr="\tstring tostr()\n\t{\n\t\tstring out=\""+msg_name+"={\";\n";
	classes = "class "+msg_name+":public gpb::gpbmsg\n{\npublic:";
	constructor = "\n\t"+msg_name+"()";
	destructor = "\n\t~"+msg_name+"(){}\n";
	functions = "\npublic:\n";
	members = "\nprivate:\n";
	// generate fields
	for (size_t j = 0; j <message.vecmems.size(); ++j) {
		const parsemsg::parsemem& mem = message.vecmems[j];
		string memname = "m_"+mem.name;
		string stype = getmemtype(mem);
		if(stype.empty())
			return false;
		members+="\t"+stype+" "+memname+";\n";

		tostr += "\t\tout += \""+memname+":[\";\n";
		std::ostringstream sBufferFlag;
		sBufferFlag << mem.flag;
		if(mem.type.compare("map")==0){
			encode += "\t\tbuf = gpb::gpbencoder::WriteMapToArray("+sBufferFlag.str()+", "+memname+", buf);\n";

			string key = addgpb(mem.keytype);
			string val = addgpb(mem.valtype);
			decode += "\t\t\tcase "+sBufferFlag.str()+":\n\t\t\t\t{\n\t\t\t\t\t"+"gpb::uint32"+" len;\n"
				"\t\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &len)) return false;\n"+
				"\t\t\t\t\t"+"gpb::uint32"+" keytag;\n"+
				"\t\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &keytag)) return false;\n"+
				"\t\t\t\t\t"+key+" key;\n"+
				"\t\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &key)) return false;\n"+
				"\t\t\t\t\t"+"gpb::uint32"+" valtag;\n"+
				"\t\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &valtag)) return false;\n"+
				"\t\t\t\t\t"+val+" val;\n"+
				"\t\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &val)) return false;\n"+
				"\t\t\t\t\t"+memname+".insert(std::make_pair(key, val));\n\t\t\t\t\tbreak;\n\t\t\t\t}\n";

			if (ismessage(mem.valtype)){
				tostr += "\t\tfor("+stype+"::iterator it = "+memname+".begin(); it != "+memname+".end(); ++it)\n\t\t{\n\t"+
					"\t\tout += gpb::gpbtostr::tostr(it->first)+\":\"+gpb::gpbtostr::tostr((gpbmsg*)&(it->seconde));\n\t\t}\n";
			}
			else
				tostr += "\t\tout += gpb::gpbtostr::tostr("+memname+");\n";
			
		}else if(mem.symbol.compare("repeated")==0){
			if (ismessage(mem.type)){
				encode += "\t\tfor(int i = 0; i < (int)"+memname+".size(); ++i)\n\t\t{\n\t"+
					"\t\tbuf = gpb::gpbencoder::WriteDataToArray("+sBufferFlag.str()+", "+memname+"[i], buf);\n\t\t}\n";
			}
			else
				encode += "\t\tbuf = gpb::gpbencoder::WriteRepeatedToArray("+sBufferFlag.str()+", "+memname+", buf);\n";

			if (mem.type.compare("string")==0 || ismessage(mem.type)){
				string newtype = mem.type;
				decode += "\t\t\tcase "+sBufferFlag.str()+":\n\t\t\t\t{\n\t\t\t\t\t"+newtype+" val;\n"
					"\t\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &val)) return false;\n"+
					"\t\t\t\t\t"+memname+".push_back(val);\n\t\t\t\t\tbreak;\n\t\t\t\t}\n";
			}
			else
				decode += "\t\t\tcase "+sBufferFlag.str()+":\n\t\t\t\t{\n\t\t\t\t\tif(!gpb::gpbdecoder::ReadRepeatedPrimitive(&input, "+memname+")) return false;\n\t\t\t\t\tbreak;\n\t\t\t\t}\n";

			if (ismessage(mem.type)){
				tostr += "\t\tfor(int i = 0; i < (int)"+memname+".size(); ++i)\n\t\t{\n\t"+
					"\t\tout += gpb::gpbtostr::tostr((gpbmsg*)&"+memname+"[i]);\n\t\t}\n";
			}
			else
				tostr += "\t\tout += gpb::gpbtostr::tostr("+memname+");\n";
		}else{
			encode += "\t\tbuf = gpb::gpbencoder::WriteDataToArray("+sBufferFlag.str()+", "+memname+", buf);\n";
			decode += "\t\t\tcase "+sBufferFlag.str()+":\n\t\t\t\t{\n\t\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &"+memname+")) return false;\n\t\t\t\t\tbreak;\n\t\t\t\t}\n";
			if (ismessage(mem.type)){
				tostr += "\t\tout += gpb::gpbtostr::tostr((gpbmsg*)&"+memname+");\n";
			}
			else{
				tostr += "\t\tout += gpb::gpbtostr::tostr("+memname+");\n";

				if(mem.type.compare("string") != 0){
					if(meminit.empty()){
						meminit = ":" + memname + "(0)";
					}else
						meminit += "," + memname + "(0)";
				}
			}
		}
		tostr += "\t\tout += \"]\";\n";
		
		if (mem.type.compare("map")==0||mem.symbol.compare("repeated")==0||ismessage(mem.type)){
			stype += "&";
		}
		functions+="\t"+stype+" get"+mem.name+"(){return "+memname+";}\n";
		functions+="\tvoid set"+mem.name+"("+stype+" "+mem.name+"){"+memname+"="+mem.name+";}\n";
	}
	encode+="\t\treturn buf - buffer;\n\t}\n";
	decode+="\t\t\tdefault: return false;\n\t\t\t}\n\t\t}\n\t}\n";
	tostr+="\t\treturn out+\"}\";\n\t}\n";
	constructor += meminit + "{}";
	msgdifines.insert(std::make_pair(msg_name, classes+constructor+destructor+encode+decode+tostr+functions+members+"};\n\n"));
	return true;
}

bool gpbgenerator::generate(const string& path, const string& file)
{
	if(!mtok.load(path, file))
		return false;
	if(!mparse.parse(mtok))
		return false;

	// generate messages
	for (vector<parsemsg>::iterator itr = mparse.vecmsgs.begin(); itr != mparse.vecmsgs.end(); ++itr) {
		genclass(*itr);
	}

	// Generate c file.
	{
		string flname = path + file;
		string::size_type pos(0);
		if((pos=flname.find(".proto", pos))!=string::npos){
			flname.replace(pos+1,5,"h");
		}

		std::ofstream sfile(flname.c_str());
		if(!sfile.is_open())
			return false;
		string fbasename = file;
		pos = 0;;
		if((pos=fbasename.find(".proto", pos))!=string::npos){
			fbasename = fbasename.substr(0, pos);
		}
		for (size_t i = 0; i < fbasename.size(); ++i)
			fbasename[i] = toupper(fbasename[i]);
		sfile<<"#ifndef _GPB_"<<fbasename<<"_H_\n";
		sfile<<"#define _GPB_"<<fbasename<<"_H_\n";

		sfile<<"#include \"gpbencode.h\"\n";

		for (size_t i = 0; i < mparse.vecincludes.size(); ++i){
			sfile<<"#include \""+mparse.vecincludes[i]+".h\"\n";
		}
		
		for (map<string, map<int, string> >::iterator itenum = mparse.mapenums.begin(); itenum != mparse.mapenums.end(); ++itenum)
		{
			sfile << "\nenum "<<itenum->first<<"{";
			map<int, string>& enumtmp = itenum->second;
			for (map<int, string>::iterator it = enumtmp.begin(); it != enumtmp.end(); ++it)
			{
				sfile << "\n\t"<<it->second<<" ="<<" "<<it->first<<",";
			}
			sfile << "\n};\n\n";
		}
		
		for (vector<parsemsg>::iterator itr = mparse.vecmsgs.begin(); itr != mparse.vecmsgs.end(); ++itr) {
			sfile << "class "<<itr->name<<";\n";
		}
		sfile << "\n";
		for (vector<parsemsg>::iterator itr = mparse.vecmsgs.begin(); itr != mparse.vecmsgs.end(); ++itr) {
			sfile << msgdifines[itr->name];
		}
		sfile<<"#endif//_GPB_"<<fbasename<<"_H_\n";
		sfile.close();
	}

	return true;
}
