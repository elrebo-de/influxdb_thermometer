/**
 * 
 * Options.cpp: InfluxDB Client write options and HTTP options
 * 
 * MIT License
 * 
 * Copyright (c) 2020 InfluxData
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
#include <NoArduino.h>
#include "Print.h"
#include "Options.h"
#include "util/helpers.h"

WriteOptions& WriteOptions::addDefaultTag(const String &name, const String &value) { 
    if(_defaultTags.length() > 0) {
        _defaultTags += ',';
    }
    char *s = escapeKey(name);
    _defaultTags += s;
    delete [] s;
    s = escapeKey(value);
    _defaultTags += '=';
    _defaultTags += s;
    delete [] s;
    return *this; 
}

void WriteOptions::printTo(Print &dest) const {
    printf("WriteOptions:\n");
    printf("\t_precision: %u\n",          (uint8_t)_writePrecision);
    printf("\t_batchSize: %u\n",          _batchSize);
    printf("\t_bufferSize: %u\n",         _bufferSize);
    printf("\t_flushInterval: %u\n",      _flushInterval);
    printf("\t_retryInterval: %u\n",      _retryInterval);
    printf("\t_maxRetryInterval: %u\n",   _maxRetryInterval);
    printf("\t_maxRetryAttempts: %u\n",   _maxRetryAttempts);
    printf("\t_defaultTags: %s\n",        _defaultTags.c_str());
    printf("\t_useServerTimestamp: %i\n", _useServerTimestamp);
}
