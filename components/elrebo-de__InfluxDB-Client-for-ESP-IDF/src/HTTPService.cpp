
#include "HTTPService.h"
#include "Platform.h"
#include "Version.h"

#include "util/debug.h"

static const char UserAgent[] PROGMEM = "influxdb-client-arduino/" INFLUXDB_CLIENT_VERSION " (" INFLUXDB_CLIENT_PLATFORM " " INFLUXDB_CLIENT_PLATFORM_VERSION ")";

// This cannot be put to PROGMEM due to the way how it is used
static const char *RetryAfter = "Retry-After";
const char *TransferEncoding = "Transfer-Encoding";

HTTPService::HTTPService(ConnectionInfo *pConnInfo):_pConnInfo(pConnInfo) {
  _apiURL = pConnInfo->serverUrl;
  _apiURL += "/api/v2/";
//*****   bool https = pConnInfo->serverUrl.startsWith("https");
//*****   if(https) {
//*****     WiFiClientSecure *wifiClientSec = new WiFiClientSecure;
//*****     if (pConnInfo->insecure) {
//***** #ifndef ARDUINO_ESP32_RELEASE_1_0_4
//*****       // This works only in ESP32 SDK 1.0.5 and higher
//*****       wifiClientSec->setInsecure();
//***** #endif
//*****     } else if(pConnInfo->certInfo && strlen_P(pConnInfo->certInfo) > 0) {
//*****       wifiClientSec->setCACert(pConnInfo->certInfo);
//*****     }
//*****     _wifiClient = wifiClientSec;
//*****   } else {
//*****     _wifiClient = new WiFiClient;
//*****   }
  if(!_httpClient) {
    _httpClient = new HTTPClient;
  }
  _httpClient->setReuse(_pConnInfo->httpOptions._connectionReuse);

  _httpClient->setUserAgent(FPSTR(UserAgent));
};

HTTPService::~HTTPService() {
  if(_httpClient) {
    delete _httpClient;
    _httpClient = nullptr;
  }
//*****   if(_wifiClient) {
//*****     delete _wifiClient;
//*****     _wifiClient = nullptr;
//*****   }
}


void HTTPService::setHTTPOptions() {
  if(!_httpClient) {
    _httpClient = new HTTPClient;
  }
  _httpClient->setReuse(_pConnInfo->httpOptions._connectionReuse);
  _httpClient->setTimeout(_pConnInfo->httpOptions._httpReadTimeout);
#if defined(ESP32) 
  _httpClient->setConnectTimeout(_pConnInfo->httpOptions._httpReadTimeout);
#endif
}

// parse URL for host and port and call probeMaxFragmentLength

bool HTTPService::beforeRequest(const char *url) {
   if(!_httpClient->begin(url)) {
    _pConnInfo->lastError = F("begin failed");
    return false;
  }
  if(_pConnInfo->authToken.length() > 0) {
    _httpClient->addHeader(F("Authorization"), "Token " + _pConnInfo->authToken);
  }
  const char * headerKeys[] = {RetryAfter, TransferEncoding} ;
  _httpClient->collectHeaders(headerKeys, 2);
  return true;
}

bool HTTPService::doPOST(const char *url, const char *data, const char *contentType, int expectedCode, httpResponseCallback cb) {
  INFLUXDB_CLIENT_DEBUG("[D] POST request - %s, data: %dbytes, type %s\n", url, strlen(data), contentType);
  if(!beforeRequest(url)) {
    return false;
  }
  if(contentType) {
    _httpClient->addHeader(F("Content-Type"), FPSTR(contentType));
  }
  _lastStatusCode = _httpClient->POST((uint8_t *) data, strlen(data));
  return afterRequest(expectedCode, cb);
}

bool HTTPService::doPOST(const char *url, Stream *stream, const char *contentType, int expectedCode, httpResponseCallback cb) {
  INFLUXDB_CLIENT_DEBUG("[D] POST request - %s, data: %dbytes, type %s\n", url, stream->available(), contentType);
  if(!beforeRequest(url)) {
    return false;
  }
  if(contentType) {
    _httpClient->addHeader(F("Content-Type"), FPSTR(contentType));
  }
  _lastStatusCode = _httpClient->sendRequest(HTTP_METHOD_POST, stream, stream->available());
  return afterRequest(expectedCode, cb);
}

bool HTTPService::doGET(const char *url, int expectedCode, httpResponseCallback cb) {
  INFLUXDB_CLIENT_DEBUG("[D] GET request - %s\n", url);
  if(!beforeRequest(url)) {
    return false;
  }
  _lastStatusCode = _httpClient->GET();
  return afterRequest(expectedCode, cb, false);
}

bool HTTPService::doDELETE(const char *url, int expectedCode, httpResponseCallback cb) {
  INFLUXDB_CLIENT_DEBUG("[D] DELETE - %s\n", url);
  if(!beforeRequest(url)) {
    return false;
  }
  _lastStatusCode = _httpClient->sendRequest(HTTP_METHOD_DELETE);
  return afterRequest(expectedCode, cb, false);
}

bool HTTPService::afterRequest(int expectedStatusCode, httpResponseCallback cb,  bool modifyLastConnStatus) {
    if(modifyLastConnStatus) {
        _lastRequestTime = millis();
        INFLUXDB_CLIENT_DEBUG("[D] HTTP status code - %d\n", _lastStatusCode);
        _lastRetryAfter = 0;
        if(_lastStatusCode >= 429) { //retryable server errors
            if(_httpClient->hasHeader(RetryAfter)) {
                _lastRetryAfter = _httpClient->header(RetryAfter).toInt();
                INFLUXDB_CLIENT_DEBUG("[D] Reply after - %d\n", _lastRetryAfter);
            }
        }
    }
    _pConnInfo->lastError = (char *)nullptr;
    bool ret = _lastStatusCode == expectedStatusCode;
    bool endConnection = true;
    if(!ret) {
        if(_lastStatusCode > 0) {
            _pConnInfo->lastError = _httpClient->getString();
            INFLUXDB_CLIENT_DEBUG("[D] Response:\n%s\n", _pConnInfo->lastError.c_str());
        } else {
            _pConnInfo->lastError = _httpClient->errorToString(_lastStatusCode);
            INFLUXDB_CLIENT_DEBUG("[E] Error - %s\n", _pConnInfo->lastError.c_str());
        }
    } else if(cb){
      endConnection = cb(_httpClient);
    }
    if(endConnection) {
        _httpClient->end();
    }
    return ret;
}