#ifndef _GPBPARSE_H_
#define _GPBPARSE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;
using std::set;

enum TokenType
{
	TYPE_BEGIN,
	TYPE_IDENTIFIER,
	TYPE_SYMBOL,
	TYPE_INTEGER,
	TYPE_END,
	TYPE_ERROR,
};

struct token
{
	token():tktype(TYPE_ERROR)
	{}
	int tktype;
	string text;
	int line;
};

class gpbtoken
{
public:
	gpbtoken();
	~gpbtoken();

	bool load(const string& path, const string& file);
	token next();

private:
	bool whitespace();
	string readidentifier();
	string readinteger();

private:
	char* content;
	char* pcurr;
	int line;
	vector<token> vectokens;
};

struct parsemsg{
	struct parsemem{
		string name;
		string type;
		string symbol;
		int flag;
		string keytype;
		string valtype;
	};
	string name;
	string barename;
	vector<parsemem> vecmems;
	set<string> setchilds;
};
class gpbparse
{
public:
	gpbparse();
	~gpbparse();

public:
	bool parse(gpbtoken& tk);

private:
	bool readsyntax(gpbtoken& tk);
	bool readinclude(gpbtoken& tk);
	bool readenum(gpbtoken& tk);
	bool readmessage(gpbtoken& tk, parsemsg& father);
	bool readmsgmem(gpbtoken& tk, parsemsg::parsemem& mem, parsemsg& father);
	bool readonemem(gpbtoken& tk, parsemsg::parsemem& mem);

public:
	vector<parsemsg> vecmsgs;
	vector<string> vecincludes;
	map<string, map<int, string> > mapenums;
};

#endif//_GPBPARSE_H_

