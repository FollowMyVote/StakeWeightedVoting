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
    kj::Array<kj::byte> packedMessage;
    kj::ArrayOutputStream outs;
public:
    template<typename Reader>
    ReaderPacker(Reader content) : outs(packedMessage) {
        capnp::MallocMessageBuilder message;
        message.setRoot(content);
        packedMessage = kj::heapArray<kj::byte>(capnp::computeSerializedSizeInWords(message) * capnp::BYTES_PER_WORD);
        capnp::writePackedMessage(outs, message);
    }
    ReaderPacker(capnp::MessageBuilder& message) : outs(packedMessage) {
        packedMessage = kj::heapArray<kj::byte>(capnp::computeSerializedSizeInWords(message) * capnp::BYTES_PER_WORD);
        capnp::writePackedMessage(outs, message);
    }

    ReaderPacker(ReaderPacker&& other)
        : packedMessage(kj::mv(other.packedMessage)), outs(packedMessage) {}
    kj::ArrayPtr<kj::byte> array() {
        return outs.getArray();
    }
};

} // namespace swv
#endif // UTILITIES_HPP
