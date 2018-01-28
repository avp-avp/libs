#pragma once

#include "../libutils/strutils.h"
#include "IPSupervisor.h"
#include "SupervisorClient.h"

typedef void CURL;

class LIBCOMM_API CWebClient
	:public CSupervisorClient
{
public:
	enum ContentType
	{
		None = 0,
		Form
	};

	enum RequestType
	{
		Get,
		Post
	};

	string GetRequestType();
	string GetContentType();
	string GetHeaders();

private: 
	CSupervisor *m_Supervisor;
	virtual void OnRecieve(CConnection* Conn);
	CIPConnection *m_Connection;

protected:
	ContentType m_ContentType;
	RequestType m_RequestType;
	string m_Response;
	bool m_HaveResponse;
	string_map m_Headers, m_Cookie;
	int m_ResponseCode;
	time_t m_RequestTime;
	int m_Timeout;

public:
	CWebClient(CSupervisor *Supervisor, int Timeout=10);
	virtual ~CWebClient();

	void SetSupervisor(CSupervisor *Supervisor){ m_Supervisor = Supervisor; };
	virtual void Send(string url, string host, string Content = "");
	void SetContentType(ContentType type){ m_ContentType = type; };
	void SetRequestType(RequestType type){ m_RequestType = type; };

	bool HaveResponse(){ return m_HaveResponse; };
	int GetResponseCode(){ return m_ResponseCode; };
	string GetResponse(){ return m_Response; };
	void AddResponse(const string &s){ m_Response+=s; };
	void ClearResponse();
	bool isTimeout();
};

#ifdef HAVE_CURL_CURL_H	

class LIBCOMM_API CCurlWebClient
	:public CWebClient
{
private:
	CURL *m_curl;

	static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

public:
	CCurlWebClient(int Timeout=3);
	virtual ~CCurlWebClient();

	virtual void Send(string url, string host, string Content = "");
};

#endif