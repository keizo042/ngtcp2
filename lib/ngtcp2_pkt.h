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
#ifndef NGTCP2_PKT_H
#define NGTCP2_PKT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <ngtcp2/ngtcp2.h>

#define NGTCP2_HEADER_FORM_BIT 0x80
#define NGTCP2_CONN_ID_BIT 0x40
#define NGTCP2_KEY_PHASE_BIT 0x20
#define NGTCP2_LONG_TYPE_MASK 0x7f
#define NGTCP2_SHORT_TYPE_MASK 0x1f

/* NGTCP2_LONG_HEADERLEN is the length of long header */
#define NGTCP2_LONG_HEADERLEN 17

#define NGTCP2_STREAM_FIN_BIT 0x20
#define NGTCP2_STREAM_SS_MASK 0x18
#define NGTCP2_STREAM_OO_MASK 0x06
#define NGTCP2_STREAM_D_BIT 0x01

/* NGTCP2_STREAM_OVERHEAD is the maximum number of bytes required
   other than payload for STREAM frame.  That is from type field to
   the beginning of the payload. */
#define NGTCP2_STREAM_OVERHEAD 15

#define NGTCP2_ACK_N_BIT 0x10
#define NGTCP2_ACK_LL_MASK 0x0c
#define NGTCP2_ACK_MM_MASK 0x03
#define NGTCP2_ACK_LL_02_MASK 0x08
#define NGTCP2_ACK_MM_02_MASK 0x02

/* The length of FNV-1a message digest for Unprotected packet */
#define NGTCP2_PKT_MDLEN 8

/*
 * ngtcp2_pkt_hd_init initializes |hd| with the given values.
 */
void ngtcp2_pkt_hd_init(ngtcp2_pkt_hd *hd, uint8_t flags, uint8_t type,
                        uint64_t conn_id, uint64_t pkt_num, uint32_t version);

/*
 * ngtcp2_pkt_decode_hd_long decodes QUIC long packet header in |pkt|
 * of length |pktlen|.  It stores the result in the object pointed by
 * |dest|, and returns the number of bytes decoded to read the packet
 * header if it succeeds, or one of the following error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Packet is too short; or it is not a long header
 * NGTCP2_ERR_UNKNOWN_PKT_TYPE
 *     Packet type is unknown
 */
ssize_t ngtcp2_pkt_decode_hd_long(ngtcp2_pkt_hd *dest, const uint8_t *pkt,
                                  size_t pktlen);

/*
 * ngtcp2_pkt_decode_hd_short decodes QUIC short packet header in
 * |pkt| of length |pktlen|.  It stores the result in the object
 * pointed by |dest|, and returns the number of bytes decoded to read
 * the packet header if it succeeds, or one of the following error
 * codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Packet is too short; or it is not a short header
 * NGTCP2_ERR_UNKNOWN_PKT_TYPE
 *     Packet type is unknown
 */
ssize_t ngtcp2_pkt_decode_hd_short(ngtcp2_pkt_hd *dest, const uint8_t *pkt,
                                   size_t pktlen);

/*
 * ngtcp2_pkt_encode_hd_long encodes |hd| as QUIC long header into
 * |out| which has length |outlen|.  It returns the number of bytes
 * written into |outlen| if it succeeds, or one of the following
 * negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer is too short
 */
ssize_t ngtcp2_pkt_encode_hd_long(uint8_t *out, size_t outlen,
                                  const ngtcp2_pkt_hd *hd);

/*
 * ngtcp2_pkt_encode_hd_short encodes |hd| as QUIC short header into
 * |out| which has length |outlen|.  It returns the number of bytes
 * written into |outlen| if it succeeds, or one of the following
 * negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer is too short
 */
ssize_t ngtcp2_pkt_encode_hd_short(uint8_t *out, size_t outlen,
                                   const ngtcp2_pkt_hd *hd);

/*
 * ngtcp2_pkt_decode_version_negotiation decodes Version Negotiation
 * packet payload |payload| of length |payloadlen|, and stores the
 * result in |dest|.  |dest| must have enough capacity to store the
 * result.  |payloadlen| also must be a multiple of sizeof(uint32_t).
 *
 * This function returns the number of versions written in |dest|.
 */
size_t ngtcp2_pkt_decode_version_negotiation(uint32_t *dest,
                                             const uint8_t *payload,
                                             size_t payloadlen);

/*
 * ngtcp2_pkt_decode_stream_frame decodes STREAM frame from |payload|
 * of length |payloadlen|.  The result is stored in the object pointed
 * by |dest|.  STREAM frame must start at `payload[0]`.  This function
 * returns when it decodes one STREAM frame, and returns the exact
 * number of bytes for one STREAM frame if it succeeds, or one of the
 * following negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include STREAM frame; or
 *     Payload is too short to include STREAM frame
 */
ssize_t ngtcp2_pkt_decode_stream_frame(ngtcp2_stream *dest,
                                       const uint8_t *payload,
                                       size_t payloadlen);

/*
 * ngtcp2_pkt_decode_ack_frame decodes ACK frame from |payload| of
 * length |payloadlen|.  The result is stored in the object pointed by
 * |dest|.  ACK frame must start at `payload[0]`.  This function
 * returns when it decodes one ACK frame, and returns the exact number
 * of bytes for one ACK frame if it succeeds, or one of the following
 * negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include ACK frame; or
 *     Payload is too short to include ACK frame
 */
ssize_t ngtcp2_pkt_decode_ack_frame(ngtcp2_ack *dest, const uint8_t *payload,
                                    size_t payloadlen, uint64_t max_rx_pkt_num);

/*
 * ngtcp2_pkt_decode_padding_frame decodes contiguous PADDING frames
 * from |payload| of length |payloadlen|.  It continues to parse
 * frames as long as the frame type is PADDING.  This function returns
 * when it encounters the frame type which is not PADDING.  The first
 * byte (``payload[0]``) must be NGTCP2_FRAME_PADDING.  This function
 * returns the exact number of bytes read for PADDING frames if it
 * succeeds, or one of the following negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include PADDING frame.
 */
ssize_t ngtcp2_pkt_decode_padding_frame(ngtcp2_padding *dest,
                                        const uint8_t *payload,
                                        size_t payloadlen);

/*
 * ngtcp2_pkt_decode_rst_stream_frame decodes RST_STREAM frame from
 * |payload| of length |payloadlen|.  The result is stored in the
 * object pointed by |dest|.  RST_STREAM frame must start at
 * `payload[0]`.  This function returns when it decodes one RST_STREAM
 * frame., and returns the exact number of bytes for one RST_STREAM
 * frame if it succeeds, or one of the following negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include RST_STREAM frame;
 *     or Payload is too short to include RST_STREAM frame.
 */
ssize_t ngtcp2_pkt_decode_rst_stream_frame(ngtcp2_rst_stream *dest,
                                           const uint8_t *payload,
                                           size_t payloadlen);

/*
 * ngtcp2_pkt_decode_connection_close_frame decodes CONNECTION_CLOSE
 * frame from |payload| of length |payloadlen|.  The result is stored
 * in the object pointed by |dest|.  CONNECTION_CLOSE frame must start
 * at `payload[0]`.  This function returns when it decodes one
 * CONNECTION_CLOSE frame., and returns the exact number of bytes for
 * one CONNECTION_CLOSE frame if it succeeds, or one of the following
 * negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include CONNECTION_CLOSE
 *     frame; or Payload is too short to include CONNECTION_CLOSE
 *     frame.
 */
ssize_t ngtcp2_pkt_decode_connection_close_frame(ngtcp2_connection_close *dest,
                                                 const uint8_t *payload,
                                                 size_t payloadlen);

/*
 * ngtcp2_pkt_decode_goaway_frame decodes GOAWAY frame from |payload|
 * of length |payloadlen|.  The result is stored in the object pointed
 * by |dest|.  GOAWAY frame must start at `payload[0]`.  This function
 * returns when it decodes one GOAWAY frame., and returns the exact
 * number of bytes for one GOAWAY frame if it succeeds, or one of the
 * following negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include GOAWAY frame; or
 *     Payload is too short to include GOAWAY frame.
 */
ssize_t ngtcp2_pkt_decode_goaway_frame(ngtcp2_goaway *dest,
                                       const uint8_t *payload,
                                       size_t payloadlen);

/*
 * ngtcp2_pkt_decode_max_data_frame decodes MAX_DATA frame from
 * |payload| of length |payloadlen|.  The result is stored in the
 * object pointed by |dest|.  MAX_DATA frame must start at
 * `payload[0]`.  This function returns when it decodes one MAX_DATA
 * frame., and returns the exact number of bytes for one MAX_DATA
 * frame if it succeeds, or one of the following negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include MAX_DATA frame; or
 *     Payload is too short to include MAX_DATA frame.
 */
ssize_t ngtcp2_pkt_decode_max_data_frame(ngtcp2_max_data *dest,
                                         const uint8_t *payload,
                                         size_t payloadlen);

/*
 * ngtcp2_pkt_decode_max_stream_data_frame decodes MAX_STREAM_DATA
 * frame from |payload| of length |payloadlen|.  The result is stored
 * in the object pointed by |dest|.  MAX_STREAM_DATA frame must start
 * at `payload[0]`.  This function returns when it decodes one
 * MAX_STREAM_DATA frame., and returns the exact number of bytes for
 * one MAX_STREAM_DATA frame if it succeeds, or one of the following
 * negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include MAX_STREAM_DATA
 *     frame; or Payload is too short to include MAX_STREAM_DATA
 *     frame.
 */
ssize_t ngtcp2_pkt_decode_max_stream_data_frame(ngtcp2_max_stream_data *dest,
                                                const uint8_t *payload,
                                                size_t payloadlen);

/*
 * ngtcp2_pkt_decode_max_stream_id_frame decodes MAX_STREAM_ID frame
 * from |payload| of length |payloadlen|.  The result is stored in the
 * object pointed by |dest|.  MAX_STREAM_ID frame must start at
 * `payload[0]`.  This function returns when it decodes one
 * MAX_STREAM_ID frame., and returns the exact number of bytes for one
 * MAX_STREAM_ID frame if it succeeds, or one of the following
 * negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include MAX_STREAM_ID
 *     frame; or Payload is too short to include MAX_STREAM_ID frame.
 */
ssize_t ngtcp2_pkt_decode_max_stream_id_frame(ngtcp2_max_stream_id *dest,
                                              const uint8_t *payload,
                                              size_t payloadlen);

/*
 * ngtcp2_pkt_decode_ping_frame decodes PING frame from |payload| of
 * length |payloadlen|.  The result is stored in the object pointed by
 * |dest|.  PING frame must start at `payload[0]`.  This function
 * returns when it decodes one PING frame., and returns the exact
 * number of bytes for one PING frame if it succeeds, or one of the
 * following negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include PING frame; or
 *     Payload is too short to include PING frame.
 */
ssize_t ngtcp2_pkt_decode_ping_frame(ngtcp2_ping *dest, const uint8_t *payload,
                                     size_t payloadlen);

/*
 * ngtcp2_pkt_decode_blocked_frame decodes BLOCKED frame from
 * |payload| of length |payloadlen|.  The result is stored in the
 * object pointed by |dest|.  BLOCKED frame must start at
 * `payload[0]`.  This function returns when it decodes one BLOCKED
 * frame., and returns the exact number of bytes for one BLOCKED frame
 * if it succeeds, or one of the following negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include BLOCKED frame; or
 *     Payload is too short to include BLOCKED frame.
 */
ssize_t ngtcp2_pkt_decode_blocked_frame(ngtcp2_blocked *dest,
                                        const uint8_t *payload,
                                        size_t payloadlen);

/*
 * ngtcp2_pkt_decode_stream_blocked_frame decodes STREAM_BLOCKED frame
 * from |payload| of length |payloadlen|.  The result is stored in the
 * object pointed by |dest|.  STREAM_BLOCKED frame must start at
 * `payload[0]`.  This function returns when it decodes one
 * STREAM_BLOCKED frame., and returns the exact number of bytes for
 * one STREAM_BLOCKED frame if it succeeds, or one of the following
 * negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include STREAM_BLOCKED
 *     frame; or Payload is too short to include STREAM_BLOCKED frame.
 */
ssize_t ngtcp2_pkt_decode_stream_blocked_frame(ngtcp2_stream_blocked *dest,
                                               const uint8_t *payload,
                                               size_t payloadlen);

/*
 * ngtcp2_pkt_decode_stream_id_needed_frame decodes STREAM_ID_NEEDED
 * frame from |payload| of length |payloadlen|.  The result is stored
 * in the object pointed by |dest|.  STREAM_ID_NEEDED frame must start
 * at `payload[0]`.  This function returns when it decodes one
 * STREAM_ID_NEEDED frame., and returns the exact number of bytes for
 * one STREAM_ID_NEEDED frame if it succeeds, or one of the following
 * negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include STREAM_ID_NEEDED
 *     frame; or Payload is too short to include STREAM_ID_NEEDED
 *     frame.
 */
ssize_t ngtcp2_pkt_decode_stream_id_needed_frame(ngtcp2_stream_id_needed *dest,
                                                 const uint8_t *payload,
                                                 size_t payloadlen);

/*
 * ngtcp2_pkt_decode_new_connection_id_frame decodes NEW_CONNECTION_ID
 * frame from |payload| of length |payloadlen|.  The result is stored
 * in the object pointed by |dest|.  NEW_CONNECTION_ID frame must
 * start at `payload[0]`.  This function returns when it decodes one
 * NEW_CONNECTION_ID frame., and returns the exact number of bytes for
 * one NEW_CONNECTION_ID frame if it succeeds, or one of the following
 * negative error codes:
 *
 * NGTCP2_ERR_INVALID_ARGUMENT
 *     Type indicates that payload does not include NEW_CONNECTION_ID
 *     frame; or Payload is too short to include NEW_CONNECTION_ID
 *     frame.
 */
ssize_t ngtcp2_pkt_decode_new_connection_id_frame(
    ngtcp2_new_connection_id *dest, const uint8_t *payload, size_t payloadlen);

/**
 * ngtcp2_pkt_encode_stream_frame encodes STREAM frame |fr| into the
 * buffer pointed by |out| of length |outlen|.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t ngtcp2_pkt_encode_stream_frame(uint8_t *out, size_t outlen,
                                       const ngtcp2_stream *fr);

/**
 * ngtcp2_pkt_encode_ack_frame encodes ACK frame |fr| into the buffer
 * pointed by |out| of length |outlen|.
 *
 * Currently, this function only encodes Largest Acknowledged and ACK
 * delay.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t ngtcp2_pkt_encode_ack_frame(uint8_t *out, size_t outlen,
                                    const ngtcp2_ack *fr);

/**
 * ngtcp2_pkt_encode_padding_frame encodes PADDING frame |fr| into the
 * buffer pointed by |out| of length |outlen|.
 *
 * This function encodes consecutive fr->len PADDING frames.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write frame(s).
 */
ssize_t ngtcp2_pkt_encode_padding_frame(uint8_t *out, size_t outlen,
                                        const ngtcp2_padding *fr);

/**
 * ngtcp2_pkt_encode_rst_stream_frame encodes RST_STREAM frame |fr|
 * into the buffer pointed by |out| of length |buflen|.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t ngtcp2_pkt_encode_rst_stream_frame(uint8_t *out, size_t outlen,
                                           const ngtcp2_rst_stream *fr);

/**
 * ngtcp2_pkt_encode_connection_close_frame encodes CONNECTION_CLOSE
 * frame |fr| into the buffer pointed by |out| of length |outlen|.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t
ngtcp2_pkt_encode_connection_close_frame(uint8_t *out, size_t outlen,
                                         const ngtcp2_connection_close *fr);

/**
 * ngtcp2_pkt_encode_goaway_frame encodes GOAWAY frame |fr| into the
 * buffer pointed by |out| of length |outlen|.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t ngtcp2_pkt_encode_goaway_frame(uint8_t *out, size_t outlen,
                                       const ngtcp2_goaway *fr);

/**
 * ngtcp2_pkt_encode_max_data_frame encodes MAX_DATA frame |fr| into
 * the buffer pointed by |out| of length |outlen|.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t ngtcp2_pkt_encode_max_data_frame(uint8_t *out, size_t outlen,
                                         const ngtcp2_max_data *fr);

/**
 * ngtcp2_pkt_encode_max_stream_data_frame encodes MAX_STREAM_DATA
 * frame |fr| into the buffer pointed by |out| of length |outlen|.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t
ngtcp2_pkt_encode_max_stream_data_frame(uint8_t *out, size_t outlen,
                                        const ngtcp2_max_stream_data *fr);

/**
 * ngtcp2_pkt_encode_max_stream_id_frame encodes MAX_STREAM_ID frame
 * |fr| into the buffer pointed by |out| of length |outlen|.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t ngtcp2_pkt_encode_max_stream_id_frame(uint8_t *out, size_t outlen,
                                              const ngtcp2_max_stream_id *fr);

/**
 * ngtcp2_pkt_encode_ping_frame encodes PING frame |fr| into the
 * buffer pointed by |out| of length |outlen|.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t ngtcp2_pkt_encode_ping_frame(uint8_t *out, size_t outlen,
                                     const ngtcp2_ping *fr);

/**
 * ngtcp2_pkt_encode_blocked_frame encodes BLOCKED frame |fr| into the
 * buffer pointed by |out| of length |outlen|.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t ngtcp2_pkt_encode_blocked_frame(uint8_t *out, size_t outlen,
                                        const ngtcp2_blocked *fr);

/**
 * ngtcp2_pkt_encode_stream_blocked_frame encodes STREAM_BLOCKED frame
 * |fr| into the buffer pointed by |out| of length |outlen|.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t ngtcp2_pkt_encode_stream_blocked_frame(uint8_t *out, size_t outlen,
                                               const ngtcp2_stream_blocked *fr);

/**
 * ngtcp2_pkt_encode_stream_id_needed_frame encodes STREAM_ID_NEEDED
 * frame |fr| into the buffer pointed by |out| of length |outlen|.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t
ngtcp2_pkt_encode_stream_id_needed_frame(uint8_t *out, size_t outlen,
                                         const ngtcp2_stream_id_needed *fr);

/**
 * ngtcp2_pkt_encode_new_connection_id_frame encodes NEW_CONNECTION_ID
 * frame |fr| into the buffer pointed by |out| of length |outlen|.
 *
 * This function returns the number of bytes written if it succeeds,
 * or one of the following negative error codes:
 *
 * NGTCP2_ERR_NOBUF
 *     Buffer does not have enough capacity to write a frame.
 */
ssize_t
ngtcp2_pkt_encode_new_connection_id_frame(uint8_t *out, size_t outlen,
                                          const ngtcp2_new_connection_id *fr);

/**
 * ngtcp2_pkt_adjust_pkt_num find the full 64 bits packet number for
 * |pkt_num|, which is expected to be least significant |n| bits.  The
 * |max_pkt_num| is the highest successfully authenticated packet
 * number.
 */
uint64_t ngtcp2_pkt_adjust_pkt_num(uint64_t max_pkt_num, uint64_t pkt_num,
                                   size_t n);

/**
 * ngtcp2_pkt_adjust_ack_pkt_num adjusts all packet numbers in |ack|
 * using the maximum packet number |max_pkt_num| received so far.
 */
void ngtcp2_pkt_adjust_ack_pkt_num(ngtcp2_ack *ack, uint64_t max_pkt_num);

#endif /* NGTCP2_PKT_H */
