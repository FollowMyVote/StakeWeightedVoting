#include "BackendConfiguration.hpp"
#include "Utilities.hpp"

#include <capnp/serialize.h>

#include <kj/debug.h>

#include <fcntl.h>
#include <errno.h>

namespace swv {

BackendConfiguration::BackendConfiguration() {}

void BackendConfiguration::open(kj::StringPtr configFilePath, bool createIfMissing) {
    auto fd = ::open(configFilePath.cStr(), O_RDONLY);

    if (!createIfMissing)
        KJ_REQUIRE(fd >= 0, "Failed to open file for reading", configFilePath, strerror(errno));
    else if (fd < 0) {
        KJ_LOG(WARNING, "Creating new configuration", configFilePath);
        filePath = kj::heapString(configFilePath);
        save();
        config = message.getRoot<Config>();
        return;
    }

    kj::AutoCloseFd closer(fd);
    capnp::readMessageCopyFromFd(fd, message);
    config = message.getRoot<Config>();
    filePath = kj::heapString(configFilePath);
}

void BackendConfiguration::load(capnp::Data::Reader serialConfig) {
    BlobMessageReader reader(serialConfig);
    message.setRoot(reader->getRoot<Config>());
    config = message.getRoot<Config>();
}

void BackendConfiguration::save() {
    auto fd = ::open(configFilePath().cStr(), O_CREAT | O_WRONLY);
    KJ_REQUIRE(fd >= 0, "Failed to open file for writing", configFilePath(), strerror(errno));
    kj::AutoCloseFd closer(fd);
    capnp::writeMessageToFd(fd, message);
}

} // namespace swv
