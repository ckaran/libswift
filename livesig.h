/*
 *  crypto.h
 *
 *  Created by Arno Bakker
 *  Copyright 2013-2016 Vrije Universiteit Amsterdam. All rights reserved.
 */
#ifndef SWIFT_LIVESIG_H_
#define SWIFT_LIVESIG_H_

// Length of signature in SIGNED_INTEGRITY when Content Integrity Protection off
#define SWIFT_CIPM_NONE_KEYLEN	21	// bytes, must be larger than Sha1Hash::SIZE
#define SWIFT_CIPM_NONE_SIGLEN  20 	// bytes


#ifdef OPENSSL

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>

#else

// Dummy funcs, so swift will compile for VOD and live with no CIPM without OpenSSL
typedef uint8_t	EVP_PKEY;
typedef int	EVP_MD_CTX;
#define EVP_PKEY_free(x)
#define EVP_PKEY_size(x)	SWIFT_CIPM_NONE_SIGLEN
#endif

namespace swift {

// http://www.iana.org/assignments/dns-sec-alg-numbers/dns-sec-alg-numbers.xml
typedef enum {
	POPT_LIVE_SIG_ALG_RSASHA1 = 5,
	POPT_LIVE_SIG_ALG_ECDSAP256SHA256 = 13,
	POPT_LIVE_SIG_ALG_ECDSAP384SHA384 = 14,
	POPT_LIVE_SIG_ALG_PRIVATEDNS = 253
} popt_live_sig_alg_t;


/** Structure for holding a signature */
struct Signature
{
    uint8_t    *sigbits_;
    uint16_t   siglen_;
    Signature() : sigbits_(NULL), siglen_(0)  {}
    Signature(uint8_t *sb, uint16_t len);
    Signature(bool hex, const uint8_t *sb, uint16_t len);
    Signature(const Signature &copy);
    Signature & operator = (const Signature &source);
    ~Signature();
    uint8_t  *bits()  { return sigbits_; }
    uint16_t length() { return siglen_; }
    std::string hex() const;

    const static Signature NOSIG;
};


#define SWIFT_RSA_DEFAULT_KEYSIZE	1024


struct SwarmPubKey;

typedef void (*simple_openssl_callback_t)(int);

struct KeyPair
{
  public:
    KeyPair() // keep compiler happy
    {
	alg_ = POPT_LIVE_SIG_ALG_PRIVATEDNS;
	evp_ = NULL;
    }
    KeyPair(popt_live_sig_alg_t alg,EVP_PKEY *evp)
    {
	alg_ = alg;
	evp_ = evp;
    }
    ~KeyPair()
    {
	if (evp_ != NULL)
	    EVP_PKEY_free(evp_);
	evp_ = NULL;

    }
    static KeyPair *Generate(popt_live_sig_alg_t alg, uint16_t keysize=SWIFT_RSA_DEFAULT_KEYSIZE, simple_openssl_callback_t callback=NULL);
    EVP_PKEY       *GetEVP() { return evp_; }
    SwarmPubKey    *GetSwarmPubKey();

    Signature *Sign(uint8_t *data, uint16_t datalength);
    bool Verify(uint8_t *data, uint16_t datalength,Signature &sig);

    popt_live_sig_alg_t	GetSigAlg() { return alg_; }

    /** Returns the number of bytes a signature takes on the wire */
    uint16_t	    GetSigSizeInBytes();

  protected:
    popt_live_sig_alg_t	alg_;
    EVP_PKEY		*evp_;
};

/** -08: SwarmID for live streams is an Algorithm Byte followed by a public key
 * encoded as in a DNSSEC DNSKEY resource record without BASE-64 encoding.
 */
struct SwarmPubKey
{
  public:
    SwarmPubKey() : bits_(NULL), len_(0)  {}
    SwarmPubKey(uint8_t	*bits, uint16_t len);
    SwarmPubKey(const SwarmPubKey& copy);
    SwarmPubKey(std::string hexstr);
    ~SwarmPubKey();
    SwarmPubKey & operator = (const SwarmPubKey &source);
    bool    operator == (const SwarmPubKey& b) const;
    uint8_t  *bits()  { return bits_; }
    uint16_t length() { return len_; }
    std::string hex() const;
    KeyPair *GetPublicKeyPair() const;

    const static SwarmPubKey NOSPUBKEY;
  protected:
    uint8_t	*bits_;
    uint16_t    len_;
};

}

#endif /* SWIFT_LIVESIG_H_ */
