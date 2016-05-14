#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <capnp/serialize-packed.h>

#include <fc/io/raw_variant.hpp>
#include <fc/crypto/digest.hpp>

#include <map.capnp.h>

namespace swv {

template <typename T>
inline T unpack(capnp::Data::Reader r) {
    return fc::raw::unpack<T>(std::vector<char>(r.begin(), r.end()));
}
inline fc::sha256 digest(capnp::Data::Reader r) {
    return fc::digest(std::vector<char>(r.begin(), r.end()));
}

inline std::map<std::string, std::string> convertMap(::Map<capnp::Text, capnp::Text>::Reader map) {
    std::map<std::string, std::string> result;
    for (const auto& pair : map.getEntries())
        result[pair.getKey()] = pair.getValue();
    return result;
}

struct BlobMessageReader {
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

} // namespace swv
#endif // UTILITIES_HPP
