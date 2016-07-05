#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include "capnp/datagram.capnp.h"

#include <capnp/serialize-packed.h>

#ifdef HAVE_GRAPHENE
#include <fc/io/raw_variant.hpp>
#include <fc/crypto/digest.hpp>
#endif

#include <map.capnp.h>

#include <map>
#include <vector>
#include <string>

namespace swv {

#ifdef HAVE_GRAPHENE
template <typename T>
inline T unpack(capnp::Data::Reader r) {
    return fc::raw::unpack<T>(std::vector<char>(r.begin(), r.end()));
}
inline fc::sha256 digest(capnp::Data::Reader r) {
    return fc::digest(std::vector<char>(r.begin(), r.end()));
}
#endif

inline std::map<std::string, std::string> convertMap(::Map<capnp::Text, capnp::Text>::Reader map) {
    std::map<std::string, std::string> result;
    for (const auto& pair : map.getEntries())
        result[pair.getKey()] = pair.getValue();
    return result;
}

inline capnp::Data::Reader readerOf(const std::vector<char>& data) {
    return kj::ArrayPtr<const kj::byte>((const kj::byte*)data.data(), data.size());
}

class BlobMessageReader {
    kj::ArrayInputStream stream;
    capnp::PackedMessageReader reader;
public:
    BlobMessageReader(capnp::Data::Reader blob)
        : stream(blob), reader(stream) {}

    capnp::PackedMessageReader* operator->() {
        return &reader;
    }
    const capnp::PackedMessageReader* operator->() const {
        return &reader;
    }
    capnp::PackedMessageReader& operator*() {
        return reader;
    }
    const capnp::PackedMessageReader& operator*() const {
        return reader;
    }
};

class ReaderPacker {
    capnp::MallocMessageBuilder message;
    kj::Array<kj::byte> packedMessage;
    kj::ArrayOutputStream outs;

    template<typename Reader>
    kj::Array<kj::byte> makeSpace(Reader content) {
        message.setRoot(content);
        return kj::heapArray<kj::byte>(capnp::computeSerializedSizeInWords(message) * capnp::BYTES_PER_WORD);
    }

public:
    template<typename Reader>
    ReaderPacker(Reader content) : packedMessage(makeSpace(content)), outs(packedMessage) {
        capnp::writePackedMessage(outs, message);
    }

    ReaderPacker(ReaderPacker&& other)
        : packedMessage(kj::mv(other.packedMessage)), outs(packedMessage) {}
    kj::ArrayPtr<kj::byte> array() {
        return outs.getArray();
    }
};

inline bool operator== (const ::Datagram::DatagramKey::Reader& a, const ::Datagram::DatagramKey::Reader& b) {
    if (a.getKey().which() != b.getKey().which())
        return false;
    if (a.getKey().isContestKey()) {
        auto ac = a.getKey().getContestKey().getCreator();
        auto bc = b.getKey().getContestKey().getCreator();
        if (ac.which() != bc.which())
            return false;
        if (ac.isAnonymous())
            return bc.isAnonymous();
        return ac.getSignature().getId() == bc.getSignature().getId() &&
                ac.getSignature().getSignature() == bc.getSignature().getSignature();
    }
    auto ad = a.getKey().getDecisionKey().getBalanceId();
    auto bd = b.getKey().getDecisionKey().getBalanceId();
    return ad.getAccountInstance() == bd.getAccountInstance() && ad.getCoinInstance() == bd.getCoinInstance();
}
inline bool operator!= (const ::Datagram::DatagramKey::Reader& a, const ::Datagram::DatagramKey::Reader& b) {
    return !(a==b);
}

} // namespace swv
#endif // UTILITIES_HPP
