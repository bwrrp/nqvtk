#include <iostream>
#include <string>
#include <vector>

using namespace std;

string fileNameFromPath(string path)
{
	// This isn't really platform independent, but close enough...
	string::size_type pos = 0;
	while ((pos = path.find("/", pos)) != string::npos) 
	{
		path.replace(pos, 1, "\\");
		pos++;
	}
	return path.substr(path.rfind("\\") + 1);
}

void printUsage(string progname)
{
	progname = fileNameFromPath(progname);
	cout << "Usage: " << progname << " outfile infile0 [infile1 [infile 2 [...]]]" << endl;
}

int main(int argc, const char* argv[])
{
	cout << "== TextToHeader v1.0 ==" << endl;

	if (argc < 3)
	{
		cout << "Not enough args!" << endl;
		printUsage(string(argv[0]));
		return 1;
	}

	// Future proofing... we may want to add options later
	if  (argc > 1 && argv[1][0] == "-")
	{
		cout << "Options not supported!" << endl;
		printUsage(string(argv[0]));
		return 1;
	}

	// Parse command line
	string outFileName = argv[1];
	vector<string> inFileNames;
	for (int i = 2; i < argc; ++i)
	{
		string arg = string(argv[i]);
		cout << "arg " << i << ": " << arg << endl;
		inFileNames.push_back(arg);
	}

	// TODO: load inFiles
	// TODO: generate C++ header
	// TODO: write to outFile

	return 0;
}
