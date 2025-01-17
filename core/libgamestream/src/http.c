/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Iwan Timmer
 *
 * Moonlight is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
 */

#include "http.h"
#include "errors.h"

#include <string.h>
#include <curl/curl.h>
#include <pthread.h>

#include <assert.h>

#ifdef __WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

struct HTTP_T {
    CURL *curl;
    int verbosity;
    pthread_mutex_t mutex;
};

static size_t _write_curl(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    PHTTP_DATA mem = (PHTTP_DATA) userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    assert(mem->memory);

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

HTTP http_init(const char *keydir, int verbosity) {
    CURL *curl = curl_easy_init();
    assert(curl);

    char certificateFilePath[4096];
    sprintf(certificateFilePath, "%s%c%s", keydir, PATH_SEPARATOR, CERTIFICATE_FILE_NAME);

    char keyFilePath[4096];
    sprintf(&keyFilePath[0], "%s%c%s", keydir, PATH_SEPARATOR, KEY_FILE_NAME);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
    curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
    curl_easy_setopt(curl, CURLOPT_SSLCERT, certificateFilePath);
    curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
    curl_easy_setopt(curl, CURLOPT_SSLKEY, keyFilePath);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_curl);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_SESSIONID_CACHE, 0L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, verbosity >= 2 ? 1L : 0L);

    struct HTTP_T *http = malloc(sizeof(struct HTTP_T));
    http->curl = curl;
    http->verbosity = verbosity;
    pthread_mutex_init(&http->mutex, NULL);
    return http;
}

int http_request(HTTP http, char *url, PHTTP_DATA data) {
    pthread_mutex_lock(&http->mutex);
    CURL *curl = http->curl;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    if (http->verbosity)
        printf("Request %s\n", url);
    int ret = GS_FAILED;
    if (data->size > 0) {
        free(data->memory);
        data->memory = malloc(1);
        if (data->memory == NULL) {
            ret = GS_OUT_OF_MEMORY;
            goto finish;
        }

        data->size = 0;
    }
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        gs_error = curl_easy_strerror(res);
        ret = GS_FAILED;
        goto finish;
    } else if (data->memory == NULL) {
        ret = GS_OUT_OF_MEMORY;
        goto finish;
    }
    if (http->verbosity)
        printf("Response of %s:\n", url);
    if (http->verbosity >= 2)
        printf("%s\n\n", data->memory);

    ret = GS_OK;
    finish:
    pthread_mutex_unlock(&http->mutex);
    return ret;
}

void http_cleanup(HTTP http) {
    curl_easy_cleanup(http->curl);
    free((void *) http);
}

void http_set_timeout(HTTP http, int timeout) {
    pthread_mutex_lock(&http->mutex);
    curl_easy_setopt(http->curl, CURLOPT_TIMEOUT, timeout);
    pthread_mutex_unlock(&http->mutex);
}

PHTTP_DATA http_create_data() {
    PHTTP_DATA data = malloc(sizeof(HTTP_DATA));
    assert(data);

    data->memory = malloc(1);
    assert(data->memory);
    data->size = 0;

    return data;
}

void http_free_data(PHTTP_DATA data) {
    if (data == NULL) return;
    if (data->memory != NULL)
        free(data->memory);

    free(data);
}
