#ifndef GETOPT_H
#define GETOPT_H
#include <string>
#include <algorithm>

std::string optarg;
std::string found_tokens;

static inline int getopt(int argc, char** argv, std::string tokens)
{
	for (int i = 1; i < argc && found_tokens != tokens; ++i)
	{
		for (int j = 0; j < tokens.length(); ++j)
		{
			std::string this_token = "-";
			this_token = this_token + tokens[j];
			if (argv[i] == this_token &&
				std::find(found_tokens.begin(), found_tokens.end(), tokens[j]) == found_tokens.end())
			{
				optarg = argv[i + 1];
				found_tokens.push_back(tokens[j]);
				return static_cast<int>(tokens[j]);
			}
		}
	}
	return -1;
}

#endif