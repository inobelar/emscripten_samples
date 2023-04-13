#ifndef EMSCRIPTEN_LOCALFILE_HPP
#define EMSCRIPTEN_LOCALFILE_HPP

#include <string>
#include <vector>
#include <functional>

namespace ems_utils {
namespace localfile {

using byte_array_t = std::vector<std::uint8_t>;
using file_load_callback_t = std::function<void(const byte_array_t& file_content, const std::string& file_name)>;

void load(const std::string& accept, file_load_callback_t fileDataReady);

void save(const void* content_bytes, std::size_t content_size, const std::string& fileNameHint);
void save(const byte_array_t& content, const std::string& fileNameHint);

} // namespace localfile
} // namespace ems_utils

#endif // EMSCRIPTEN_LOCALFILE_HPP
