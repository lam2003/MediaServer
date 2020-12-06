#ifndef SMS_CONFIG_H
#define SMS_CONFIG_H

#include <common/repo_version.h>

#define SERVER_NAME "SMS-" REPO_VERSION "(build in " REPO_DATE ")"

#define DEFAULT_VHOST "__defaultVhost__"
#define VHOST_KEY "vhost"

#define SMS_RTSP_SCHEMA "rtsp"
#define SMS_RTSP_RTP_BEGIN_CHAR '$'
#define SMS_HTTP_CRLF_CHAR_LEN 2
#define SMS_HTTP_CRLF_CHAR "\r\n"
#define SMS_HTTP_SPACE_CHAR " "
#define SMS_HTTP_SEPARATOR_CHAR ": "
#define SMS_HTTP_HEADER_END_LEN 4
#define SMS_HTTP_HEADER_END "\r\n\r\n"
#endif