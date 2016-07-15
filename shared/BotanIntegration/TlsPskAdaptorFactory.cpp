#include "TlsPskAdaptor.hpp"
#include "TlsPskAdaptorFactory.hpp"

#include <botan/rng.h>
#include <botan/tls_server.h>
#include <botan/tls_client.h>
#include <botan/credentials_manager.h>

#include <kj/async-io.h>

using namespace std::placeholders;

const static int SYMMETRIC_KEY_SIZE = 32;

namespace fmv {

class CredentialsManager : public Botan::Credentials_Manager {
    Botan::SymmetricKey sessionTicketKey;
    TlsPskAdaptorFactory::GetPskFunction getPskForAccount;
    std::string myIdentity;
public:
    CredentialsManager(TlsPskAdaptorFactory::GetPskFunction&& getPskForAccount, std::string myIdentity)
        : getPskForAccount(std::move(getPskForAccount)), myIdentity(myIdentity) {}
    virtual ~CredentialsManager();

    virtual std::string psk_identity(const std::string&, const std::string&, const std::string&) override {
        return myIdentity;
    }
    virtual Botan::SymmetricKey psk(const std::string& type, const std::string& context,
                                    const std::string& identity) override {
        if (type == "tls-server" && context == "session-ticket" && identity == "") {
            if (sessionTicketKey == Botan::SymmetricKey())
                sessionTicketKey = Botan::SymmetricKey(*Botan::RandomNumberGenerator::make_rng(), SYMMETRIC_KEY_SIZE);
            return sessionTicketKey;
        }
        return getPskForAccount(identity);
    }
};

class TlsPolicy : public Botan::TLS::Strict_Policy {
public:
    virtual ~TlsPolicy();

    virtual std::vector<std::string> allowed_ciphers() const override {
        return {"ChaCha20Poly1305"};
    }
    virtual std::vector<std::string> allowed_macs() const override {
        return {"AEAD"};
    }
    virtual std::vector<std::string> allowed_key_exchange_methods() const override {
        return {"ECDHE_PSK"};
    }
    virtual std::vector<std::string> allowed_signature_methods() const override {
        return {"ECDSA"};
    }
    virtual bool acceptable_protocol_version(Botan::TLS::Protocol_Version version) const override {
        return version == Botan::TLS::Protocol_Version::latest_tls_version();
    }
};

struct FactoryEquipment {
    Botan::RandomNumberGenerator& rng = *Botan::RandomNumberGenerator::make_rng();
    fmv::CredentialsManager credentialsManager;
    fmv::TlsPolicy policy;
    Botan::TLS::Session_Manager_In_Memory sessionManager;

    FactoryEquipment(TlsPskAdaptorFactory::GetPskFunction&& getPskForAccount, std::string myIdentity)
        : credentialsManager(std::move(getPskForAccount), myIdentity),
          sessionManager(rng) {}
};

TlsPskAdaptorFactory::TlsPskAdaptorFactory(GetPskFunction&& getPskForAccount, std::string myAccountName)
    : equipment(kj::heap<FactoryEquipment>(std::move(getPskForAccount), myAccountName)) {

}

TlsPskAdaptorFactory::~TlsPskAdaptorFactory() {}

kj::Own<kj::AsyncIoStream> TlsPskAdaptorFactory::addClientTlsAdaptor(kj::Own<kj::AsyncIoStream>&& stream) {
    auto adaptor = kj::heap<TlsPskAdaptor>(kj::mv(stream));
    adaptor->setChannel(kj::heap<Botan::TLS::Client>(adaptor->outputFunction(),
                                                     adaptor->dataCallback(),
                                                     adaptor->alertCallback(),
                                                     adaptor->handshakeCallback(),
                                                     equipment->sessionManager,
                                                     equipment->credentialsManager,
                                                     equipment->policy,
                                                     equipment->rng));
    return kj::mv(adaptor);
}

kj::Own<kj::AsyncIoStream> TlsPskAdaptorFactory::addServerTlsAdaptor(kj::Own<kj::AsyncIoStream>&& stream) {
    auto adaptor = kj::heap<TlsPskAdaptor>(kj::mv(stream));
    adaptor->setChannel(kj::heap<Botan::TLS::Server>(adaptor->outputFunction(),
                                                     adaptor->dataCallback(),
                                                     adaptor->alertCallback(),
                                                     adaptor->handshakeCallback(),
                                                     equipment->sessionManager,
                                                     equipment->credentialsManager,
                                                     equipment->policy,
                                                     equipment->rng));
    return kj::mv(adaptor);
}

// Implement these outside-of-class to squelch compiler warning about vtables in every translation unit
CredentialsManager::~CredentialsManager() {}
TlsPolicy::~TlsPolicy() {}

} // namespace fmv
