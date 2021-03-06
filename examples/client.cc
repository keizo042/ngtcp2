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
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <algorithm>

#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <openssl/bio.h>
#include <openssl/err.h>

#include "client.h"
#include "template.h"
#include "network.h"
#include "debug.h"
#include "util.h"
#include "crypto.h"

using namespace ngtcp2;

namespace {
auto randgen = util::make_mt19937();
} // namespace

namespace {
int bio_write(BIO *b, const char *buf, int len) {
  BIO_clear_retry_flags(b);

  auto c = static_cast<Client *>(BIO_get_data(b));

  c->write_client_handshake(reinterpret_cast<const uint8_t *>(buf), len);

  return len;
}
} // namespace

namespace {
int bio_read(BIO *b, char *buf, int len) {
  BIO_clear_retry_flags(b);

  auto c = static_cast<Client *>(BIO_get_data(b));

  len = c->read_server_handshake(reinterpret_cast<uint8_t *>(buf), len);
  if (len == 0) {
    BIO_set_retry_read(b);
    return -1;
  }

  return len;
}
} // namespace

namespace {
int bio_puts(BIO *b, const char *str) { return bio_write(b, str, strlen(str)); }
} // namespace

namespace {
int bio_gets(BIO *b, char *buf, int len) { return -1; }
} // namespace

namespace {
long bio_ctrl(BIO *b, int cmd, long num, void *ptr) {
  switch (cmd) {
  case BIO_CTRL_FLUSH:
    return 1;
  }

  return 0;
}
} // namespace

namespace {
int bio_create(BIO *b) {
  BIO_set_init(b, 1);
  return 1;
}
} // namespace

namespace {
int bio_destroy(BIO *b) {
  if (b == nullptr) {
    return 0;
  }

  return 1;
}
} // namespace

namespace {
BIO_METHOD *create_bio_method() {
  static auto meth = BIO_meth_new(BIO_TYPE_FD, "bio");
  BIO_meth_set_write(meth, bio_write);
  BIO_meth_set_read(meth, bio_read);
  BIO_meth_set_puts(meth, bio_puts);
  BIO_meth_set_gets(meth, bio_gets);
  BIO_meth_set_ctrl(meth, bio_ctrl);
  BIO_meth_set_create(meth, bio_create);
  BIO_meth_set_destroy(meth, bio_destroy);
  return meth;
}
} // namespace

namespace {
void writecb(struct ev_loop *loop, ev_io *w, int revents) {}
} // namespace

namespace {
void readcb(struct ev_loop *loop, ev_io *w, int revents) {
  auto c = static_cast<Client *>(w->data);

  if (c->on_read() != 0) {
    c->disconnect();
  }
}
} // namespace

namespace {
void timeoutcb(struct ev_loop *loop, ev_timer *w, int revents) {
  auto c = static_cast<Client *>(w->data);

  debug::print_timestamp();
  std::cerr << "Timeout" << std::endl;

  c->disconnect();
}
} // namespace

Client::Client(struct ev_loop *loop, SSL_CTX *ssl_ctx)
    : remote_addr_{},
      max_pktlen_(0),
      loop_(loop),
      ssl_ctx_(ssl_ctx),
      ssl_(nullptr),
      fd_(-1),
      ncread_(0),
      nsread_(0),
      conn_(nullptr),
      crypto_ctx_{} {
  ev_io_init(&wev_, writecb, 0, EV_WRITE);
  ev_io_init(&rev_, readcb, 0, EV_READ);
  wev_.data = this;
  rev_.data = this;
  ev_timer_init(&timer_, timeoutcb, 5., 0.);
  timer_.data = this;
}

Client::~Client() { disconnect(); }

void Client::disconnect() {
  ev_timer_stop(loop_, &timer_);

  ev_io_stop(loop_, &rev_);
  ev_io_stop(loop_, &wev_);

  if (conn_) {
    ngtcp2_conn_del(conn_);
    conn_ = nullptr;
  }

  if (ssl_) {
    SSL_free(ssl_);
    ssl_ = nullptr;
  }

  if (fd_ != -1) {
    close(fd_);
    fd_ = -1;
  }
}

namespace {
ssize_t send_client_initial(ngtcp2_conn *conn, uint32_t flags,
                            uint64_t *ppkt_num, const uint8_t **pdest,
                            void *user_data) {
  auto c = static_cast<Client *>(user_data);

  if (c->tls_handshake() != 0) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }

  *ppkt_num = std::uniform_int_distribution<uint64_t>(
      0, std::numeric_limits<int32_t>::max())(randgen);

  auto len = c->read_client_handshake(pdest);

  return len;
}
} // namespace

namespace {
ssize_t send_client_cleartext(ngtcp2_conn *conn, uint32_t flags,
                              const uint8_t **pdest, void *user_data) {
  auto c = static_cast<Client *>(user_data);

  if (c->tls_handshake() != 0) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }

  auto len = c->read_client_handshake(pdest);

  return len;
}
} // namespace

namespace {
int recv_handshake_data(ngtcp2_conn *conn, const uint8_t *data, size_t datalen,
                        void *user_data) {
  auto c = static_cast<Client *>(user_data);

  c->write_server_handshake(data, datalen);

  if (c->tls_handshake() != 0) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }

  return 0;
}
} // namespace

namespace {
int handshake_completed(ngtcp2_conn *conn, void *user_data) {
  auto c = static_cast<Client *>(user_data);

  debug::handshake_completed(conn, user_data);

  if (c->setup_crypto_context() != 0) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }

  return 0;
}
} // namespace

namespace {
ssize_t do_encrypt(ngtcp2_conn *conn, uint8_t *dest, size_t destlen,
                   const uint8_t *plaintext, size_t plaintextlen,
                   const uint8_t *key, size_t keylen, const uint8_t *nonce,
                   size_t noncelen, const uint8_t *ad, size_t adlen,
                   void *user_data) {
  auto c = static_cast<Client *>(user_data);

  auto nwrite = c->encrypt_data(dest, destlen, plaintext, plaintextlen, key,
                                keylen, nonce, noncelen, ad, adlen);
  if (nwrite < 0) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }

  return nwrite;
}
} // namespace

namespace {
ssize_t do_decrypt(ngtcp2_conn *conn, uint8_t *dest, size_t destlen,
                   const uint8_t *ciphertext, size_t ciphertextlen,
                   const uint8_t *key, size_t keylen, const uint8_t *nonce,
                   size_t noncelen, const uint8_t *ad, size_t adlen,
                   void *user_data) {
  auto c = static_cast<Client *>(user_data);

  auto nwrite = c->decrypt_data(dest, destlen, ciphertext, ciphertextlen, key,
                                keylen, nonce, noncelen, ad, adlen);
  if (nwrite < 0) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }

  return nwrite;
}
} // namespace

int Client::init(int fd, const Address &remote_addr) {
  int rv;

  remote_addr_ = remote_addr;

  switch (remote_addr_.su.storage.ss_family) {
  case AF_INET:
    max_pktlen_ = NGTCP2_MAX_PKTLEN_IPV4;
    break;
  case AF_INET6:
    max_pktlen_ = NGTCP2_MAX_PKTLEN_IPV6;
    break;
  default:
    return -1;
  }

  fd_ = fd;
  ssl_ = SSL_new(ssl_ctx_);
  auto bio = BIO_new(create_bio_method());
  BIO_set_data(bio, this);
  SSL_set_bio(ssl_, bio, bio);
  SSL_set_app_data(ssl_, this);
  SSL_set_connect_state(ssl_);

  auto callbacks = ngtcp2_conn_callbacks{
      send_client_initial,
      send_client_cleartext,
      nullptr,
      recv_handshake_data,
      debug::send_pkt,
      debug::send_frame,
      debug::recv_pkt,
      debug::recv_frame,
      handshake_completed,
      debug::recv_version_negotiation,
      do_encrypt,
      do_decrypt,
  };

  auto conn_id = std::uniform_int_distribution<uint64_t>(
      0, std::numeric_limits<uint64_t>::max())(randgen);

  rv = ngtcp2_conn_client_new(&conn_, conn_id, NGTCP2_PROTO_VERSION, &callbacks,
                              this);
  if (rv != 0) {
    std::cerr << "ngtcp2_conn_client_new: " << ngtcp2_strerror(rv) << std::endl;
    return -1;
  }

  ev_io_set(&wev_, fd_, EV_WRITE);
  ev_io_set(&rev_, fd_, EV_READ);

  ev_io_start(loop_, &rev_);
  ev_timer_start(loop_, &timer_);

  return 0;
}

int Client::tls_handshake() {
  ERR_clear_error();

  auto rv = SSL_do_handshake(ssl_);
  if (rv <= 0) {
    auto err = SSL_get_error(ssl_, rv);
    switch (err) {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      return 0;
    case SSL_ERROR_SSL:
      std::cerr << "TLS handshake error: "
                << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
      return -1;
    default:
      std::cerr << "TLS handshake error: " << err << std::endl;
      return -1;
    }
  }

  ngtcp2_conn_handshake_completed(conn_);

  return 0;
}

int Client::feed_data(uint8_t *data, size_t datalen) {
  int rv;

  rv = ngtcp2_conn_recv(conn_, data, datalen, util::timestamp());
  if (rv != 0) {
    std::cerr << "ngtcp2_conn_recv: " << ngtcp2_strerror(rv) << std::endl;
    return -1;
  }

  return 0;
}

int Client::on_read() {
  std::array<uint8_t, 65536> buf;

  auto nread =
      recvfrom(fd_, buf.data(), buf.size(), MSG_DONTWAIT, nullptr, nullptr);

  if (nread == -1) {
    std::cerr << "recvfrom: " << strerror(errno) << std::endl;
    return 0;
  }

  if (feed_data(buf.data(), nread) != 0) {
    return -1;
  }

  return on_write();
}

int Client::on_write() {
  std::array<uint8_t, NGTCP2_MAX_PKTLEN_IPV4> buf;
  assert(buf.size() >= max_pktlen_);

  for (;;) {
    auto n =
        ngtcp2_conn_send(conn_, buf.data(), max_pktlen_, util::timestamp());
    if (n < 0) {
      std::cerr << "ngtcp2_conn_send: " << ngtcp2_strerror(n) << std::endl;
      return -1;
    }
    if (n == 0) {
      return 0;
    }

    auto nwrite = write(fd_, buf.data(), n);
    if (nwrite == -1) {
      std::cerr << "write: " << strerror(errno) << std::endl;
      return -1;
    }
  }
}

void Client::write_client_handshake(const uint8_t *data, size_t datalen) {
  std::copy_n(data, datalen, std::back_inserter(chandshake_));
}

size_t Client::read_client_handshake(const uint8_t **pdest) {
  auto n = chandshake_.size() - ncread_;
  *pdest = chandshake_.data() + ncread_;
  ncread_ = chandshake_.size();
  return n;
}

size_t Client::read_server_handshake(uint8_t *buf, size_t buflen) {
  auto n = std::min(buflen, shandshake_.size() - nsread_);
  std::copy_n(std::begin(shandshake_) + nsread_, n, buf);
  nsread_ += n;
  return n;
}

void Client::write_server_handshake(const uint8_t *data, size_t datalen) {
  std::copy_n(data, datalen, std::back_inserter(shandshake_));
}

int Client::setup_crypto_context() {
  int rv;

  rv = crypto::negotiated_prf(crypto_ctx_, ssl_);
  if (rv != 0) {
    return -1;
  }
  rv = crypto::negotiated_aead(crypto_ctx_, ssl_);
  if (rv != 0) {
    return -1;
  }

  auto length = EVP_MD_size(crypto_ctx_.prf);

  crypto_ctx_.secretlen = length;

  rv = crypto::export_client_secret(crypto_ctx_.tx_secret.data(),
                                    crypto_ctx_.secretlen, ssl_);
  if (rv != 0) {
    return -1;
  }

  std::array<uint8_t, 64> key{}, iv{};

  auto keylen = crypto::derive_packet_protection_key(
      key.data(), key.size(), crypto_ctx_.tx_secret.data(),
      crypto_ctx_.secretlen, crypto_ctx_);
  if (rv != 0) {
    return -1;
  }

  auto ivlen = crypto::derive_packet_protection_iv(
      iv.data(), iv.size(), crypto_ctx_.tx_secret.data(), crypto_ctx_.secretlen,
      crypto_ctx_);
  if (rv != 0) {
    return -1;
  }

  ngtcp2_conn_update_tx_keys(conn_, key.data(), keylen, iv.data(), ivlen);

  rv = crypto::export_server_secret(crypto_ctx_.rx_secret.data(),
                                    crypto_ctx_.secretlen, ssl_);
  if (rv != 0) {
    return -1;
  }

  keylen = crypto::derive_packet_protection_key(
      key.data(), key.size(), crypto_ctx_.rx_secret.data(),
      crypto_ctx_.secretlen, crypto_ctx_);
  if (rv != 0) {
    return -1;
  }

  ivlen = crypto::derive_packet_protection_iv(
      iv.data(), iv.size(), crypto_ctx_.rx_secret.data(), crypto_ctx_.secretlen,
      crypto_ctx_);
  if (rv != 0) {
    return -1;
  }

  ngtcp2_conn_update_rx_keys(conn_, key.data(), keylen, iv.data(), ivlen);

  ngtcp2_conn_set_aead_overhead(conn_, crypto::aead_max_overhead(crypto_ctx_));

  return 0;
}

ssize_t Client::encrypt_data(uint8_t *dest, size_t destlen,
                             const uint8_t *plaintext, size_t plaintextlen,
                             const uint8_t *key, size_t keylen,
                             const uint8_t *nonce, size_t noncelen,
                             const uint8_t *ad, size_t adlen) {
  return crypto::encrypt(dest, destlen, plaintext, plaintextlen, crypto_ctx_,
                         key, keylen, nonce, noncelen, ad, adlen);
}

ssize_t Client::decrypt_data(uint8_t *dest, size_t destlen,
                             const uint8_t *ciphertext, size_t ciphertextlen,
                             const uint8_t *key, size_t keylen,
                             const uint8_t *nonce, size_t noncelen,
                             const uint8_t *ad, size_t adlen) {
  return crypto::decrypt(dest, destlen, ciphertext, ciphertextlen, crypto_ctx_,
                         key, keylen, nonce, noncelen, ad, adlen);
}

namespace {
SSL_CTX *create_ssl_ctx() {
  auto ssl_ctx = SSL_CTX_new(TLS_method());

  SSL_CTX_set_min_proto_version(ssl_ctx, TLS1_3_VERSION);
  SSL_CTX_set_max_proto_version(ssl_ctx, TLS1_3_VERSION);

  SSL_CTX_set_default_verify_paths(ssl_ctx);

  assert(SSL_CTX_set1_curves_list(ssl_ctx, "P-256"));

  return ssl_ctx;
}
} // namespace

namespace {
int create_sock(Address &remote_addr, const char *addr, const char *port) {
  addrinfo hints{};
  addrinfo *res, *rp;
  int rv;

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  rv = getaddrinfo(addr, port, &hints, &res);
  if (rv != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
    return -1;
  }

  auto res_d = defer(freeaddrinfo, res);

  int fd = -1;

  for (rp = res; rp; rp = rp->ai_next) {
    fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd == -1) {
      continue;
    }

    if (connect(fd, rp->ai_addr, rp->ai_addrlen) == -1) {
      goto next;
    }

    break;

  next:
    close(fd);
  }

  if (!rp) {
    std::cerr << "Could not connect" << std::endl;
    return -1;
  }

  auto val = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val,
                 static_cast<socklen_t>(sizeof(val))) == -1) {
    return -1;
  }

  remote_addr.len = rp->ai_addrlen;
  memcpy(&remote_addr.su, rp->ai_addr, rp->ai_addrlen);

  return fd;
}

} // namespace

namespace {
int run(Client &c, const char *addr, const char *port) {
  int rv;
  Address remote_addr;

  auto fd = create_sock(remote_addr, addr, port);
  if (fd == -1) {
    return -1;
  }

  if (c.init(fd, remote_addr) != 0) {
    return -1;
  }

  c.on_write();

  ev_run(EV_DEFAULT, 0);

  return 0;
}
} // namespace

namespace {
void print_usage() { std::cerr << "Usage: client ADDR PORT" << std::endl; }
} // namespace

int main(int argc, char **argv) {
  for (;;) {
    static int flag = 0;
    constexpr static option long_opts[] = {{nullptr, 0, nullptr, 0}};

    auto optidx = 0;
    auto c = getopt_long(argc, argv, "", long_opts, &optidx);
    if (c == -1) {
      break;
    }
    switch (c) {
    case '?':
      print_usage();
      exit(EXIT_FAILURE);
    default:
      break;
    };
  }

  if (argc - optind < 2) {
    std::cerr << "Too few arguments" << std::endl;
    print_usage();
    exit(EXIT_FAILURE);
  }

  auto addr = argv[optind++];
  auto port = argv[optind++];

  auto ssl_ctx = create_ssl_ctx();
  auto ssl_ctx_d = defer(SSL_CTX_free, ssl_ctx);

  debug::reset_timestamp();

  if (isatty(STDOUT_FILENO)) {
    debug::set_color_output(true);
  }

  Client c(EV_DEFAULT, ssl_ctx);

  if (run(c, addr, port) != 0) {
    exit(EXIT_FAILURE);
  }
}
