#ifndef __TC_HTTP_H_
#define __TC_HTTP_H_

#include "util/tc_common.h"
#include <map>
#include <sstream>
#include <cassert>
#include <vector>
#include <list>
#include <string.h>

using namespace std;
namespace tars
{
/////////////////////////////////////////////////
/**
 * @file tc_http.h
 * @brief  http��.
 *
 * ����TC_HttpRequest��TC_HttpResponse�����ࣻ
 *
 * ֧��GET HEAD POST OPTIONS������HTTP������֧�֣�
 *
 * ͨ��TC_HttpRequest::checkRequest�ж�http�����Ƿ����ꣻ
 *
 * ��TC_ClientSocket��ϣ�֧��ͬ������http������֧��http��chunk���룻
 *
 * ����http����ʱ�����̰߳�ȫ
 *
 * 1 ֧��httpЭ��Ľ���,֧��GET/POSTģʽ
 *
 * 2 ��Ƕ֧�ֶ�chunk����Ľ���
 *
 * 3 ֧��ͬ��http�������Ӧ(ֻ֧�ֶ�����,��֧��keep-alive)
 *
 * 4 ֧��http��Ӧ�������������
 *
 * 5 ����Cookie������
 *
 * @author  jarodruan@tencent.com,markzhang@tencent.com
 */
/////////////////////////////////////////////////

/**
 * @brief  �򵥵�URL������.
 *
 * eg��
 * ftp://user:password@www.qq.com:8080/abc/dev/query?a=b&c=3#ref.
 *
 * scheme: ftp.
 *
 * user:user.
 *
 * pass:password.
 *
 * domain:www.qq.com.
 *
 * port:8080.
 *
 * path:/abc/dev/query.
 *
 * query:a=b&c=3.
 *
 * ref:ref.
 *
 * request:/abc/dev/query?a=b&c=3#ref.
 *
 * relativePath:/abc/dev/.
 *
 * rootPath:ftp://user:password@www.qq.com:8080/.
 */

class TC_URL
{
public:

    /**
     *  @brief  URL����
     */
    enum URL_TYPE
    {
        HTTP,
        HTTPS,
        FTP
    };

    /**
     *  @brief  ���캯��
     */
    TC_URL() : _iURLType(HTTP)
    {
    }

    /**
	 * @brief  ����URL, url�����Ǿ���·��.
	 *
     * @throws       TC_URL_Exception
	 * @param sURL  �����URL��
	 * @return      �����ɹ�����true�����򷵻�false
     */
    bool parseURL(const string &sURL);

    /**
     * @brief   ���
     */
    void clear();

    /**
     * @brief  ��ȡscheme.
     *
     * @return const string&���͵�scheme
     */
    string getScheme() const;

    /**
     * @brief  ��ȡ�û���.
     *
     * @return const string& ���û���
     */
    string getUserName() const;

    /**
     * @brief  ��ȡ����.
     *
     * @return const string& ������
     */
    string getPassword() const;

    /**
     * @brief  ��ȡ����.
     *
     * @return const string& ������
     */
    string getDomain() const;

    /**
     * @brief   ��ȡ�˿�.
     *
     * @return const string& ���˿�
     */
    string getPort() const;

    /**
     * @brief   �Ƿ���ȱʡ�Ķ˿�.
     *
     * @return bool�ͣ���ȱʡ�Ķ˿ڷ���true�����򷵻�false
     */
    bool isDefaultPort() const;

    /**
     * @brief   ��ȡ·��.
     *
     * @return const string& ��·��ֵ
     */
    string getPath() const;

    /**
     * @brief   ��ȡ��ѯ����.
     *
     * @return string ����ѯ���
     */
    string getQuery() const;

    /**
	 * @brief ȡ��Request��������Host,
	 *  	  ����http://www.qq.com/abc?a=b#ref��Ϊ:/abc?a=b#ref
     * @return ��������������request
     */
    string getRequest() const;

    /**
     * @brief  ��ȡRef.
     *
     * @return Ref��
     */
    string getRef() const;

    /**
     * @brief   �Ƿ���Ч.
     *
     * @return ��Ч����true�����򷵻�false
     */
    bool isValid() const;

    /**
     * @brief   ��ȡ������URL.
     *
     * @return �������URL
     */
    string getURL() const;

    /**
     * @brief   ��ȡURL����.
     *
     * @return  URL����
     */
    int getType() const { return _iURLType; }

    /**
     * @brief   ��ȡ���·��.
     *
     * @return ���·���ַ���
     */
    string getRelativePath() const;

    /**
     * @brief   ��ȡ��·��.
     *
     * @return  ��·���ַ���
     */
    string getRootPath() const;

	/**
	 * @brief ����·��.
	 *
     * �Ե�ǰURL����·��.
	 *
     * 1 http://www.qq.com/a/b?a=b, /test/abc => http://www.qq.com/test/abc
	 *
     * 2 http://www.qq.com/a/b?a=b, test/abc => http://www.qq.com/a/b/test/abc
	 *
	 * 3 http://www.qq.com/a/b?a=b, ../test/abc   => http://www.qq.com/a/test/abc
	 *
	 * @param  sRelativeURL: ��Ե�ǰURL�ĵ�ַ
	 * @return ���ص������URL
     */
    TC_URL buildWithRelativePath(const string &sRelativeURL) const;

    /**
     * @brief ������.
     */
    void specialize();

protected:
    /**
     * @brief  ����URL.
     *
     * @return URL��
     */
    string toURL();

    /**
	 * @brief ��ȡ��.
	 *
	 * @param frag
     * @return string
     */
    //string getFragment(const string& frag) const;

    /**
     * @brief  ���͵��ַ�����ת��
     *
     * @return string��ת������ַ���
     */
    string type2String() const;

    /**
     * @brief  ��ȡЭ���ȱʡ�˿�.
     *
     * @return string��ȱʡ�˿�
     */
    string getDefaultPort() const;

    /**
	 * @brief  ��URL.
	 *
	 * @param sPath ԭ·��
     * @return      �򻯺��URL
     */
    string simplePath(const string &sPath) const;

protected:

	/**
     * URL����
     */
    int                 _iURLType;

	string				_sScheme;
	string				_sUser;
	string				_sPass;
	string				_sDomain;
	string				_sPort;
	string				_sPath;
	string				_sQuery;
	string				_sRef;
    /**
     * �������URL
     */
    string              _sURL;
};

/**
 * @brief  httpЭ�������, �������Ӧ���ڸ����н���
 */
class TC_Http
{
public:
    /**
     * @brief  ���캯��
     */
    TC_Http()
    {
        TC_Http::reset();
        setConnection("close");     //Ĭ�Ͼ��ö�����
    }

	/**
     * @brief  ������ͷ����Сд
     */
    struct CmpCase
    {
        bool operator()(const string &s1, const string &s2) const
        {
            //return TC_Common::upper(s1) < TC_Common::upper(s2);
			if(strcasecmp(s1.c_str(), s2.c_str()) < 0)
			{
				return true;
			}
			else
			{
				return false;
			}
        }
    };

    typedef multimap<string, string, CmpCase> http_header_type;

    /**
	 * @brief  ɾ��ͷ��.
	 *
     * @param sHeader:ͷ����Ϣ
     */
    void eraseHeader(const string &sHeader) { _headers.erase(sHeader); }

    /**
	 * @brief  ���� Cache-Control.
	 *
     * @param sCacheControl
     */
    void setCacheControl(const string &sCacheControl){setHeader("Cache-Control", sCacheControl);}

    /**
	 * @brief  ���� Connection.
	 *
     * @param sConnection��Connection��Ϣ
     */
    void setConnection(const string &sConnection){setHeader("Connection", sConnection);}

    /**
	 * @brief  ���� Content-Type.
	 *
     * @param sContentType��Content��Ϣ
     */
    void setContentType(const string &sContentType){setHeader("Content-Type", sContentType);}

    /**
	 * @brief  �������ݳ���.
	 *
     * @param iContentLength��Ҫ���õĳ���ֵ
     */
    void setContentLength(size_t iContentLength)
    {
        setHeader("Content-Length", TC_Common::tostr(iContentLength));
    }

    /**
	 * @brief  ���� Referer.
	 *
     * @param sReferer��Referer��Ϣ
     */
    void setReferer(const string &sReferer){setHeader("Referer", sReferer);}

    /**
	 * @brief  ���� Host.
	 *
     * @param sHost ���� www.qq.com:80
     */
    void setHost(const string &sHost){setHeader("Host", sHost);}

    /**
	 * @brief  ���� Accept-Encoding.
	 *
     * @param sAcceptEncoding, gzip, compress, deflate, identity
     */
    void setAcceptEncoding(const string &sAcceptEncoding){setHeader("Accept-Encoding", sAcceptEncoding);}

    /**
	 * @brief  ���� Accept-Language.
	 *
     * @param sAcceptLanguage��Accept-Language��Ϣ
     */
    void setAcceptLanguage(const string &sAcceptLanguage){setHeader("Accept-Language", sAcceptLanguage);}

    /**
	 * @brief  ���� Accept.
	 *
     * @param sAccept Accept��Ϣ
     */
    void setAccept(const string &sAccept){setHeader("Accept", sAccept);}

    /**
	 * @brief  ���� Transfer-Encoding.
	 *
     * @param sTransferEncoding��Transfer-Encoding��Ϣ
     */
    void setTransferEncoding(const string& sTransferEncoding) {setHeader("Transfer-Encoding", sTransferEncoding); }

    /**
	 * @brief  ����header��
	 * ���õ�ֵ��ʹ���Ѿ��е�get/set��������
	 * (��Set-Cookie&Cookie��,ͷ���ֶβ����ظ�)
     * @param sHeadName  header������
     * @param sHeadValue header��ֵ
     */
    void setHeader(const string &sHeadName, const string &sHeadValue)
    {
        //Set-Cookie��Cookie�����ж��ͷ
		const char * pStr1 = "SET-COOKIE";
		const char * pStr2 = "COOKIE";//ԭ����COOKIEֻ��һ���������м��������⣬����
		if((strcasecmp(sHeadName.c_str(), pStr1) != 0) && (strcasecmp(sHeadName.c_str(), pStr2) != 0))
		{
			_headers.erase(sHeadName);
		}
       /* if(TC_Common::upper(sHeadName) != "SET-COOKIE" && TC_Common::upper(sHeadName) != "COOKIE")
        {
            _headers.erase(sHeadName);
        }*/
        _headers.insert(multimap<string, string>::value_type(sHeadName, sHeadValue));
    }

    /**
	 * @brief  ����header�����õ�ֵ��ʹ���Ѿ��е�get/set������
	 *  	   (ͷ���ֶο����ظ�)
     * @param sHeadName   header������
     * @param sHeadValue  header��ֵ
     */
    void setHeaderMulti(const string &sHeadName, const string &sHeadValue)
    {
        _headers.insert(multimap<string, string>::value_type(sHeadName, sHeadValue));
    }

    /**
	 * @brief  ��ȡͷ(�ظ���Ҳ��ȡ).
	 *
	 * @param sHeadName  header������
     * @return           vector<string>header��ֵ
     */
    vector<string> getHeaderMulti(const string &sHeadName) const;

    /**
     * @brief  ��ȡhttp���ݳ���.
     *
     * @return http���ݳ���.
     */
    size_t getContentLength() const;

    /**
     * @brief  ��ȡhttp����Host.
     *
     * @return host������Ϣ
     */
    string getHost() const;

    /**
     * @brief ��ȡhttpͷ������.
     *
     * @return ͷ������
     */
    size_t getHeadLength() const { return _headLength; }

    /**
     * @brief ��ȡhttp����.
     *
     * @return http���ݴ�
     */
    string getContent() const { return _content; }

    /**
	 * @brief ����http body(Ĭ�ϲ��޸�content-length).
	 *
     * @param content               http body����
     * @param bUpdateContentLength  �Ƿ����ContentLength
     */
    void setContent(const string &content, bool bUpdateContentLength = false)
    {
        _content = content;

        if(bUpdateContentLength)
        {
            eraseHeader("Content-Length");
            if(_content.length() > 0)
                setContentLength(_content.length());
        }
    }

    /**
     * @brief ��ȡ��������.
     *
     * @return string
     */
    string getContentType() const;

    /**
	 * @brief ��ȡhttpͷ��������(Set-Cookie��Cookie��Ҫ�������ȡ,
	 *  	  �����к�����ȡ)
	 * @param sHeader header����
     * @return        header�������Ϣ
     */
    string getHeader(const string& sHeader) const;

    /**
     * @brief ��ȡhttpͷ��map.
     *
     * @return http_header_type&
     */
     const http_header_type& getHeaders() const{return _headers;}

     /**
      * @brief ����
      */
     void reset();

    /**
	 * @brief ��ȡһ��.
	 *
	 * @param ppChar  ��ȡλ��ָ��
     * @return string ��ȡ������
     */
    static string getLine(const char** ppChar);

	/**
	 * @brief ��ȡһ��.
	 *
     * @param ppChar   ��ȡλ��ָ��
	 * @param iBufLen  ����
     * @return string  ��ȡ������
     */
    static string getLine(const char** ppChar, int iBufLen);

    /**
     * @brief ����ͷ���ַ���(��������һ��).
     *
     * @return string��ͷ���ַ���
     */
    string genHeader() const;

    /**
     * @brief ��httpԭʼ���ݰ��Ƿ���chunk�����ʽ.
     *
     * @return bool����������true�����򷵻�false
     */
    bool isChunked() const { return _bIsChunked; }

    /**
	 * @brief ��������head����������һ��,
	 *  	  ��һ�����������Ӧ����һ���� ��������ݽ���Ϊmap��ʽ
	 * @param szBuffer
     * @return const char*, ƫ�Ƶ�ָ��
     */
    static const char* parseHeader(const char* szBuffer, http_header_type &sHeader);

protected:

    /**
     * httpͷ�����б���
     */
    http_header_type    _headers;

    /**
     * httpͷ������
     */
    size_t              _headLength;

    /**
     * httpͷ������
     */
    string              _content;

    /**
     * �Ƿ���chunkģʽ
     */
    bool                _bIsChunked;
};

/********************* TC_HttpCookie***********************/

/**
 * @brief �򵥵�Cookie������.
 */
class TC_HttpCookie
{
public:
    typedef map<string, string, TC_Http::CmpCase> http_cookie_data;

    struct Cookie
    {
        http_cookie_data    _data;
        string              _domain;
        string              _path;
        time_t              _expires;       //����0��ʾֻ�ڵ�ǰ�ػ���Ч
        bool                _isSecure;
    };

    /**
     * @brief �������
     */
    void clear();

    /**
	 * @brief ����Cookie����������.
	 *
     * @param sCookieRspURL ������Cookie HTTP��Ӧ��URL
     * @param vCookies      set-Cookie�ַ���
     */
    void addCookie(const string &sCookieRspURL, const vector<string> &vCookies);

    /**
	 * @brief ����Cookie��������.
	 *
     * @param cookie
     */
    void addCookie(const Cookie &cookie);

    /**
	 * @brief ����Cookie��������.
	 *
     * @param cookie
     */
    void addCookie(const list<Cookie> &cookie);

    /**
	 * @brief ��ȡĳ��url��cookie��ֵ�ԣ�ȥ����Domain,Path���ֶ�
     * @param sReqURL ��Ҫƥ���URL
     * @param cookies
     */
    void getCookieForURL(const string &sReqURL, list<Cookie> &cookies);

    /**
	 * @brief ��ȡĳ��url��cookie��ֵ�ԣ�ȥ����Domain,Path���ֶ�
     * @param sReqURL  ��Ҫƥ���URL
     * @param sCookie
     * @param string
     */
    void getCookieForURL(const string &sReqURL, string &sCookie);

    /**
	 * @brief ƥ��������sCookieDomain���ж����
     * sCookieDomainδ��.��ͷ, ��sCookieDomain=sDomain
     * sCookieDomain��.��ͷ,��sDomain�ұ�ƥ��sDomain,sDomain��sCookieDomain������һ��.
	 * (x.y.z.com ƥ�� .y.z.com, x.y.z.com ��ƥ��.z.com )
     * @param sCookieDomain
	 * @param sDomain       ����
     * @return bool         ƥ��ɹ�����true�����򷵻�false
     */
    static bool matchDomain(const string &sCookieDomain, const string &sDomain);

    /**
	 * @brief ƥ��·��.
     *
     * @param sCookiePath sPath��ǰ׺
	 * @param sPath       ·��
     * @return            ƥ��·���ĳ���
     */
    static size_t matchPath(const string &sCookiePath, const string &sPath);

    /**
     * @brief ��ȡ���е�Cookie.
     *
     * @return list<Cookie>&
     */
    list<Cookie> getAllCookie();

    /**
	 * @brief  ɾ�����ڵ�Cookie�����������뵱ǰ�ػ���Cookie��ɾ��
     * @param t       ������ǰʱ���cookie��ɾ��
	 * @param bErase true:�����ڵ�ǰ�ػ���CookieҲɾ��
	 *  			 false:�����ڵ�ǰ�ػ���Cookie��ɾ��
     */
    void deleteExpires(time_t t, bool bErase = false);

    /**
	 * @brief  ��������û�й��ڵ�Cookie����Expires�ֶε�Cookie��
	 * �����������뵱ǰ�ػ���Cookie�ǲ����ص�ͨ�������������л�
	 * @param t ������ǰʱ���cookie��ɾ��
     * @return  list<Cookie>:���з���������cookie
     */
    list<Cookie> getSerializeCookie(time_t t);

protected:
    /**
	 * @brief  �ж�Cookie����Ч.
	 *
	 * @param tURL
     * @return size_t
     */
    size_t isCookieMatch(const Cookie &cookie, const TC_URL &tURL) const;

    /**
	 * @brief  ���Cookie�Ƿ���ڣ���ǰ�ػ��Ĳ������(expires=0)
	 * @param cookie ������cookie����
     * @return bool  ���ڷ���true�����򷵻�false
     */
    bool isCookieExpires(const Cookie &cookie) const;

    /**
	 * @brief  ���Cookie.
	 *
     * @param cookie  Ҫ��ӵ�cookie����
     * @param cookies Ҫ����ӵ�list����
     */
    void addCookie(const Cookie &cookie, list<Cookie> &cookies);

    /**
	 * @brief ����Domain.
	 *
	 * @param sDomain  ����ǰ��Domain
     * @return string �������Domain
     */
    bool fixDomain(const string &sDomain, string &sFixDomain);

protected:

    /**
	 * @brief  ����Cookie.
	 *
     * key:domain+path
     */
    list<Cookie> _cookies;
};

/********************* TC_HttpResponse ***********************/

class TC_HttpResponse : public TC_Http
{
public:

    /**
     * @brief ����
     */
    TC_HttpResponse()
    {
        TC_HttpResponse::reset();
    }

    /**
     * @brief reset��Ӧ��
     */
    void reset();

    /**
	 * @brief ����decode,�����buffer���Զ��ڽ��������б��������
     * ����decode֮ǰ����reset��
     * (������յ�bufferֱ����ӵ�sBuffer���漴��, Ȼ����������)
     * (�ܹ�����������TC_HttpResponse���Զ���sBuffer����������ֱ�����������ϻ��߽�������true)
     * @param buffer
     * @throws TC_HttpResponse_Exception, ��֧�ֵ�httpЭ��, �׳��쳣
	 * @return true:������һ��������buffer
	 *  	  false:����Ҫ������������������������ر����ӵ�ģʽ��
	 *  	  , Ҳ���ܲ���Ҫ�ٽ�����
     */
    bool incrementDecode(string &sBuffer);

    /**
	 * @brief ����httpӦ��(����string��ʽ) ��
	 * ע��:���httpͷ��û��Content-Length�ҷ�chunkģʽ, �򷵻�true
	 * ��Ҫ������ж�(http����������ر�����Ҳ���Ƿ���http��Ӧ������)
     * @param sBuffer
     * @return bool, sBuffer�Ƿ���������http����
     */
    bool decode(const string &sBuffer);

    /**
	 *  @brief ����httpӦ��
     * ע��:���httpͷ��û��Content-Length�ҷ�chunkģʽ, �򷵻�true
     * ��Ҫ������ж�(http����������ر�����Ҳ���Ƿ���http��Ӧ������)
     * @param sBuffer
     * @return bool, sBuffer�Ƿ���������http����
     */
    bool decode(const char *sBuffer, size_t iLength);

    /**
	 * @brief ����Ӧ���(����string��ʽ).
	 *
	 * @param sBuffer
     * @return string ������Ӧ���
     */
    string encode() const;

    /**
	 * @brief ����Ӧ���(����vector<char>��ʽ).
	 *
	 * @param sBuffer
     * @return string ������Ӧ���(vector<char>��ʽ��)
     */
    void encode(vector<char> &sBuffer) const;

    /**
     * @brief ��ȡ��һ��(HTTP/1.1 200 OK).
     *
     * @return string ��ȡ������
     */
    string getResponseHeaderLine() const { return _headerLine; }

    /**
     * @brief ��ȡHTTP��Ӧ״̬(����200).
     *
     * @return int
     */
    int  getStatus() const {return _status; }

    /**
	 * @brief ����״̬.
	 *
     * @param status ״ֵ̬
     */
    void setStatus(int status) { _status = status; }

    /**
     * @brief ��ȡ����(����OK).
     *
     * @return string ������Ϣ
     */
    string getAbout() const { return _about; }

    /**
	 * @brief ��������.
	 *
     * @param about ������Ϣ
     */
    void setAbout(const string &about) { _about = about; }

    /**
     * @brief ��ȡ�汾, ����HTTP/1.1   .
     *
     * @return string �汾��Ϣ
     */
    string getVersion() const { return _version; }

    /**
	 * @brief ����ģʽ:HTTP/1.1(Ĭ��)����HTTP/1.0  .
	 *
     * @param sVersion
     */
    void setVersion(const string &sVersion) { _version = sVersion; }

    /**
	 * @brief ����Ӧ��״̬(����string��ʽ).
	 *
     * @param status ״̬��
     * @param about ������Ϣ
     * @param sBody postЭ��body������
     */
    void setResponse(int status = 200, const string& about = "OK", const string& sBody = "");

    /**
	 * @brief ����Ӧ��״̬.
	 *
     * @param status ״̬��
     * @param about  ������Ϣ
     * @param sBuffer postЭ��body������
     * @param iLength sBuffer����
     */
    void setResponse(int status, const string& about, const char *sBuffer, size_t iLength);

    /**
	 * @brief ����Ӧ��, ȱʡstatus=200, about=OK.
	 *
     * @param sBuffer Ӧ������
     * @param iLength sBuffer����
     */
    void setResponse(const char *sBuffer, size_t iLength);

    /**
	 * @brief ���÷���.
	 *
     * @param sServer �������Ϣ
     */
    void setServer(const string &sServer){setHeader("Server", sServer);}

    /**
	 * @brief ����Set-Cookie.
	 *
     * @param sCookie Cookie��Ϣ
     */
    void setSetCookie(const string &sCookie){setHeader("Set-Cookie", sCookie);}

    /**
     * @brief ��ȡSet-Cookie.
     *
     * @return vector<string>
     */
    vector<string> getSetCookie() const;

    /**
	 * @brief ����Ӧ��ͷ.
	 *
	 * @param szBuffer Ӧ��ͷ��Ϣ
     * @return
     */
    void parseResponseHeader(const char* szBuffer);

protected:

    /**
     * Ӧ��״̬
     */
    int     _status;

    /**
     * Ӧ������
     */
    string  _about;

    /**
     * ��ȡ�汾
     */
    string  _version;

    /**
     * ��ȡ��һ��
     */
    string  _headerLine;

    /**
     * ��ʱ��content length
     */
    size_t  _iTmpContentLength;
};

/********************* TC_HttpRequest ***********************/

class TC_HttpRequest : public TC_Http
{
public:

    ///////////////////////////////////////////////////////////////////
    TC_HttpRequest()
    {
        TC_HttpRequest::reset();
        setUserAgent("TC_Http");
    }

    /**
     * @brief ����Э������
     */
    enum
    {
        REQUEST_GET,
        REQUEST_POST,
        REQUEST_OPTIONS,
        REQUEST_HEAD
    };

    /**
	 * @brief ���http�����Ƿ���ȫ.
	 *
     * @param sBuffer http����
     * @throws TC_HttpRequest_Exception, ��֧�ֵ�httpЭ��, �׳��쳣
     * @return  true: ��ȫ, false:��ȫ
     */
    static bool checkRequest(const char* sBuffer, size_t len);

    /**
     * @brief ����
     */
    void reset();

    /**
	 * @brief ���� User-Agent.
	 *
     * @param sUserAgent
     */
    void setUserAgent(const string &sUserAgent){setHeader("User-Agent", sUserAgent);}

    /**
	 * @brief ���� Cookie.
	 *
     * @param sCookie
     */
    void setCookie(const string &sCookie){setHeader("Cookie", sCookie);}

    /**
     * @brief ��ȡԭʼCookie��.
     *
     * @return vector<string>
     */
    vector<string> getCookie();

    /**
	 * @brief ����http����, �������HTTP�������׳��쳣.
	 *
     * @param sBuffer Ҫ������http����
	 * @return        sBuffer�Ƿ���������http����
	 * @throw         TC_HttpRequest_Exception
     */
    bool decode(const string &sBuffer);

    /**
	 * @brief ����http����,
	 *  	  �������HTTP�������׳��쳣(����vector<char>��ʽ).
	 *
     * @param sBuffer http����
     * @param iLength ����
     * @throw         TC_HttpRequest_Exception
     * @return        sBuffer�Ƿ���������http����
     */
    bool decode(const char *sBuffer, size_t iLength);

    /**
	 * @brief ��������(����string��ʽ).
	 *
	 * @param sUrl
     * @return string
     */
    string encode();

    /**
	 * @brief ��������(����vector<char>��ʽ).
	 *
     * @param buffer��������
     */
    void encode(vector<char> &buffer);

    /**
	 * @brief ����Get�����.
	 *
     * @param sUrl         ����:http://www.qq.com/query?a=b&c=d
     * @param bCreateHost  �Ƿ��´���ͷ����Host��Ϣ
     *                     (Ĭ��, �����Host��Ϣ��, �Ͳ�����)
     *                     (ע��, ����encode��ʱ�򴴽���)
     */
    void setGetRequest(const string &sUrl, bool bNewCreateHost = false);

	/**
	 * @brief ����Head�����.
	 *
	 * @param sUrl         ����:http://www.qq.com/query?a=b&c=d
	 * @param bCreateHost  �Ƿ��´���ͷ����Host��Ϣ
	 *                     (Ĭ��, �����Host��Ϣ��, �Ͳ�����)
	 *                     (ע��, ����encode��ʱ�򴴽���)
	 */
	void setHeadRequest(const string &sUrl, bool bNewCreateHost = false);

    /**
	 * @brief ����POST�����(����string��ʽ).
	 *
     * @param sUrl        ����:http://www.qq.com/query
     * @param sPostBody   ���requestType��GET, ��sPostBody��Ч
     * @param bCreateHost �Ƿ��´���ͷ����Host��Ϣ
     *                     (Ĭ��, �����Host��Ϣ��, �Ͳ�����)
     *                     (ע��, ����encode��ʱ�򴴽���)
     */
    void setPostRequest(const string &sUrl, const string &sPostBody, bool bNewCreateHost = false);

    /**
	 * @brief ����POST�����(����vector<char>��ʽ).
	 *
     * @param sUrl        ����:http://www.qq.com/query
     * @param sPostBody   ���requestType��GET, ��sPostBody��Ч
     * @param bCreateHost �Ƿ��´���ͷ����Host��Ϣ
     *                     (Ĭ��, �����Host��Ϣ��, �Ͳ�����)
     *                     (ע��, ����encode��ʱ�򴴽���)
     */
    void setPostRequest(const string &sUrl, const char *sBuffer, size_t iLength, bool bNewCreateHost = false);

    /**
	 * @brief ����Get�����.
	 *
     * @param sUrl         ����:http://www.qq.com/query?a=b&c=d
     * @param bCreateHost  �Ƿ��´���ͷ����Host��Ϣ
     *                     (Ĭ��, �����Host��Ϣ��, �Ͳ�����)
     *                     (ע��, ����encode��ʱ�򴴽���)
     */
    void setOptionsRequest(const string &sUrl, bool bNewCreateHost = false);

    /**
	 * @brief ��ȡurl����ĵ�ַ�Ͷ˿�.
	 *
     * @param sHost
     * @param iPort
     */
    void getHostPort(string &sDomain, uint32_t &iPort);

    /**
     * @brief �Ƿ���GET����.
     *
     * @return ��GET���󷵻�true�����򷵻�false
     */
    bool isGET() const { return _requestType == REQUEST_GET; }

    /**
	 * @brief �Ƿ���HEAD����.
	 *
	 * @return ��HEAD���󷵻�true�����򷵻�false
	 */
	bool isHEAD() const { return _requestType == REQUEST_HEAD; }

    /**
     * @brief �Ƿ���POST����.
     *
     * @return ��GET���󷵻�true�����򷵻�false
     */
    bool isPOST() const { return _requestType == REQUEST_POST; }

    /**
     * @brief �Ƿ���OPTIONS����.
     *
     * @return ��GET���󷵻�true�����򷵻�false
     */
    bool isOPTIONS() const { return _requestType == REQUEST_OPTIONS; }

    /**
     * @brief ��ȡ�����URL.
     *
     * @return const TC_URL&
     */
    const TC_URL &getURL() const { return _httpURL; }

    /**
     * @brief ��ȡ������http����.
     *
     * @return http����
     */
    string getOriginRequest() const { return _httpURL.getURL(); }

    /**
	 * @brief ��ȡhttp�����url��������Host,
	 *  	  ����http://www.qq.com/abc?a=b, ��Ϊ:/abc?a=b
     * @return http�����url
     */
    string getRequest() const { return _httpURL.getRequest(); }

    /**
	 * @brief ��ȡhttp�����url����, ��?ǰ�棬������Host,
	 *  	  ����http://www.qq.com/abc?a=b#def, ��Ϊ:/abc
	 * @return http�����url����
	 * */
    string getRequestUrl() const { return _httpURL.getPath(); }

    /**
	 * @brief ��ȡhttp����url��Ĳ������֣���?����#ǰ�沿�ֲ�
	 *  	  ����Host, ����http://www.qq.com/abc?a=b#def, ��Ϊ:a=b
     * @return http����url��Ĳ�������
     */
    string getRequestParam() const { return _httpURL.getQuery(); }

    /**
	 * @brief ��������ͷ��.
	 *
	 * @param szBuffer ����ͷ��
     * @return size_t
     */
    size_t parseRequestHeader(const char* szBuffer);

    /**
	 * @brief �������͵��ַ���.
	 *
	 * @param iRequestType  ����
     * @return              ��������ַ���
     */
    string requestType2str(int iRequestType) const;

protected:

    /**
	 * @brief ��http�������.
	 *
     * @param sUrl         ��Ҫ���б����http����
	 * @param iRequestType �����������
     * @return void
     */
    void encode(int iRequestType, ostream &os);

    /**
     * @brief ����URL
     */
    void parseURL(const string& sUrl);

protected:

    /**
     * ����URL
     */
    TC_URL             _httpURL;

    /**
     * ��������
     */
    int                _requestType;
};


}
#endif

