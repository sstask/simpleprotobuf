#ifndef _GPB_MY_H_
#define _GPB_MY_H_
#include "gpbencode.h"
#include "aa.h"
class mymsg;
class mymsg1;
class mymsg1_my;
class mymsg2;
class mymsg3;
class mymsg1_my:public gpb::gpbmsg{
public:
	mymsg1_my(){}
	~mymsg1_my(){}
	int encode(gpb::uint8* buffer){
		gpb::uint8* buf = buffer;
		buf = gpb::gpbencoder::WriteDataToArray(1, m_len, buf);
		return buf - buffer;
	}
	bool decode(const gpb::uint8* buffer, int len){
		gpb::io::CodedInputStream input(buffer, len);
		for(;;){
			if(input.BufferSize() <= 0) return true;
			gpb::uint32 tag = 0;
			if(!input.ReadVarint32(&tag)) return false;
			int fieldnumber = gpb::WireFormatLite::GetTagFieldNumber(tag);
			switch (fieldnumber)
			{
			case 1:{
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &m_len)) return false;break;
			}
			default: return false;
			}
		}
	}

public:
	gpb::uint32 getlen(){return m_len;}
	void setlen(gpb::uint32 len){m_len=len;}

private:
	gpb::uint32 m_len;
};

class mymsg1:public gpb::gpbmsg{
public:
	mymsg1(){}
	~mymsg1(){}
	int encode(gpb::uint8* buffer){
		gpb::uint8* buf = buffer;
		buf = gpb::gpbencoder::WriteDataToArray(1, m_len, buf);
		buf = gpb::gpbencoder::WriteRepeatedToArray(2, m_typ, buf);
		for(int i = 0; i < (int)m_msg.size(); ++i){
			buf = gpb::gpbencoder::WriteDataToArray(3, m_msg[i], buf);
		}
		buf = gpb::gpbencoder::WriteDataToArray(4, m_mes, buf);
		return buf - buffer;
	}
	bool decode(const gpb::uint8* buffer, int len){
		gpb::io::CodedInputStream input(buffer, len);
		for(;;){
			if(input.BufferSize() <= 0) return true;
			gpb::uint32 tag = 0;
			if(!input.ReadVarint32(&tag)) return false;
			int fieldnumber = gpb::WireFormatLite::GetTagFieldNumber(tag);
			switch (fieldnumber)
			{
			case 1:{
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &m_len)) return false;break;
			}
			case 2:{
				string val;
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &val)) return false;
				m_typ.push_back(val);break;
			}
			case 3:{
				mymsg1_my val;
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &val)) return false;
				m_msg.push_back(val);break;
			}
			case 4:{
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &m_mes)) return false;break;
			}
			default: return false;
			}
		}
	}

public:
	gpb::uint32 getlen(){return m_len;}
	void setlen(gpb::uint32 len){m_len=len;}
	vector<string> gettyp(){return m_typ;}
	void settyp(vector<string> typ){m_typ=typ;}
	vector<mymsg1_my>& getmsg(){return m_msg;}
	void setmsg(vector<mymsg1_my>& msg){m_msg=msg;}
	mymsg1_my& getmes(){return m_mes;}
	void setmes(mymsg1_my& mes){m_mes=mes;}

private:
	gpb::uint32 m_len;
	vector<string> m_typ;
	vector<mymsg1_my> m_msg;
	mymsg1_my m_mes;
};

class mymsg:public gpb::gpbmsg{
public:
	mymsg(){}
	~mymsg(){}
	int encode(gpb::uint8* buffer){
		gpb::uint8* buf = buffer;
		buf = gpb::gpbencoder::WriteDataToArray(1, m_len, buf);
		buf = gpb::gpbencoder::WriteRepeatedToArray(2, m_typ, buf);
		return buf - buffer;
	}
	bool decode(const gpb::uint8* buffer, int len){
		gpb::io::CodedInputStream input(buffer, len);
		for(;;){
			if(input.BufferSize() <= 0) return true;
			gpb::uint32 tag = 0;
			if(!input.ReadVarint32(&tag)) return false;
			int fieldnumber = gpb::WireFormatLite::GetTagFieldNumber(tag);
			switch (fieldnumber)
			{
			case 1:{
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &m_len)) return false;break;
			}
			case 2:{
				if(!gpb::gpbdecoder::ReadRepeatedPrimitive(&input, m_typ)) return false;break;
			}
			default: return false;
			}
		}
	}

public:
	gpb::uint32 getlen(){return m_len;}
	void setlen(gpb::uint32 len){m_len=len;}
	vector<gpb::uint32> gettyp(){return m_typ;}
	void settyp(vector<gpb::uint32> typ){m_typ=typ;}

private:
	gpb::uint32 m_len;
	vector<gpb::uint32> m_typ;
};

class mymsg2:public gpb::gpbmsg{
public:
	mymsg2(){}
	~mymsg2(){}
	int encode(gpb::uint8* buffer){
		gpb::uint8* buf = buffer;
		buf = gpb::gpbencoder::WriteDataToArray(1, m_len, buf);
		buf = gpb::gpbencoder::WriteDataToArray(2, m_typ, buf);
		buf = gpb::gpbencoder::WriteMapToArray(3, m_str, buf);
		return buf - buffer;
	}
	bool decode(const gpb::uint8* buffer, int len){
		gpb::io::CodedInputStream input(buffer, len);
		for(;;){
			if(input.BufferSize() <= 0) return true;
			gpb::uint32 tag = 0;
			if(!input.ReadVarint32(&tag)) return false;
			int fieldnumber = gpb::WireFormatLite::GetTagFieldNumber(tag);
			switch (fieldnumber)
			{
			case 1:{
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &m_len)) return false;break;
			}
			case 2:{
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &m_typ)) return false;break;
			}
			case 3:{
				gpb::uint32 len;
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &len)) return false;
				gpb::uint32 keytag;
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &keytag)) return false;
				gpb::uint32 key;
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &key)) return false;
				gpb::uint32 valtag;
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &valtag)) return false;
				string val;
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &val)) return false;
				m_str.insert(make_pair(key, val));break;
			}
			default: return false;
			}
		}
	}

public:
	gpb::uint32 getlen(){return m_len;}
	void setlen(gpb::uint32 len){m_len=len;}
	gpb::uint32 gettyp(){return m_typ;}
	void settyp(gpb::uint32 typ){m_typ=typ;}
	map<gpb::uint32, string>& getstr(){return m_str;}
	void setstr(map<gpb::uint32, string>& str){m_str=str;}

private:
	gpb::uint32 m_len;
	gpb::uint32 m_typ;
	map<gpb::uint32, string> m_str;
};

class mymsg3:public gpb::gpbmsg{
public:
	mymsg3(){}
	~mymsg3(){}
	int encode(gpb::uint8* buffer){
		gpb::uint8* buf = buffer;
		buf = gpb::gpbencoder::WriteDataToArray(1, m_tt, buf);
		return buf - buffer;
	}
	bool decode(const gpb::uint8* buffer, int len){
		gpb::io::CodedInputStream input(buffer, len);
		for(;;){
			if(input.BufferSize() <= 0) return true;
			gpb::uint32 tag = 0;
			if(!input.ReadVarint32(&tag)) return false;
			int fieldnumber = gpb::WireFormatLite::GetTagFieldNumber(tag);
			switch (fieldnumber)
			{
			case 1:{
				if(!gpb::gpbdecoder::ReadPrimitive(&input, &m_tt)) return false;break;
			}
			default: return false;
			}
		}
	}

public:
	test& gettt(){return m_tt;}
	void settt(test& tt){m_tt=tt;}

private:
	test m_tt;
};

#endif//_GPB_MY_H_
