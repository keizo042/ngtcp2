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
#include "ngtcp2_pkt_test.h"

#include <assert.h>

#include <CUnit/CUnit.h>

#include "ngtcp2_pkt.h"
#include "ngtcp2_test_helper.h"

void test_ngtcp2_pkt_decode_hd_long(void) {
  ngtcp2_pkt_hd hd, nhd;
  uint8_t buf[256];
  ssize_t rv;

  ngtcp2_pkt_hd_init(&hd, NGTCP2_PKT_FLAG_LONG_FORM,
                     NGTCP2_PKT_VERSION_NEGOTIATION, 0xf1f2f3f4f5f6f7f8llu,
                     0xe1e2e3e4u, 0xd1d2d3d4u);

  rv = ngtcp2_pkt_encode_hd_long(buf, sizeof(buf), &hd);

  CU_ASSERT(NGTCP2_LONG_HEADERLEN == rv);

  rv = ngtcp2_pkt_decode_hd_long(&nhd, buf, NGTCP2_LONG_HEADERLEN);

  CU_ASSERT(NGTCP2_LONG_HEADERLEN == rv);
  CU_ASSERT(hd.flags == nhd.flags);
  CU_ASSERT(hd.conn_id == nhd.conn_id);
  CU_ASSERT(hd.pkt_num == nhd.pkt_num);
  CU_ASSERT(hd.version == nhd.version);
}

void test_ngtcp2_pkt_decode_hd_short(void) {
  ngtcp2_pkt_hd hd, nhd;
  uint8_t buf[256];
  ssize_t rv;
  size_t expectedlen;

  /* NGTCP2_PKT_03 */
  ngtcp2_pkt_hd_init(&hd, NGTCP2_PKT_FLAG_NONE, NGTCP2_PKT_03,
                     0xf1f2f3f4f5f6f7f8llu, 0xe1e2e3e4u, 0xd1d2d3d4u);

  expectedlen = 5;

  rv = ngtcp2_pkt_encode_hd_short(buf, sizeof(buf), &hd);

  CU_ASSERT((ssize_t)expectedlen == rv);

  rv = ngtcp2_pkt_decode_hd_short(&nhd, buf, expectedlen);

  CU_ASSERT((ssize_t)expectedlen == rv);
  CU_ASSERT(hd.flags == nhd.flags);
  CU_ASSERT(NGTCP2_PKT_03 == nhd.type);
  CU_ASSERT(0 == nhd.conn_id);
  CU_ASSERT(hd.pkt_num == nhd.pkt_num);
  CU_ASSERT(0 == nhd.version);

  /* NGTCP2_PKT_02 */
  ngtcp2_pkt_hd_init(&hd, NGTCP2_PKT_FLAG_NONE, NGTCP2_PKT_02,
                     0xf1f2f3f4f5f6f7f8llu, 0xe1e2e3e4u, 0xd1d2d3d4u);

  expectedlen = 3;

  rv = ngtcp2_pkt_encode_hd_short(buf, sizeof(buf), &hd);

  CU_ASSERT((ssize_t)expectedlen == rv);

  rv = ngtcp2_pkt_decode_hd_short(&nhd, buf, expectedlen);

  CU_ASSERT((ssize_t)expectedlen == rv);
  CU_ASSERT(hd.flags == nhd.flags);
  CU_ASSERT(NGTCP2_PKT_02 == nhd.type);
  CU_ASSERT(0 == nhd.conn_id);
  CU_ASSERT((hd.pkt_num & 0xffff) == nhd.pkt_num);
  CU_ASSERT(0 == nhd.version);

  /* NGTCP2_PKT_01 */
  ngtcp2_pkt_hd_init(&hd, NGTCP2_PKT_FLAG_NONE, NGTCP2_PKT_01,
                     0xf1f2f3f4f5f6f7f8llu, 0xe1e2e3e4u, 0xd1d2d3d4u);

  expectedlen = 2;

  rv = ngtcp2_pkt_encode_hd_short(buf, sizeof(buf), &hd);

  CU_ASSERT((ssize_t)expectedlen == rv);

  rv = ngtcp2_pkt_decode_hd_short(&nhd, buf, expectedlen);

  CU_ASSERT((ssize_t)expectedlen == rv);
  CU_ASSERT(hd.flags == nhd.flags);
  CU_ASSERT(NGTCP2_PKT_01 == nhd.type);
  CU_ASSERT(0 == nhd.conn_id);
  CU_ASSERT((hd.pkt_num & 0xff) == nhd.pkt_num);
  CU_ASSERT(0 == nhd.version);

  /* With connection ID, and Key Phase */
  ngtcp2_pkt_hd_init(&hd, NGTCP2_PKT_FLAG_CONN_ID | NGTCP2_PKT_FLAG_KEY_PHASE,
                     NGTCP2_PKT_03, 0xf1f2f3f4f5f6f7f8llu, 0xe1e2e3e4u,
                     0xd1d2d3d4u);

  expectedlen = 13;

  rv = ngtcp2_pkt_encode_hd_short(buf, sizeof(buf), &hd);

  CU_ASSERT((ssize_t)expectedlen == rv);

  rv = ngtcp2_pkt_decode_hd_short(&nhd, buf, expectedlen);

  CU_ASSERT((ssize_t)expectedlen == rv);
  CU_ASSERT(hd.flags == nhd.flags);
  CU_ASSERT(NGTCP2_PKT_03 == nhd.type);
  CU_ASSERT(hd.conn_id == nhd.conn_id);
  CU_ASSERT(hd.pkt_num == nhd.pkt_num);
  CU_ASSERT(0 == nhd.version);
}

void test_ngtcp2_pkt_decode_stream_frame(void) {
  uint8_t buf[256];
  size_t buflen;
  ngtcp2_frame fr;
  ssize_t rv;
  size_t expectedlen;

  /* 32 bits Stream ID + 64 bits Offset + Data Length */
  buflen = ngtcp2_t_encode_stream_frame(buf, NGTCP2_STREAM_D_BIT, 0xf1f2f3f4u,
                                        0xf1f2f3f4f5f6f7f8llu, 0x14);

  expectedlen = 1 + 4 + 8 + 2 + 20;

  CU_ASSERT(expectedlen == buflen);

  rv = ngtcp2_pkt_decode_stream_frame(&fr.stream, buf, buflen);

  CU_ASSERT((ssize_t)expectedlen == rv);
  CU_ASSERT(0 == fr.stream.fin);
  CU_ASSERT(0xf1f2f3f4u == fr.stream.stream_id);
  CU_ASSERT(0xf1f2f3f4f5f6f7f8llu == fr.stream.offset);
  CU_ASSERT(0x14 == fr.stream.datalen);

  /* Cutting 1 bytes from the tail must cause invalid argument
     error */
  rv = ngtcp2_pkt_decode_stream_frame(&fr.stream, buf, buflen - 1);

  CU_ASSERT(NGTCP2_ERR_INVALID_ARGUMENT == rv);

  memset(&fr, 0, sizeof(fr));

  /* 24 bits Stream ID + 32 bits Offset + Data Length */
  buflen = ngtcp2_t_encode_stream_frame(buf, NGTCP2_STREAM_D_BIT, 0xf1f2f3,
                                        0xf1f2f3f4u, 0x14);

  expectedlen = 1 + 3 + 4 + 2 + 20;

  CU_ASSERT(expectedlen == buflen);

  rv = ngtcp2_pkt_decode_stream_frame(&fr.stream, buf, buflen);

  CU_ASSERT((ssize_t)expectedlen == rv);
  CU_ASSERT(0 == fr.stream.fin);
  CU_ASSERT(0xf1f2f3 == fr.stream.stream_id);
  CU_ASSERT(0xf1f2f3f4u == fr.stream.offset);
  CU_ASSERT(0x14 == fr.stream.datalen);

  /* Cutting 1 bytes from the tail must cause invalid argument
     error */
  rv = ngtcp2_pkt_decode_stream_frame(&fr.stream, buf, buflen - 1);

  CU_ASSERT(NGTCP2_ERR_INVALID_ARGUMENT == rv);

  memset(&fr, 0, sizeof(fr));

  /* 16 bits Stream ID + 16 bits Offset + Data Length */
  buflen = ngtcp2_t_encode_stream_frame(buf, NGTCP2_STREAM_D_BIT, 0xf1f2,
                                        0xf1f2, 0x14);

  expectedlen = 1 + 2 + 2 + 2 + 20;

  CU_ASSERT(expectedlen == buflen);

  rv = ngtcp2_pkt_decode_stream_frame(&fr.stream, buf, buflen);

  CU_ASSERT((ssize_t)expectedlen == rv);
  CU_ASSERT(0 == fr.stream.fin);
  CU_ASSERT(0xf1f2 == fr.stream.stream_id);
  CU_ASSERT(0xf1f2 == fr.stream.offset);
  CU_ASSERT(0x14 == fr.stream.datalen);

  /* Cutting 1 bytes from the tail must cause invalid argument
     error */
  rv = ngtcp2_pkt_decode_stream_frame(&fr.stream, buf, buflen - 1);

  CU_ASSERT(NGTCP2_ERR_INVALID_ARGUMENT == rv);

  memset(&fr, 0, sizeof(fr));

  /* 8 bits Stream ID + no Offset + Data Length */
  buflen =
      ngtcp2_t_encode_stream_frame(buf, NGTCP2_STREAM_D_BIT, 0xf1, 0x00, 0x14);

  expectedlen = 1 + 1 + 0 + 2 + 20;

  CU_ASSERT(expectedlen == buflen);

  rv = ngtcp2_pkt_decode_stream_frame(&fr.stream, buf, buflen);

  CU_ASSERT((ssize_t)expectedlen == rv);
  CU_ASSERT(0 == fr.stream.fin);
  CU_ASSERT(0xf1 == fr.stream.stream_id);
  CU_ASSERT(0x00 == fr.stream.offset);
  CU_ASSERT(0x14 == fr.stream.datalen);

  /* Cutting 1 bytes from the tail must cause invalid argument
     error */
  rv = ngtcp2_pkt_decode_stream_frame(&fr.stream, buf, buflen - 1);

  CU_ASSERT(NGTCP2_ERR_INVALID_ARGUMENT == rv);

  memset(&fr, 0, sizeof(fr));

  /* Fin bit set + no Data Length */
  buflen = ngtcp2_t_encode_stream_frame(buf, NGTCP2_STREAM_FIN_BIT, 0xf1f2f3f4u,
                                        0x00, 0x14);

  expectedlen = 1 + 4 + 20;

  CU_ASSERT(expectedlen == buflen);

  rv = ngtcp2_pkt_decode_stream_frame(&fr.stream, buf, buflen);

  CU_ASSERT((ssize_t)expectedlen == rv);
  CU_ASSERT(1 == fr.stream.fin);
  CU_ASSERT(0xf1f2f3f4u == fr.stream.stream_id);
  CU_ASSERT(0x00 == fr.stream.offset);
  CU_ASSERT(0x14 == fr.stream.datalen);

  memset(&fr, 0, sizeof(fr));
}

void test_ngtcp2_pkt_decode_ack_frame(void) {
  uint8_t buf[256];
  size_t buflen;
  ngtcp2_frame fr;
  ssize_t rv;
  size_t expectedlen;

  /* 64 bits Largest Acknowledged + No Num Blocks + 0 NumTS*/
  buflen = ngtcp2_t_encode_ack_frame(buf, 0xf1f2f3f4f5f6f7f8llu,
                                     0xe1e2e3e4e5e6e7e8llu, 99,
                                     0xd1d2d3d4d5d6d7d8llu);

  expectedlen = 1 + 1 + 1 + 8 + 2 + 8 + 1 + 8;

  CU_ASSERT(expectedlen == buflen);

  rv = ngtcp2_pkt_decode_ack_frame(&fr.ack, buf, buflen, 0);

  CU_ASSERT((ssize_t)expectedlen == rv);
  CU_ASSERT(0x1f == fr.ack.flags);
  CU_ASSERT(0xf1f2f3f4f5f6f7f8llu == fr.ack.largest_ack);
  CU_ASSERT(1 == fr.ack.num_blks);
  CU_ASSERT(0xe1e2e3e4e5e6e7e8llu == fr.ack.first_ack_blklen);
  CU_ASSERT(99 == fr.ack.blks[0].gap);
  CU_ASSERT(0xd1d2d3d4d5d6d7d8llu == fr.ack.blks[0].blklen);
  CU_ASSERT(0 == fr.ack.num_ts);
}

void test_ngtcp2_pkt_decode_padding_frame(void) {
  uint8_t buf[256];
  ngtcp2_frame fr;
  ssize_t rv;
  size_t paddinglen = 31;

  memset(buf, 0, paddinglen);
  buf[paddinglen] = NGTCP2_FRAME_STREAM;

  rv = ngtcp2_pkt_decode_padding_frame(&fr.padding, buf, paddinglen + 1);

  CU_ASSERT((ssize_t)paddinglen == rv);
  CU_ASSERT((size_t)31 == fr.padding.len);
}

void test_ngtcp2_pkt_encode_stream_frame(void) {
  const uint8_t data[] = "0123456789abcdef0";
  uint8_t buf[256];
  ngtcp2_frame fr, nfr;
  ssize_t rv;
  size_t framelen;

  /* 32 bits Stream ID + 64 bits Offset + Data Length */
  fr.type = NGTCP2_FRAME_STREAM;
  fr.stream.fin = 0;
  fr.stream.stream_id = 0xf1f2f3f4u;
  fr.stream.offset = 0xf1f2f3f4f5f6f7f8llu;
  fr.stream.datalen = strsize(data);
  fr.stream.data = data;

  framelen = 1 + 4 + 8 + 2 + 17;

  rv = ngtcp2_pkt_encode_stream_frame(buf, sizeof(buf), &fr.stream);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_stream_frame(&nfr.stream, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(0x1f == nfr.stream.flags);
  CU_ASSERT(fr.stream.fin == nfr.stream.fin);
  CU_ASSERT(fr.stream.stream_id == nfr.stream.stream_id);
  CU_ASSERT(fr.stream.offset == nfr.stream.offset);
  CU_ASSERT(fr.stream.datalen == nfr.stream.datalen);
  CU_ASSERT(0 == memcmp(fr.stream.data, nfr.stream.data, fr.stream.datalen));

  memset(&nfr, 0, sizeof(nfr));

  /* 24 bits Stream ID + 32 bits Offset + Data Length */
  fr.type = NGTCP2_FRAME_STREAM;
  fr.stream.fin = 0;
  fr.stream.stream_id = 0xf1f2f3;
  fr.stream.offset = 0xf1f2f3f4u;
  fr.stream.datalen = strsize(data);
  fr.stream.data = data;

  framelen = 1 + 3 + 4 + 2 + 17;

  rv = ngtcp2_pkt_encode_stream_frame(buf, sizeof(buf), &fr.stream);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_stream_frame(&nfr.stream, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(0x15 == nfr.stream.flags);
  CU_ASSERT(fr.stream.fin == nfr.stream.fin);
  CU_ASSERT(fr.stream.stream_id == nfr.stream.stream_id);
  CU_ASSERT(fr.stream.offset == nfr.stream.offset);
  CU_ASSERT(fr.stream.datalen == nfr.stream.datalen);
  CU_ASSERT(0 == memcmp(fr.stream.data, nfr.stream.data, fr.stream.datalen));

  memset(&nfr, 0, sizeof(nfr));

  /* 16 bits Stream ID + 16 bits Offset + Data Length */
  fr.type = NGTCP2_FRAME_STREAM;
  fr.stream.fin = 0;
  fr.stream.stream_id = 0xf1f2;
  fr.stream.offset = 0xf1f2;
  fr.stream.datalen = strsize(data);
  fr.stream.data = data;

  framelen = 1 + 2 + 2 + 2 + 17;

  rv = ngtcp2_pkt_encode_stream_frame(buf, sizeof(buf), &fr.stream);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_stream_frame(&nfr.stream, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(0x0b == nfr.stream.flags);
  CU_ASSERT(fr.stream.fin == nfr.stream.fin);
  CU_ASSERT(fr.stream.stream_id == nfr.stream.stream_id);
  CU_ASSERT(fr.stream.offset == nfr.stream.offset);
  CU_ASSERT(fr.stream.datalen == nfr.stream.datalen);
  CU_ASSERT(0 == memcmp(fr.stream.data, nfr.stream.data, fr.stream.datalen));

  memset(&nfr, 0, sizeof(nfr));

  /* 8 bits Stream ID + No Offset + Data Length */
  fr.type = NGTCP2_FRAME_STREAM;
  fr.stream.fin = 0;
  fr.stream.stream_id = 0xf1;
  fr.stream.offset = 0;
  fr.stream.datalen = strsize(data);
  fr.stream.data = data;

  framelen = 1 + 1 + 2 + 17;

  rv = ngtcp2_pkt_encode_stream_frame(buf, sizeof(buf), &fr.stream);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_stream_frame(&nfr.stream, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(0x01 == nfr.stream.flags);
  CU_ASSERT(fr.stream.fin == nfr.stream.fin);
  CU_ASSERT(fr.stream.stream_id == nfr.stream.stream_id);
  CU_ASSERT(fr.stream.offset == nfr.stream.offset);
  CU_ASSERT(fr.stream.datalen == nfr.stream.datalen);
  CU_ASSERT(0 == memcmp(fr.stream.data, nfr.stream.data, fr.stream.datalen));

  memset(&nfr, 0, sizeof(nfr));

  /* Fin + 32 bits Stream ID + 64 bits Offset + Data Length */
  fr.type = NGTCP2_FRAME_STREAM;
  fr.stream.fin = 1;
  fr.stream.stream_id = 0xf1f2f3f4u;
  fr.stream.offset = 0xf1f2f3f4f5f6f7f8llu;
  fr.stream.datalen = strsize(data);
  fr.stream.data = data;

  framelen = 1 + 4 + 8 + 2 + 17;

  rv = ngtcp2_pkt_encode_stream_frame(buf, sizeof(buf), &fr.stream);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_stream_frame(&nfr.stream, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(0x3f == nfr.stream.flags);
  CU_ASSERT(fr.stream.fin == nfr.stream.fin);
  CU_ASSERT(fr.stream.stream_id == nfr.stream.stream_id);
  CU_ASSERT(fr.stream.offset == nfr.stream.offset);
  CU_ASSERT(fr.stream.datalen == nfr.stream.datalen);
  CU_ASSERT(0 == memcmp(fr.stream.data, nfr.stream.data, fr.stream.datalen));

  memset(&nfr, 0, sizeof(nfr));

  /* NOBUF: Fin + 32 bits Stream ID + 64 bits Offset + Data Length */
  fr.type = NGTCP2_FRAME_STREAM;
  fr.stream.fin = 1;
  fr.stream.stream_id = 0xf1f2f3f4u;
  fr.stream.offset = 0xf1f2f3f4f5f6f7f8llu;
  fr.stream.datalen = strsize(data);
  fr.stream.data = data;

  framelen = 1 + 4 + 8 + 2 + 17;

  rv = ngtcp2_pkt_encode_stream_frame(buf, framelen - 1, &fr.stream);

  CU_ASSERT(NGTCP2_ERR_NOBUF == rv);
}

void test_ngtcp2_pkt_encode_ack_frame(void) {
  uint8_t buf[256];
  ngtcp2_frame fr, nfr;
  ssize_t rv;
  size_t framelen;
  size_t i;

  /* 0 Num Blocks */
  fr.type = NGTCP2_FRAME_ACK;
  fr.ack.largest_ack = 0xf1f2f3f4llu;
  fr.ack.first_ack_blklen = 0;
  fr.ack.ack_delay = 0;
  fr.ack.num_blks = 0;
  fr.ack.num_ts = 0;

  framelen = 1 + 1 + 4 + 2 + 4;

  rv = ngtcp2_pkt_encode_ack_frame(buf, sizeof(buf), &fr.ack);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_ack_frame(&nfr.ack, buf, framelen, 0);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT((NGTCP2_ACK_LL_02_MASK | NGTCP2_ACK_MM_02_MASK) == nfr.ack.flags);
  CU_ASSERT(fr.ack.largest_ack == nfr.ack.largest_ack);
  CU_ASSERT(fr.ack.ack_delay == nfr.ack.ack_delay);
  CU_ASSERT(fr.ack.num_blks == nfr.ack.num_blks);
  CU_ASSERT(fr.ack.num_ts == nfr.ack.num_ts);

  memset(&nfr, 0, sizeof(nfr));

  /* 2 Num Blocks */
  fr.type = NGTCP2_FRAME_ACK;
  fr.ack.largest_ack = 0xf1f2f3f4llu;
  fr.ack.first_ack_blklen = 0xe1e2e3e4llu;
  fr.ack.ack_delay = 0xf1f2;
  fr.ack.num_blks = 2;
  fr.ack.blks[0].gap = 255;
  fr.ack.blks[0].blklen = 0xd1d2d3d4llu;
  fr.ack.blks[1].gap = 1;
  fr.ack.blks[1].blklen = 0xd1d2d3d4llu;

  framelen = 1 + 1 + 1 + 4 + 2 + 4 + (1 + 4) * 2;

  rv = ngtcp2_pkt_encode_ack_frame(buf, sizeof(buf), &fr.ack);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_ack_frame(&nfr.ack, buf, framelen, 0);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT((NGTCP2_ACK_N_BIT | NGTCP2_ACK_LL_02_MASK |
             NGTCP2_ACK_MM_02_MASK) == nfr.ack.flags);
  CU_ASSERT(fr.ack.largest_ack == nfr.ack.largest_ack);
  CU_ASSERT(fr.ack.ack_delay = nfr.ack.ack_delay);
  CU_ASSERT(fr.ack.num_blks == nfr.ack.num_blks);
  CU_ASSERT(fr.ack.num_ts == nfr.ack.num_ts);

  for (i = 0; i < fr.ack.num_blks; ++i) {
    CU_ASSERT(fr.ack.blks[i].gap == nfr.ack.blks[i].gap);
    CU_ASSERT(fr.ack.blks[i].blklen == nfr.ack.blks[i].blklen);
  }

  memset(&nfr, 0, sizeof(nfr));
}

void test_ngtcp2_pkt_encode_rst_stream_frame(void) {
  uint8_t buf[32];
  ngtcp2_rst_stream fr, nfr;
  ssize_t rv;
  size_t framelen = 1 + 4 + 4 + 8;

  fr.type = NGTCP2_FRAME_RST_STREAM;
  fr.stream_id = 1000000007;
  fr.error_code = 1000000009;
  fr.final_offset = 0xf1f2f3f4f5f6f7f8llu;

  rv = ngtcp2_pkt_encode_rst_stream_frame(buf, sizeof(buf), &fr);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_rst_stream_frame(&nfr, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(fr.stream_id == nfr.stream_id);
  CU_ASSERT(fr.error_code == nfr.error_code);
  CU_ASSERT(fr.final_offset == nfr.final_offset);
}

void test_ngtcp2_pkt_encode_connection_close_frame(void) {
  uint8_t buf[2048];
  ngtcp2_frame fr, nfr;
  ssize_t rv;
  size_t framelen;
  uint8_t reason[1024];

  memset(reason, 0xfa, sizeof(reason));

  /* no Reason Phrase */
  fr.type = NGTCP2_FRAME_CONNECTION_CLOSE;
  fr.connection_close.error_code = 0xf1f2f3f4u;
  fr.connection_close.reasonlen = 0;
  fr.connection_close.reason = NULL;

  framelen = 1 + 4 + 2;

  rv = ngtcp2_pkt_encode_connection_close_frame(buf, sizeof(buf),
                                                &fr.connection_close);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_connection_close_frame(&nfr.connection_close, buf,
                                                framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(fr.connection_close.error_code == nfr.connection_close.error_code);
  CU_ASSERT(fr.connection_close.reasonlen == nfr.connection_close.reasonlen);
  CU_ASSERT(fr.connection_close.reason == nfr.connection_close.reason);

  memset(&nfr, 0, sizeof(nfr));

  /* 1024 bytes Reason Phrase */
  fr.type = NGTCP2_FRAME_CONNECTION_CLOSE;
  fr.connection_close.error_code = 0xf1f2f3f4u;
  fr.connection_close.reasonlen = sizeof(reason);
  fr.connection_close.reason = reason;

  framelen = 1 + 4 + 2 + sizeof(reason);

  rv = ngtcp2_pkt_encode_connection_close_frame(buf, sizeof(buf),
                                                &fr.connection_close);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_connection_close_frame(&nfr.connection_close, buf,
                                                framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(fr.connection_close.error_code == nfr.connection_close.error_code);
  CU_ASSERT(fr.connection_close.reasonlen == nfr.connection_close.reasonlen);
  CU_ASSERT(0 == memcmp(reason, nfr.connection_close.reason, sizeof(reason)));

  memset(&nfr, 0, sizeof(nfr));
}

void test_ngtcp2_pkt_encode_goaway(void) {
  uint8_t buf[16];
  ngtcp2_goaway fr, nfr;
  ssize_t rv;
  size_t framelen = 1 + 4 + 4;

  fr.type = NGTCP2_FRAME_GOAWAY;
  fr.largest_client_stream_id = 0xf1f2f3f4u;
  fr.largest_server_stream_id = 0xf5f6f7f8u;

  rv = ngtcp2_pkt_encode_goaway_frame(buf, sizeof(buf), &fr);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_goaway_frame(&nfr, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(fr.largest_client_stream_id == nfr.largest_client_stream_id);
  CU_ASSERT(fr.largest_server_stream_id == nfr.largest_server_stream_id);
}

void test_ngtcp2_pkt_encode_max_data_frame(void) {
  uint8_t buf[16];
  ngtcp2_max_data fr, nfr;
  ssize_t rv;
  size_t framelen = 1 + 8;

  fr.type = NGTCP2_FRAME_MAX_DATA;
  fr.max_data = 0xf1f2f3f4f5f6f7f8llu;

  rv = ngtcp2_pkt_encode_max_data_frame(buf, sizeof(buf), &fr);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_max_data_frame(&nfr, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(fr.max_data == nfr.max_data);
}

void test_ngtcp2_pkt_encode_max_stream_data_frame(void) {
  uint8_t buf[16];
  ngtcp2_max_stream_data fr, nfr;
  ssize_t rv;
  size_t framelen = 1 + 4 + 8;

  fr.type = NGTCP2_FRAME_MAX_STREAM_DATA;
  fr.stream_id = 0xf1f2f3f4u;
  fr.max_stream_data = 0xf5f6f7f8f9fafbfcllu;

  rv = ngtcp2_pkt_encode_max_stream_data_frame(buf, sizeof(buf), &fr);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_max_stream_data_frame(&nfr, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(fr.stream_id == nfr.stream_id);
  CU_ASSERT(fr.max_stream_data == nfr.max_stream_data);
}

void test_ngtcp2_pkt_encode_max_stream_id_frame(void) {
  uint8_t buf[16];
  ngtcp2_max_stream_id fr, nfr;
  ssize_t rv;
  size_t framelen = 1 + 4;

  fr.type = NGTCP2_FRAME_MAX_STREAM_ID;
  fr.max_stream_id = 0xf1f2f3f4u;

  rv = ngtcp2_pkt_encode_max_stream_id_frame(buf, sizeof(buf), &fr);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_max_stream_id_frame(&nfr, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(fr.max_stream_id == nfr.max_stream_id);
}

void test_ngtcp2_pkt_encode_ping_frame(void) {
  uint8_t buf[8];
  ngtcp2_ping fr, nfr;
  ssize_t rv;
  size_t framelen = 1;

  fr.type = NGTCP2_FRAME_PING;

  rv = ngtcp2_pkt_encode_ping_frame(buf, sizeof(buf), &fr);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_ping_frame(&nfr, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
}

void test_ngtcp2_pkt_encode_blocked_frame(void) {
  uint8_t buf[8];
  ngtcp2_blocked fr, nfr;
  ssize_t rv;
  size_t framelen = 1;

  fr.type = NGTCP2_FRAME_BLOCKED;

  rv = ngtcp2_pkt_encode_blocked_frame(buf, sizeof(buf), &fr);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_blocked_frame(&nfr, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
}

void test_ngtcp2_pkt_encode_stream_blocked_frame(void) {
  uint8_t buf[16];
  ngtcp2_stream_blocked fr, nfr;
  ssize_t rv;
  size_t framelen = 1 + 4;

  fr.type = NGTCP2_FRAME_STREAM_BLOCKED;
  fr.stream_id = 0xf1f2f3f4u;

  rv = ngtcp2_pkt_encode_stream_blocked_frame(buf, sizeof(buf), &fr);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_stream_blocked_frame(&nfr, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(fr.stream_id == nfr.stream_id);
}

void test_ngtcp2_pkt_encode_stream_id_needed_frame(void) {
  uint8_t buf[8];
  ngtcp2_stream_id_needed fr, nfr;
  ssize_t rv;
  size_t framelen = 1;

  fr.type = NGTCP2_FRAME_STREAM_ID_NEEDED;

  rv = ngtcp2_pkt_encode_stream_id_needed_frame(buf, sizeof(buf), &fr);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_stream_id_needed_frame(&nfr, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
}

void test_ngtcp2_pkt_encode_new_connection_id_frame(void) {
  uint8_t buf[16];
  ngtcp2_new_connection_id fr, nfr;
  ssize_t rv;
  size_t framelen = 1 + 2 + 8;

  fr.type = NGTCP2_FRAME_NEW_CONNECTION_ID;
  fr.seq = 0xf1f2;
  fr.conn_id = 0xf3f4f5f6f7f8f9fallu;

  rv = ngtcp2_pkt_encode_new_connection_id_frame(buf, sizeof(buf), &fr);

  CU_ASSERT((ssize_t)framelen == rv);

  rv = ngtcp2_pkt_decode_new_connection_id_frame(&nfr, buf, framelen);

  CU_ASSERT((ssize_t)framelen == rv);
  CU_ASSERT(fr.type == nfr.type);
  CU_ASSERT(fr.seq = nfr.seq);
  CU_ASSERT(fr.conn_id == nfr.conn_id);
}

void test_ngtcp2_pkt_adjust_pkt_num(void) {
  CU_ASSERT(0xaa831f94llu ==
            ngtcp2_pkt_adjust_pkt_num(0xaa82f30ellu, 0x1f94, 16));

  CU_ASSERT(0x01ff == ngtcp2_pkt_adjust_pkt_num(0x0100, 0xff, 1));
  CU_ASSERT(0x02ff == ngtcp2_pkt_adjust_pkt_num(0x01ff, 0xff, 1));
}
