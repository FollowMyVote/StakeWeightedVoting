#ifndef TLSPSKSERVER_HPP
#define TLSPSKSERVER_HPP

#include <kj/memory.h>

#include <functional>
#include <vector>

namespace kj { class AsyncIoStream; }

// Forward declare some types from Botan
namespace Botan { using byte = uint8_t; namespace TLS { class Alert; class Session; } }

namespace fmv {
class TlsPskAdaptor;

class TlsPskAdaptorFactory {
protected:
    kj::Own<struct FactoryEquipment> equipment;

public:
    using GetPskFunction = std::function<std::vector<uint8_t>(const std::string&)>;

    TlsPskAdaptorFactory(GetPskFunction&& getPskForAccount, std::string myAccountName);
    ~TlsPskAdaptorFactory();

    kj::Own<fmv::TlsPskAdaptor> addClientTlsAdaptor(kj::Own<kj::AsyncIoStream>&& stream);
    kj::Own<fmv::TlsPskAdaptor> addServerTlsAdaptor(kj::Own<kj::AsyncIoStream>&& stream);
};

} // namespace fmv
#endif // TLSPSKSERVER_HPP
