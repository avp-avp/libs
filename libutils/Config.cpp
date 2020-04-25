#include "stdafx.h"
#include "libutils.h"
#include "Config.h"
#include "Exception.h"
#include <fstream>
#include <iostream>
//#include "strutils.h"


CConfig::CConfig(void)
{
}

CConfig::~CConfig(void)
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	m_Document.Close();
#endif
}

void CConfig::Close()
{
}

void CConfig::Load(string ConfigFileName)
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	//
	//  Parse the XML file, catching any XML exceptions that might propogate
	//  out of it.
	//
	m_Document.Load(ConfigFileName);
#elif defined(USE_JSON)
    try {
		Json::Value root;
		ifstream file(ConfigFileName);
		file >> root;
		m_Document = root;
	} 
	catch (Json::RuntimeError ex) {
		throw CHaException(CHaException::ErrParsingError, "Failed to parse JSON file %s. Error %s", ConfigFileName.c_str(), ex.what());
	}
	/*
	string doc;
	getline(file, doc, (char)EOF);
	if (doc.length() == 0)
		throw CHaException(CHaException::ErrParsingError, "Failed to load JSON file %s. Error %s", ConfigFileName.c_str(), reader.getFormattedErrorMessages().c_str());
	else if (doc[0] == (char)0xef)
		doc = doc.substr(3);
		
	bool parsingSuccessful = reader.parse(doc, root, std::ifstream::binary);
	if (!parsingSuccessful)
		throw CHaException(CHaException::ErrParsingError, "Failed to parse JSON file %s. Error %s", ConfigFileName.c_str(), reader.getFormattedErrorMessages().c_str());
	*/
#else
#	error usupported configuration
#endif
	//

}

string CConfig::getStr(string path, bool bMandatory, string defaultValue)
{
	return m_Document.getStr(path, bMandatory, defaultValue);
}

CConfigItem CConfig::getNode(const char* path)
{
	return m_Document.getNode(path);
}

void CConfig::getList(const char* path, CConfigItemList &list)
{
	 m_Document.getList(path, list);
}
