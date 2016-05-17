#ifndef BACKENDCONFIGURATION_HPP
#define BACKENDCONFIGURATION_HPP

#include "config.capnp.h"

#include <capnp/message.h>

namespace kj { class InputStream; }

namespace swv {

class BackendConfiguration {
    kj::String filePath;

    capnp::MallocMessageBuilder message;
    Config::Builder config = message.initRoot<Config>();

public:
    BackendConfiguration();
    /**
     * @brief Convenience constructor which immediately calls @ref open
     */
    BackendConfiguration(kj::StringPtr configFilePath, bool createIfMissing = true) {
        open(configFilePath, createIfMissing);
    }

    /**
     * @brief Open the specified file and load config from it
     * @param configFilePath Path to config file to open
     * @param createIfMissing Whether or not to initialize the config file to defaults if it's missing
     *
     * @throws kj::Exception If opening or loading fails
     *
     * If loading fails, the loaded config may be corrupted, but the @ref configFilePath will not have been changed. If
     * a valid config was loaded from file before, @ref reload may be called to reload the config from that file.
     */
    void open(kj::StringPtr configFilePath, bool createIfMissing = true);
    /**
     * @brief Load a config from buffer without altering the config file
     * @param serialConfig Buffer to load config from
     *
     * serialConfig is expected to contain a binary-serialized config (not packed)
     *
     * This does not change @ref configFilePath nor the config file it references. If it is desired to overwrite the
     * config file with this new config, call @ref save. If it is desired to return to the config stored in the config
     * file, call @ref reload.
     */
    void load(capnp::Data::Reader serialConfig);
    /**
     * @brief Load a config from stream without altering the config file
     * @param serialConfigStream Stream to load config from
     *
     * serialConfigStream is expected to contain a binary-serialized config (not packed)
     *
     * This does not change @ref configFilePath nor the config file it references. If it is desired to overwrite the
     * config file with this new config, call @ref save. If it is desired to return to the config stored in the config
     * file, call @ref reload.
     */
    void load(kj::InputStream& serialConfigStream);
    /**
     * @brief Reload the config from the file referenced by @ref configFilePath, throwing away any changes made to the
     * config in memory.
     */
    void reload() {
        open(configFilePath());
    }
    /**
     * @brief Save the in-memory config to the file referenced by @ref configFilePath.
     */
    void save();

    /**
     * @brief Return the path to the config file on disk
     * @return Path to config file
     *
     * It is not guaranteed that the in-memory config matches the config on disk. To revert the in-memory config to the
     * on-disk config, call @ref reload; to save the in-memory config to disk, call @ref save.
     */
    kj::StringPtr configFilePath() {
        return filePath;
    }

    /**
     * @brief Get a Reader for the config
     */
    Config::Reader reader() const {
        return config.asReader();
    }
};

} // namespace swv
#endif // BACKENDCONFIGURATION_HPP
