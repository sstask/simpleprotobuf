#ifndef _GPB_GENERATOR_H_
#define _GPB_GENERATOR_H_

class gpbgenerator
{
public:
	gpbgenerator(){}
	~gpbgenerator(){}

public:
	bool generate(const char* path, const char* file);
};

#endif//_GPB_GENERATOR_H_