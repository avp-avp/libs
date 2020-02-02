#include "stdafx.h"
#include "ConfigItem.h"
#include "Exception.h"

#ifdef _LIBUTILS_USE_XML_LIBXML2
	#include "libxml/tree.h"
	#include "libxml/parser.h"
	#include <libxml/xpath.h>
	#include <libxml/xpathInternals.h>
#endif

#ifdef USE_CONFIG

#ifdef _LIBUTILS_USE_XML_LIBXML2
	const char* CConfigItem::CONFIG_EXTENSION = "xml";
#elif defined(USE_JSON)
	const char* CConfigItem::CONFIG_EXTENSION = "json";
#endif


CConfigItemList::CConfigItemList()
:vector<CConfigItem*>()
{
}

CConfigItemList::CConfigItemList(const CConfigItemList& cpy)
:vector<CConfigItem*>()
{
	for (const_iterator i=cpy.begin();i!=cpy.end();i++)
		push_back(new CConfigItem(**i));
}

CConfigItemList::~CConfigItemList()
{
	for (iterator i=begin();i!=end();i++)
		delete (*i);
}

CConfigItem::CConfigItem(void)
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	m_Node = NULL;
#endif
}

CConfigItem::CConfigItem(configNode node)
{
	m_Node = node;
}

CConfigItem::CConfigItem(const CConfigItem& cpy)
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	m_Node = NULL;
#elif defined(USE_JSON)
	m_Node = Json::Value();
#endif

	SetNode(cpy);
}


CConfigItem::~CConfigItem(void)
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
//	if (m_Node)
//		xmlFreeNode(m_Node);
#endif
}

void CConfigItem::ParseXPath(string Path, string &First, string &Other)
{
	int pos = Path.find_first_of('/');
	if (pos>0)
	{
		First = Path.substr(0,pos);
		Other = Path.substr(pos+1);
	}
	else
	{
		First = Path;
		Other = "";
	}
}

bool CConfigItem::isEmpty() 
{ 
#ifdef _LIBUTILS_USE_XML_LIBXML2
	return m_Node == NULL;
#elif defined(USE_JSON)
	return m_Node.isObject() && m_Node.empty() || m_Node.isArray() && m_Node.empty();
#endif
};


string CConfigItem::getStr(string path, bool bMandatory, string defaultValue) const
{
	string First, Other;
	ParseXPath(path, First, Other);

	if (Other.length())
	{
		return getNode(First).getStr(Other, bMandatory, defaultValue);
	}
	else
	{
#ifdef _LIBUTILS_USE_XML_LIBXML2
		char* val = (char*)xmlGetProp(m_Node, BAD_CAST path.c_str());

		if (!val)
		{
			if (bMandatory)
				throw CHaException(CHaException::ErrAttributeNotFound, path);
			else
				return defaultValue;
		}

		string sVal = val;
		xmlFree(val);
		return sVal;
#elif defined(USE_JSON)
		if (path.length() == 0)
		{
			if (m_Node.isString())
				return m_Node.asCString();
			else 
				throw CHaException(CHaException::ErrAttributeNotFound, "attribute '%s' is not string", path.c_str());
		}
		else
		{
			Json::Value val = m_Node[path];

			if (val.isNull() && !bMandatory)
				return defaultValue;
			else if (val.isString())
				return val.asCString();
			else 
				throw CHaException(CHaException::ErrAttributeNotFound, "attribute '%s' is not string", path.c_str());
		}
#endif
	}
}

int CConfigItem::getInt(string path, bool bMandatory, int defaultValue) const
{
	string First, Other;
	ParseXPath(path, First, Other);

	if (Other.length())
	{
		return getNode(First).getInt(Other, bMandatory, defaultValue);
	}
	else
	{
#ifdef _LIBUTILS_USE_XML_LIBXML2
		char* val = (char*)xmlGetProp(m_Node, BAD_CAST path.c_str());

		if (!val)
		{
			if (bMandatory)
				throw CHaException(CHaException::ErrAttributeNotFound, path);
			else
				return defaultValue;
		}

		int iVal = atoi(val);
		xmlFree(val);
		return iVal;
#elif defined(USE_JSON)
		try
		{
			if (path.length() == 0)
				return m_Node.asInt();
			else
			{
				Json::Value val = m_Node[path];

				if (val.isNull() && !bMandatory)
					return defaultValue;
				else
					return val.asInt();
			}
		} catch (std::exception ex) {
			throw CHaException(CHaException::ErrAttributeNotFound, "Value is not a int. Param %s", path.c_str());
		}
#endif
	}
}

CConfigItem CConfigItem::getNode(string path, bool bMandatory) const
{
	string First, Other;
	ParseXPath(path, First, Other);

	if (Other.length())
	{
		CConfigItem parent = getNode(First);

		if (parent.isEmpty())
			return CConfigItem();

		return parent.getNode(Other);
	}
	else
	{
#ifdef _LIBUTILS_USE_XML_LIBXML2
		xmlNodePtr retVal = NULL, cur = m_Node->children;

		while(cur)
		{
			if (cur->type==XML_ELEMENT_NODE && strcmp((char*)cur->name, First.c_str())==0)
			{
				if (retVal)
				{
					return NULL;
				}
				retVal = cur;
			}

			cur = cur->next;
		}
#elif defined(USE_JSON)
		configNode retVal = m_Node[First];
#endif

		return retVal;
	}
}

void CConfigItem::getValues(configValues &values)  const
{
	values.empty();
	string_vector v = m_Node.getMemberNames();

	for_each(string_vector, v, name)
	{
		values[*name] = m_Node[*name];
	}
}


void CConfigItem::getList(string path, CConfigItemList &list)
{
	if (!m_Node)
		return;

//	;

	string First, Other;
	ParseXPath(path, First, Other);

	if (Other.length())
	{
		getNode(First).getList(Other, list);
	}
	else
	{
		list.clear();
#ifdef _LIBUTILS_USE_XML_LIBXML2
		xmlNodePtr cur = m_Node->children;

		while(cur)
		{
			if (First.compare((char*)cur->name)==0)
			{
				list.push_back(new CConfigItem(cur));
			}
			cur = cur->next;
		}

		if (list.size() == 0)
		{
			CConfigItem node = getNode(path + "s");
			if (node.isNode())
			{
				node.getList(path, list);
			}
		}
#elif defined(USE_JSON)
		try {
			configNode values = m_Node[First];
			for (Json::ArrayIndex i=0;i<values.size();i++) {
				list.push_back(new CConfigItem(values[i]));
			}
		} catch (std::exception ex) {
			throw CHaException(CHaException::ErrAttributeNotFound, "Value is not a array. Param %s", path.c_str());
		}
#endif
	}
}

void CConfigItem::SetNode(configNode node)
{
	m_Node = node;
	/*  never free single node
	if (m_Node)
		xmlFreeNode(m_Node);
	m_Node = xmlCopyNode(node, true);
	*/
}

bool CConfigItem::isNode()
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	return m_Node && m_Node->type == XML_ELEMENT_NODE;
#elif defined(USE_JSON)
	return m_Node.isObject();
#endif
};

bool CConfigItem::isStr()
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	return m_Node && m_Node->type != XML_ELEMENT_NODE; // STR=INT for XML
#elif defined(USE_JSON)
	return m_Node.isString();
#endif
};

bool CConfigItem::isInt()
{
#ifdef _LIBUTILS_USE_XML_LIBXML2
	return m_Node && m_Node->type != XML_ELEMENT_NODE;  // STR=INT for XML
#elif defined(USE_JSON)
	return m_Node.isInt();
#endif
};



#endif