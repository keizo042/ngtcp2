/*
 * ngtcp2
 *
 * Copyright (c) 2017 ngtcp2 contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "ngtcp2_err.h"

const char *ngtcp2_strerror(int liberr) {
  switch (liberr) {
  case NGTCP2_ERR_INVALID_ARGUMENT:
    return "ERR_INVALID_ARGUMENT";
  case NGTCP2_ERR_UNKNOWN_PKT_TYPE:
    return "ERR_UNKNOWN_PKT_TYPE";
  case NGTCP2_ERR_NOBUF:
    return "ERR_NOBUF";
  case NGTCP2_ERR_BAD_PKT_HASH:
    return "ERR_BAD_PKT_HASH";
  case NGTCP2_ERR_PROTO:
    return "ERR_PROTO";
  case NGTCP2_ERR_INVALID_STATE:
    return "ERR_INVALID_STATE";
  case NGTCP2_ERR_NOMEM:
    return "ERR_NOMEM";
  case NGTCP2_ERR_CALLBACK_FAILURE:
    return "ERR_CALLBACK_FAILURE";
  case NGTCP2_ERR_INTERNAL:
    return "ERR_INTERNAL";
  default:
    return "UNKNOWN";
  }
}
