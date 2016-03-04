#ifndef _GPB_GENERATOR_H_
#define _GPB_GENERATOR_H_

#include "gpbparse.h"

class gpbgenerator
{
public:
	gpbgenerator(){}
	~gpbgenerator(){}

public:
	bool generate(const string& path, const string& file);

private:
	bool genclass(const parsemsg& message);
	string getmemtype(const parsemsg::parsemem& mem);
	string addgpb(const string& memtp);
	bool ismessage(const string& msg);
private:
	gpbtoken mtok;
	gpbparse mparse;

	map<string, string> msgdifines;
};

#endif//_GPB_GENERATOR_H_