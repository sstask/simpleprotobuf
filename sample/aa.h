#ifndef _GPB_AA_H_
#define _GPB_AA_H_
#include "gpbencode.h"
class test;

class test:public gpb::gpbmsg
{
public:
	test():m_a(0){}
	~test(){}
	int encode(gpb::uint8* buffer)
	{
		gpb::uint8* buf = buffer;
		buf = gpb::gpbencoder::WriteDataToArray(1, m_a, buf);
		buf = gpb::gpbencoder::WriteRepeatedToArray(2, m_b, buf);
		buf = gpb::gpbencoder::WriteMapToArray(4, m_d, buf);
		return buf - buffer;
	}
	bool decode(const gpb::uint8* buffer, int len)
	{
		gpb::io::CodedInputStream input(buffer, len);
		for(;;)
		{
			if(input.BufferSize() <= 0) return true;
			gpb::uint32 tag = 0;
			if(!input.ReadVarint32(&tag)) return false;
			int fieldnumber = gpb::WireFormatLite::GetTagFieldNumber(tag);
			switch (fieldnumber)
			{
			case 1:
				{
					if(!gpb::gpbdecoder::ReadPrimitive(&input, &m_a)) return false;
					break;
				}
			case 2:
				{
					string val;
					if(!gpb::gpbdecoder::ReadPrimitive(&input, &val)) return false;
					m_b.push_back(val);
					break;
				}
			case 4:
				{
					gpb::uint32 len;
					if(!gpb::gpbdecoder::ReadPrimitive(&input, &len)) return false;
					gpb::uint32 keytag;
					if(!gpb::gpbdecoder::ReadPrimitive(&input, &keytag)) return false;
					gpb::int32 key;
					if(!gpb::gpbdecoder::ReadPrimitive(&input, &key)) return false;
					gpb::uint32 valtag;
					if(!gpb::gpbdecoder::ReadPrimitive(&input, &valtag)) return false;
					gpb::int32 val;
					if(!gpb::gpbdecoder::ReadPrimitive(&input, &val)) return false;
					m_d.insert(std::make_pair(key, val));
					break;
				}
			default: return false;
			}
		}
	}
	string tostr()
	{
		string out="test={";
		out += "m_a:[";
		out += gpb::gpbtostr::tostr(m_a);
		out += "]";
		out += "m_b:[";
		out += gpb::gpbtostr::tostr(m_b);
		out += "]";
		out += "m_d:[";
		out += gpb::gpbtostr::tostr(m_d);
		out += "]";
		return out+"}";
	}

public:
	gpb::int32 geta(){return m_a;}
	void seta(gpb::int32 a){m_a=a;}
	vector<string>& getb(){return m_b;}
	void setb(vector<string>& b){m_b=b;}
	map<gpb::int32, gpb::int32>& getd(){return m_d;}
	void setd(map<gpb::int32, gpb::int32>& d){m_d=d;}

private:
	gpb::int32 m_a;
	vector<string> m_b;
	map<gpb::int32, gpb::int32> m_d;
};

#endif//_GPB_AA_H_
