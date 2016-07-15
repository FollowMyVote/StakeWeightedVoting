#ifndef TLSPSKSERVER_HPP
#define TLSPSKSERVER_HPP

#include <memory>
#include <functional>
#include <vector>

namespace kj { template<typename T> class Own; class AsyncIoStream; }

// Forward declare some types from Botan
namespace Botan { using byte = uint8_t; namespace TLS { class Alert; class Session; } }

namespace fmv {

class TlsPskAdaptorFactory {
protected:
    std::unique_ptr<struct FactoryEquipment> equipment;

public:
    using GetPskFunction = std::function<std::vector<uint8_t>(std::string)>;

    TlsPskAdaptorFactory(GetPskFunction&& getPskForAccount);

    kj::Own<kj::AsyncIoStream> addClientTlsAdaptor(kj::Own<kj::AsyncIoStream>&& stream);
    kj::Own<kj::AsyncIoStream> addServerTlsAdaptor(kj::Own<kj::AsyncIoStream>&& stream);
};

} // namespace fmv
#endif // TLSPSKSERVER_HPP
