#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

string replaceAll(string source, const string &target, const string &replacement)
{
	string::size_type pos = 0;
	while ((pos = source.find(target, pos)) != string::npos) 
	{
		source.replace(pos, target.size(), replacement);
		pos += replacement.size() > 0 ? replacement.size() : 1;
	}
	return source;
}

string fileNameFromPath(string path)
{
	// This isn't really platform independent, but close enough...
	path = replaceAll(path, "/", "\\");
	return path.substr(path.rfind("\\") + 1);
}

string stripExtension(string file)
{
	return file.substr(0, file.find("."));
}

string escapeSpecialChars(string input)
{
	// Escape special characters in the string
	input = replaceAll(input, "\\", "\\\\");
	input = replaceAll(input, "\"", "\\\"");
	input = replaceAll(input, "\t", "\\t");
	return input;
}

void printUsage(string progname)
{
	progname = fileNameFromPath(progname);
	cout << "Usage: " << progname << " outfile infile0 [infile1 [infile 2 [...]]]" << endl;
	cout << "Example: " << progname << " bla bla1.txt bla2.txt bla3.txt" << endl;
	cout << "This will generate bla.h and bla.cpp with strings bla1, bla2 and bla3." << endl;
	cout << endl;
}

int main(int argc, const char* argv[])
{
	cout << "== TextToHeader v1.0 ==" << endl;

	if (argc < 3)
	{
		cout << "Not enough args!" << endl;
		printUsage(argv[0]);
		return 1;
	}

	// Future proofing... we may want to add options later
	if  (argc > 1 && *argv[1] == (char)'-')
	{
		cout << "Options not supported!" << endl;
		printUsage(argv[0]);
		return 1;
	}

	// Parse command line
	string outFileName = argv[1];
	vector<string> inFileNames;
	for (int i = 2; i < argc; ++i)
	{
		string arg = argv[i];
		inFileNames.push_back(arg);
	}

	string headerFileName = outFileName + ".h";
	string sourceFileName = outFileName + ".cpp";

	// Header pre and post
	string headerPre = 
		"#pragma once\n"
		"// This file is generated automatically during the build process\n"
		"// DO NOT MODIFY!\n"
		"// Change the corresponding text files instead!\n"
		"\n"
		"namespace strings\n"
		"{\n";
	string headerPost = 
		"}\n";

	// Source pre and post
	string sourcePre = 
		"// This file is generated automatically during the build process\n"
		"// DO NOT MODIFY!\n"
		"// Change the corresponding text files instead!\n"
		"\n"
		"namespace strings\n"
		"{\n";
	string sourcePost = 
		"}\n";

	ofstream header(headerFileName.c_str());
	ofstream source(sourceFileName.c_str());

	header << headerPre;
	source << sourcePre;

	// Process input files
	for (vector<string>::const_iterator it = inFileNames.begin(); 
		it != inFileNames.end(); ++it)
	{
		string file = fileNameFromPath(*it);
		cout << "\t" << file << endl;
		// Generate header code
		string varName = stripExtension(file);
		header << "extern const char *" << varName << ";\n";
		// Load the file and generate source code
		source << "extern const char *" << varName << " = ";
		string line;
		ifstream inFile(it->c_str());
		while (!inFile.eof())
		{
			getline(inFile, line);
			line = escapeSpecialChars(line);
			source << "\n\t\"" << line << "\\n\"";
		}
		inFile.close();
		source << ";\n";
	}

	header << headerPost;
	source << sourcePost;

	header.close();
	source.close();

	cout << "Generated " << headerFileName << ", " << sourceFileName << endl;
	cout << endl;

	return 0;
}
