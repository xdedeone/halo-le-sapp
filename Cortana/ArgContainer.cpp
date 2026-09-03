#include "ArgContainer.h"

using namespace std;

ArgContainer::ArgContainer(const string &arg)
{
	argc = 0;
	size_t word_start = 0;
	size_t closing;
	while(true)
    {
		if(arg[word_start] == '"')
		{
			word_start++;
			closing = arg.find_first_of('"', word_start);
			if(closing == -1)
				closing = arg.length();
		}
		else if(arg[word_start] == '\'')
		{
			word_start++;
			closing = arg.find_first_of('\'', word_start);
			if(closing == -1)
				closing = arg.length();
		}
		else
		{
			closing = arg.find_first_of(' ', word_start);
			if(closing == -1)
				closing = arg.length();
		}

		args.emplace_back(arg.substr(word_start, closing - word_start));
		argc++;
		word_start = arg.find_first_not_of(' ', closing + 1);
		if(word_start == -1)
			break;

		if(word_start + 2 <= arg.length() && (arg[word_start+1] == '"' || arg[word_start+1] == '\''))
			word_start++;
	}

}

const string emptyarg; 
const string& ArgContainer::operator[](size_t i) const
{
	if(i < args.size())
		return args[i];

	return emptyarg;
}