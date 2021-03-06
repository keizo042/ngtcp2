ngtcp2
======

"Call it TCP/2.  One More Time."

ngtcp2 project is an effort to implement QUIC protocol which is now
being discussed in IETF QUICWG for its standardization.

Development status
------------------

First Implementation Draft
~~~~~~~~~~~~~~~~~~~~~~~~~~

We are focusing on implementing `First Implementation Draft
<https://github.com/quicwg/base-drafts/wiki/First-Implementation-Draft>`_
which is a subset of QUIC transport and QUIC TLS draft-04.

* https://tools.ietf.org/html/draft-ietf-quic-transport-04
* https://tools.ietf.org/html/draft-ietf-quic-tls-04

Requirements
------------

The libngtcp2 C library itself does not depend on any external
libraries.  It should compile with the modern C++ compilers on the
recent Linux.

The following packages are required to configure the build system:

* pkg-config >= 0.20
* autoconf
* automake
* autotools-dev
* libtool

libngtcp2 uses cunit for its unit test frame work:

* cunit >= 2.1

To build sources under the examples directory, libev is required:

* libev

The client and server under examples directory require boringssl or
OpenSSL (master branch) as crypto backend:

* boringssl (https://boringssl.googlesource.com/boringssl/)
* or, OpenSSL (https://github.com/openssl/openssl/)

At the of time writing, choosing crypto backend from them dictates
TLSv1.3 draft version.  boringssl implements TLSv1.3 draft-18.  On the
other hand, OpenSSL implements TLSv1.3 draft-21.  They are
incompatible.  If you want TLSv1.3 draft-18, choose boringssl.  If you
want TLSv1.3 draft-21, choose OpenSSL.  If you want to try draft-20,
pull OpenSSL master a95d7574dbcd91c734c1542a423e1cac34dc18b5.

To build boringssl, golang is required:

* golang

Build from git
--------------

If you choose boringssl, build it like so:

.. code-block:: text

   $ git clone https://boringssl.googlesource.com/boringssl
   $ cd boringssl
   $ mkdir build
   $ cd build
   $ cmake ..
   $ make -j$(nproc)
   $ cd ../../
   $ git clone https://github.com/ngtcp2/ngtcp2
   $ cd ngtcp2
   $ autoreconf -i
   $ ./configure OPENSSL_CFLAGS=-I$PWD/../boringssl/include OPENSSL_LIBS="-L$PWD/../boringssl/build/ssl -L$PWD/../boringssl/build/crypto -lssl -lcrypto -pthread"
   $ make -j$(nproc) check

Otherwise, you choose OpenSSL, build it like so:

.. code-block:: text

   $ git clone --depth 1 https://github.com/openssl/openssl
   $ cd openssl
   $ # For Linux
   $ ./Configure enable-tls1_3 --prefix=$PWD/build linux-x86_64
   $ make -j$(nproc)
   $ make install_sw
   $ cd ..
   $ git clone https://github.com/ngtcp2/ngtcp2
   $ cd ngtcp2
   $ autoreconf -i
   $ ./configure PKG_CONFIG_PATH=$PWD/../openssl/build/lib/pkgconfig LDFLAGS="-Wl,-rpath,$PWD/../openssl/build/lib"
   $ make -j$(nproc) check

Client/Server
-------------

After successful build, the client and server executable should be
found under examples directory.

.. code-block:: text

    $ examples/client 127.0.0.1 3000
    t=0.000245 TX Client Initial CID=98968335033bb4fa PKN=492337170 V=ff000005
        STREAM
        stream_id=00000000 offset=0 data_length=174
        PADDING
        length=1049
    t=0.002178 RX Server Cleartext CID=464e241ac7e5dc8c PKN=1925126390 V=ff000005
        ACK
        num_blks=0 num_ts=0 largest_ack=492337170 ack_delay=1665
        first_ack_block_length=0
        STREAM
        stream_id=00000000 offset=0 data_length=1200
    t=0.002614 TX Client Cleartext CID=464e241ac7e5dc8c PKN=492337171 V=ff000005
        ACK
        num_blks=0 num_ts=0 largest_ack=1925126390 ack_delay=435
        first_ack_block_length=0
    t=0.002643 RX Server Cleartext CID=464e241ac7e5dc8c PKN=1925126391 V=ff000005
        STREAM
        stream_id=00000000 offset=1200 data_length=215
    t=0.002899 TX Client Cleartext CID=464e241ac7e5dc8c PKN=492337172 V=ff000005
        ACK
        num_blks=0 num_ts=0 largest_ack=1925126391 ack_delay=256
        first_ack_block_length=0
        STREAM
        stream_id=00000000 offset=174 data_length=74
    t=0.002913 QUIC handshake has completed
    t=0.003003 TX Short 03 CID=464e241ac7e5dc8c PKN=492337173
        CONNECTION_CLOSE
        error_code=80000001 reason_length=0
    t=0.003308 RX Short 03 CID=464e241ac7e5dc8c PKN=1925126392
        ACK
        num_blks=0 num_ts=0 largest_ack=492337172 ack_delay=231
        first_ack_block_length=0
        CONNECTION_CLOSE
        error_code=80000001 reason_length=0
    t=5.004411 Timeout

.. code-block:: text

    $ examples/server 127.0.0.1 3000 server.key server.crt
    t=1.868633 RX Client Initial CID=98968335033bb4fa PKN=492337170 V=ff000005
        STREAM
        stream_id=00000000 offset=0 data_length=174
        PADDING
        length=1049
    t=1.870304 TX Server Cleartext CID=464e241ac7e5dc8c PKN=1925126390 V=ff000005
        ACK
        num_blks=0 num_ts=0 largest_ack=492337170 ack_delay=1665
        first_ack_block_length=0
        STREAM
        stream_id=00000000 offset=0 data_length=1200
    t=1.870344 TX Server Cleartext CID=464e241ac7e5dc8c PKN=1925126391 V=ff000005
        STREAM
        stream_id=00000000 offset=1200 data_length=215
    t=1.870825 RX Client Cleartext CID=464e241ac7e5dc8c PKN=492337171 V=ff000005
        ACK
        num_blks=0 num_ts=0 largest_ack=1925126390 ack_delay=435
        first_ack_block_length=0
    t=1.871189 RX Client Cleartext CID=464e241ac7e5dc8c PKN=492337172 V=ff000005
        ACK
        num_blks=0 num_ts=0 largest_ack=1925126391 ack_delay=256
        first_ack_block_length=0
        STREAM
        stream_id=00000000 offset=174 data_length=74
    t=1.871333 QUIC handshake has completed
    t=1.871420 TX Short 03 CID=464e241ac7e5dc8c PKN=1925126392
        ACK
        num_blks=0 num_ts=0 largest_ack=492337172 ack_delay=231
        first_ack_block_length=0
        CONNECTION_CLOSE
        error_code=80000001 reason_length=0
    t=1.871459 RX Short 03 CID=464e241ac7e5dc8c PKN=492337173
        CONNECTION_CLOSE
        error_code=80000001 reason_length=0
    t=6.873543 Timeout
    t=6.873663 Closing QUIC connection

License
-------

The MIT License

Copyright (c) 2016 ngtcp2 contributors
