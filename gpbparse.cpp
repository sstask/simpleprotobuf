#include "gpbparse.h"
#include <set>

gpbtoken::gpbtoken():content(NULL)
	,pcurr(NULL)
	,line(0)
{}

gpbtoken::~gpbtoken(){
	if(content)
		free(content);
}

bool gpbtoken::whitespace(){
	while(*pcurr!='\0'){
		char c = *pcurr;
		if(c == ' '|| c == '\t' || c == '\r' || c == '\v' || c == '\f'){
			pcurr++;
		}else if(c == '\n'){
			line++;
			pcurr++;
		}else if(c == '/'){
			pcurr++;
			if(*pcurr == '/'){
				while(*pcurr!='\0' && *pcurr!='\n')
					pcurr++;
			}else if(*pcurr == '*'){
				bool isfind = false;
				while(*pcurr!='\0'){
					pcurr++;
					if(c == '\n')
						line++;
					if(*pcurr == '*'){
						pcurr++;
						if(*pcurr == '/'){
							isfind = true;
							pcurr++;
							break;
						}
					}
				}
				if(!isfind)
					return false;
			}else{
				return false;
			}
		}
		else
			return true;
	}
	return true;
}

bool isletter(char c){
	if(('a' <= c && c <= 'z') ||
		('A' <= c && c <= 'Z') ||
		(c == '_'))
		return true;
	return false;
}
bool isdigit(char c){
	if('0' <= c && c <= '9')
		return true;
	return false;
}

string gpbtoken::readidentifier()
{
	string ret;
	while(*pcurr!='\0' && (isletter(*pcurr)||isdigit(*pcurr))){
		ret+=*pcurr;
		pcurr++;
	}
	return ret;
}

string gpbtoken::readinteger()
{
	string ret;
	while(*pcurr!='\0' && isdigit(*pcurr)){
		ret+=*pcurr;
		pcurr++;
	}
	return ret;
}

token gpbtoken::next(){
	token tk;
	if(pcurr == NULL){
		if(vectokens.empty()){
			tk.tktype = TYPE_BEGIN;
			pcurr = content;
		}
		else
			tk.tktype = TYPE_END;
		return tk;
	}
	if(!whitespace()){
		tk.tktype = TYPE_ERROR;
		return tk;
	}
	if(isletter(*pcurr)){
		tk.tktype=TYPE_IDENTIFIER;
		tk.text=readidentifier();
	}else if(isdigit(*pcurr)){
		tk.tktype=TYPE_INTEGER;
		tk.text=readinteger();
	}else if(*pcurr=='\0'){
		tk.tktype = TYPE_END;
	}else{
		tk.tktype=TYPE_SYMBOL;
		tk.text=*pcurr;
		pcurr++;
	}
	vectokens.push_back(tk);
	return tk;
}

bool gpbtoken::load(const string& path, const string& file)
{
	FILE *fp = fopen((path+file).c_str(), "rb");
	if (!fp)
		return false;
	int fsize = 0;
	fseek(fp,0,SEEK_END);
	fsize = ftell(fp);
	fseek(fp,0,SEEK_SET);
	content = (char*)malloc(fsize + 3);
	if (!content){
		fclose(fp);
		return false;
	}
	memset(content, 0, fsize + 3);
	if(0 == fread(content, 1, fsize, fp))
		return false;
	fclose(fp);
	return true;
}

gpbparse::gpbparse()
{
}

gpbparse::~gpbparse()
{
}

bool gpbparse::parse(gpbtoken& tk)
{
	token tok = tk.next();
	if(tok.tktype != TYPE_BEGIN)
		return false;
	while(tok.tktype!=TYPE_END&&tok.tktype!=TYPE_ERROR){
		tok = tk.next();
		if(tok.tktype==TYPE_ERROR)
			return false;
		if(tok.tktype==TYPE_END)
			return true;
		bool succ = true;
		if(tok.text.compare("syntax")==0){
			succ = readsyntax(tk);
		}else if(tok.text.compare("import")==0){
			succ = readinclude(tk);
		}else if(tok.text.compare("enum")==0){
			succ = readenum(tk);
		}else if(tok.text.compare("message")==0){
			parsemsg temp;
			succ = readmessage(tk, temp);
		}else{
			succ =false;
		}
		if(!succ)
			return succ;
	}
	return true;
}

bool gpbparse::readsyntax(gpbtoken& tk)
{
	token tok = tk.next();
	if(tok.text.compare("=")!=0)
		return false;
	tok = tk.next();
	if(tok.text.compare("\"")!=0)
		return false;
	tok = tk.next();
	if(tok.tktype != TYPE_IDENTIFIER)
		return false;
	tok = tk.next();
	if(tok.text.compare("\"")!=0)
		return false;
	tok = tk.next();
	if(tok.text.compare(";")!=0)
		return false;
	return true;
}

bool gpbparse::readinclude(gpbtoken& tk)
{
	string inclu;
	token tok = tk.next();
	if(tok.text.compare("\"")!=0)
		return false;
	tok = tk.next();
	if(tok.tktype != TYPE_IDENTIFIER)
		return false;
	inclu = tok.text;
	vecincludes.push_back(inclu);
	tok = tk.next();
	if(tok.text.compare(".")!=0)
		return false;
	tok = tk.next();
	if(tok.text.compare("proto")!=0)
		return false;
	tok = tk.next();
	if(tok.text.compare("\"")!=0)
		return false;
	tok = tk.next();
	if(tok.text.compare(";")!=0)
		return false;
	return true;
}

bool iskeywords(const string& key){
	static const char* k[]={"optional","repeated"};
	static std::set<string> keys(k,k+2);
	return keys.find(key)!=keys.end();
}
bool isinteger(const string& key){
	for(unsigned i = 0; i < key.length(); ++i){
		if(key[i]<'0'||key[i]>'9')
			return false;
	}
	return true;
}

bool gpbparse::readenum(gpbtoken& tk)
{
	token tok = tk.next();
	if(tok.tktype != TYPE_IDENTIFIER)
		return false;
	string enumname = tok.text;
	tok = tk.next();
	if(tok.text.compare("{")!=0)
		return false;
	do 
	{
		tok = tk.next();
		if(tok.text.compare("}")==0)
			break;
		if(tok.tktype != TYPE_IDENTIFIER)
			return false;
		string enummem = tok.text;
		tok = tk.next();
		if(tok.text.compare("=")!=0)
			return false;
		tok = tk.next();
		if(tok.tktype != TYPE_INTEGER||!isinteger(tok.text))
			return false;
		int flag = atoi(tok.text.c_str());
		tok = tk.next();
		if(tok.text.compare(";")!=0)
			return false;
		mapenums[enumname][flag] = enummem;
	} while (1);
	tok = tk.next();
	if(tok.text.compare(";")!=0)
		return false;
	return true;
}

bool gpbparse::readmessage(gpbtoken& tk, parsemsg& father)
{
	parsemsg thismsg;
	token tok = tk.next();
	if(tok.tktype != TYPE_IDENTIFIER)
		return false;
	if(!father.name.empty())
		thismsg.name = father.name+"_"+tok.text;
	else
		thismsg.name = tok.text;
	thismsg.barename = tok.text;
	tok = tk.next();
	if(tok.text.compare("{")!=0)
		return false;
	do 
	{
		parsemsg::parsemem mem;
		if(!readmsgmem(tk,mem,thismsg))
			return false;
		if(mem.name.compare("}")==0)
			break;
		if(!mem.name.empty()){
			if(thismsg.setchilds.find(mem.type) != thismsg.setchilds.end()){
				mem.type = thismsg.name + "_" + mem.type;
			}
			thismsg.vecmems.push_back(mem);
		}
	} while (1);
	if(!father.name.empty())
		father.setchilds.insert(thismsg.barename);
	vecmsgs.push_back(thismsg);
	return true;
}

bool gpbparse::readmsgmem(gpbtoken& tk, parsemsg::parsemem& mem, parsemsg& father)
{
	token tok = tk.next();
	if(tok.tktype==TYPE_ERROR)
		return false;
	if(tok.tktype==TYPE_END)
		return true;
	if(tok.text.compare("}")==0){
		mem.name = "}";
		return true;
	}else if(tok.text.compare("message")==0){
		return readmessage(tk, father);
	}else if(iskeywords(tok.text)){
		mem.symbol = tok.text;
		tok = tk.next();
		if(tok.tktype != TYPE_IDENTIFIER)
			return false;
	}
	mem.type = tok.text;
	if(tok.text.compare("map")==0){
		tok = tk.next();
		if(tok.text.compare("<")!=0)
			return false;
		tok = tk.next();
		if(tok.tktype != TYPE_IDENTIFIER)
			return false;
		mem.keytype = tok.text;
		tok = tk.next();
		if(tok.text.compare(",")!=0)
			return false;
		tok = tk.next();
		if(tok.tktype != TYPE_IDENTIFIER)
			return false;
		mem.valtype = tok.text;
		tok = tk.next();
		if(tok.text.compare(">")!=0)
			return false;
	}
	return readonemem(tk,mem);
}

bool gpbparse::readonemem(gpbtoken& tk, parsemsg::parsemem& mem)
{
	token tok = tk.next();
	if(tok.tktype != TYPE_IDENTIFIER)
		return false;
	mem.name = tok.text;
	tok = tk.next();
	if(tok.text.compare("=")!=0)
		return false;
	tok = tk.next();
	if(tok.tktype != TYPE_INTEGER||!isinteger(tok.text))
		return false;
	mem.flag = atoi(tok.text.c_str());
	tok = tk.next();
	if(tok.text.compare(";")!=0)
		return false;
	return true;
}
