#include "gpb_generator.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace google::protobuf;

static map<string, const Descriptor*> mapmsgs;

string GetFiledType(const FieldDescriptor* field){
	string stype;
	switch(field->cpp_type()) {
	case FieldDescriptor::CPPTYPE_INT32   : 
	case FieldDescriptor::CPPTYPE_ENUM    : 
		stype = "gpb::int32";
		break;
	case FieldDescriptor::CPPTYPE_INT64   : 
		stype = "gpb::int64";
		break;
	case FieldDescriptor::CPPTYPE_UINT32  : 
		stype = "gpb::uint32";
		break;
	case FieldDescriptor::CPPTYPE_UINT64  : 
		stype = "gpb::uint64";
		break;
	case FieldDescriptor::CPPTYPE_FLOAT   : 
		stype = "gpb::float";
		break;
	case FieldDescriptor::CPPTYPE_DOUBLE  : 
		stype = "gpb::double";
		break;
	case FieldDescriptor::CPPTYPE_BOOL    : 
		stype = "gpb::bool";
		break;
	case FieldDescriptor::CPPTYPE_STRING  : 
		stype = "string";
		break;
	case FieldDescriptor::CPPTYPE_MESSAGE : 
		{
			stype = field->message_type()->full_name();
			string::size_type pos(0);
			while((pos=stype.find(".", pos))!=string::npos){
				stype.replace(pos,1,"_");
			}
		}
		break;
	}
	if(field->is_map()){
		map<string, const Descriptor*>::iterator itr = mapmsgs.find(stype);
		if(itr != mapmsgs.end()){
			const Descriptor* mes = itr->second;
			string key = GetFiledType(mes->field(0));
			string val = GetFiledType(mes->field(1));
			stype = "map<"+key+", "+val+">";
		}else{
			stype="";
		}
	}else if(field->is_repeated()){
		stype = "vector<"+stype+">";
	}
	return stype;
}

bool genclass(const Descriptor* message, map<string, string>& msgdifines)
{
	string classes;
	string functions;
	string members;
	string encode="\tint encode(gpb::uint8* buffer){\n\t\tgpb::uint8* buf = buffer;\n";
	string decode="\tbool decode(const gpb::uint8* buffer, int len){\n\t\tgpb::io::CodedInputStream input(buffer, len);\n\t\tfor(;;){\n\t\t\tif(input.BufferSize() <= 0) return true;\n\t\t\tgpb::uint32 tag = 0;\n\t\t\tif(!input.ReadVarint32(&tag)) return false;\n"
		"\t\t\tint fieldnumber = gpb::WireFormatLite::GetTagFieldNumber(tag);\n\t\t\tswitch (fieldnumber)\n\t\t\t{\n";

	string msg_name = message->full_name();
	string::size_type pos(0);
	while((pos=msg_name.find(".", pos))!=string::npos){
		msg_name.replace(pos,1,"_");
	}
	mapmsgs.insert(make_pair(msg_name, message));
	if(message->options().map_entry())
		return true;
	string tostr="\tstring tostr(){\n\t\tstring out=\""+msg_name+"={\";\n";
	classes = "class "+msg_name+":public gpb::gpbmsg{\npublic:\n\t"+msg_name+"(){}\n\t~"+msg_name+"(){}\n";
	functions = "\npublic:\n";
	members = "\nprivate:\n";
	// generate fields
	for (int j = 0; j < message->field_count(); ++j) {
		const FieldDescriptor* field = message->field(j);
		string stype = GetFiledType(field);
		if(stype.empty())
			return false;
		string memname = "m_"+field->name();
		members+="\t"+stype+" "+memname+";\n";

		tostr += "\t\tout += \""+memname+":[\";\n";
		ostringstream sBuffer;
		sBuffer << field->number();
		if(field->is_map()){
			encode += "\t\tbuf = gpb::gpbencoder::WriteMapToArray("+sBuffer.str()+", "+memname+", buf);\n";

			string newtype = field->message_type()->full_name();
			string::size_type pos(0);
			while((pos=newtype.find(".", pos))!=string::npos){
				newtype.replace(pos,1,"_");
			}
			map<string, const Descriptor*>::iterator itr = mapmsgs.find(newtype);
			if(itr == mapmsgs.end())
				return false;
			const Descriptor* mes = itr->second;
			string key = GetFiledType(mes->field(0));
			string val = GetFiledType(mes->field(1));
			decode += "\t\t\tcase "+sBuffer.str()+":{\n\t\t\t\t"+"gpb::uint32"+" len;\n"
				"\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &len)) return false;\n"+
				"\t\t\t\t"+"gpb::uint32"+" keytag;\n"+
				"\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &keytag)) return false;\n"+
				"\t\t\t\t"+key+" key;\n"+
				"\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &key)) return false;\n"+
				"\t\t\t\t"+"gpb::uint32"+" valtag;\n"+
				"\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &valtag)) return false;\n"+
				"\t\t\t\t"+val+" val;\n"+
				"\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &val)) return false;\n"+
				"\t\t\t\t"+memname+".insert(std::make_pair(key, val));break;\n\t\t\t}\n";

			if (mes->field(1)->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
				tostr += "\t\tfor("+stype+"::iterator it = "+memname+".begin(); it != "+memname+".end(); ++it){\n\t"+
					"\t\tout += gpb::gpbtostr::tostr(it->first)+\":\"+gpb::gpbtostr::tostr((gpbmsg*)&(it->seconde));\n\t\t}\n";
			}
			else
				tostr += "\t\tout += gpb::gpbtostr::tostr("+memname+");\n";
			
		}else if(field->is_repeated()){
			if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
				encode += "\t\tfor(int i = 0; i < (int)"+memname+".size(); ++i){\n\t"+
					"\t\tbuf = gpb::gpbencoder::WriteDataToArray("+sBuffer.str()+", "+memname+"[i], buf);\n\t\t}\n";
			}
			else
				encode += "\t\tbuf = gpb::gpbencoder::WriteRepeatedToArray("+sBuffer.str()+", "+memname+", buf);\n";

			if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING || field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
				string newtype = "string";
				if(field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
					newtype = field->message_type()->full_name();
					string::size_type pos(0);
					while((pos=newtype.find(".", pos))!=string::npos){
						newtype.replace(pos,1,"_");
					}
				}
				decode += "\t\t\tcase "+sBuffer.str()+":{\n\t\t\t\t"+newtype+" val;\n"
					"\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &val)) return false;\n"+
					"\t\t\t\t"+memname+".push_back(val);break;\n\t\t\t}\n";
			}
			else
				decode += "\t\t\tcase "+sBuffer.str()+":{\n\t\t\t\tif(!gpb::gpbdecoder::ReadRepeatedPrimitive(&input, "+memname+")) return false;break;\n\t\t\t}\n";

			if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
				tostr += "\t\tfor(int i = 0; i < (int)"+memname+".size(); ++i){\n\t"+
					"\t\tout += gpb::gpbtostr::tostr((gpbmsg*)&"+memname+"[i]);\n\t\t}\n";
			}
			else
				tostr += "\t\tout += gpb::gpbtostr::tostr("+memname+");\n";
		}else{
			encode += "\t\tbuf = gpb::gpbencoder::WriteDataToArray("+sBuffer.str()+", "+memname+", buf);\n";
			decode += "\t\t\tcase "+sBuffer.str()+":{\n\t\t\t\tif(!gpb::gpbdecoder::ReadPrimitive(&input, &"+memname+")) return false;break;\n\t\t\t}\n";
			if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
				tostr += "\t\tout += gpb::gpbtostr::tostr((gpbmsg*)&"+memname+");\n";
			}
			else tostr += "\t\tout += gpb::gpbtostr::tostr("+memname+");\n";
		}
		tostr += "\t\tout += \"]\";\n";
		
		if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE || field->is_repeated() || field->is_map()){
			stype += "&";
		}
		functions+="\t"+stype+" get"+field->name()+"(){return "+memname+";}\n";
		functions+="\tvoid set"+field->name()+"("+stype+" "+field->name()+"){"+memname+"="+field->name()+";}\n";
	}
	encode+="\t\treturn buf - buffer;\n\t}\n";
	decode+="\t\t\tdefault: return false;\n\t\t\t}\n\t\t}\n\t}\n";
	tostr+="\t\treturn out+\"}\";\n\t}\n";
	msgdifines.insert(std::make_pair(msg_name, classes+encode+decode+tostr+functions+members+"};\n\n"));
	return true;
}

void printmsg(ofstream& sfile,const Descriptor* message, map<string, string>& msgdifines)
{
	string msg_name = message->full_name();
	string::size_type pos(0);
	while((pos=msg_name.find(".", pos))!=string::npos){
		msg_name.replace(pos,1,"_");
	}
	map<string, string>::iterator itr = msgdifines.find(msg_name);
	if(itr != msgdifines.end())
		sfile << itr->second;
}

string getprotofilename(const string& name)
{
	string ret = name;
	string::size_type pos(0);
	if((pos=name.find(".proto", pos))!=string::npos){
		ret = name.substr(0, pos);
	}
	return ret;
}

bool gpbgenerator::generate(const char* path, const char* file)
{
	google::protobuf::compiler::DiskSourceTree sourceTree;
	sourceTree.MapPath("", path);
	google::protobuf::compiler::Importer importer(&sourceTree, NULL);
	const google::protobuf::FileDescriptor* pfd = importer.Import(file);
	if (pfd == NULL)
	{
		return false;
	}

	map<string, string> msgdifines;
	// generate messages
	for (int i = 0; i < pfd->message_type_count(); i++) {
		const Descriptor* message	= pfd->message_type(i);
		for (int j = 0; j < message->nested_type_count(); ++j){
			const Descriptor* nest = message->nested_type(j);
			genclass(nest, msgdifines);
		}
		genclass(message, msgdifines);
	}

	// Generate cc file.
	{
		string flname = pfd->name();
		string::size_type pos(0);
		if((pos=flname.find(".proto", pos))!=string::npos){
			flname.replace(pos+1,5,"h");
		}

		ofstream sfile(flname);
		if(!sfile.is_open())
			return false;
		string fbasename = getprotofilename(pfd->name());
		for (size_t i = 0; i < fbasename.size(); ++i)
			fbasename[i] = toupper(fbasename[i]);
		sfile<<"#ifndef _GPB_"<<fbasename<<"_H_\n";
		sfile<<"#define _GPB_"<<fbasename<<"_H_\n";

		sfile<<"#include \"gpbencode.h\"\n";

		for (int i = 0; i < pfd->dependency_count(); ++i)
		{
			sfile<<"#include \""+getprotofilename(pfd->dependency(i)->name())+".h\"\n";
		}
		
		for (map<string, string>::iterator itr = msgdifines.begin(); itr != msgdifines.end(); ++itr)
		{
			sfile << "class "<<itr->first<<";\n";
		}
		for (int i = 0; i < pfd->message_type_count(); i++) {
			const Descriptor* message	= pfd->message_type(i);
			for (int j = 0; j < message->nested_type_count(); ++j){
				const Descriptor* nest = message->nested_type(j);
				printmsg(sfile, nest, msgdifines);
			}
			printmsg(sfile, message, msgdifines);
		}
		sfile<<"#endif//_GPB_"<<fbasename<<"_H_\n";
		sfile.close();
	}

	return true;
}
