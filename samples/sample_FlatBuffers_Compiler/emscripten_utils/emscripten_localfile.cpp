#include "emscripten_localfile.hpp"

#if defined(__EMSCRIPTEN__)

#include <emscripten.h>
#include <emscripten/html5.h>

#include "emscripten_version.hpp"

#if (EMSCRIPTEN_VERSION >= EMSCRIPTEN_VERSION_CHECK(1, 38, 27))
    #define POINTER_STRINGIFY_DISABLED true
#else
    #define POINTER_STRINGIFY_DISABLED false
#endif

//
// This file implements file load via HTML file input element and file save via browser download.
//

// Global user file data ready callback and C helper function. JavaScript will
// call this function when the file data is ready; the helper then forwards
// the call to the current handler function. This means there can be only one
// file open in proress at a given time.
using file_data_ready_callback_t = std::function<void(char* content, size_t content_size, const char* file_name)>;
file_data_ready_callback_t g_FileDataReadyCallback;

extern "C" EMSCRIPTEN_KEEPALIVE void callFileDataReady(char* content, size_t content_size, const char* file_name)
{
    if (g_FileDataReadyCallback == nullptr)
        return;

    g_FileDataReadyCallback(content, content_size, file_name);
    g_FileDataReadyCallback = nullptr;
}

namespace {
    void loadFile(const char *accept, file_data_ready_callback_t fileDataReady)
    {
        if (::g_FileDataReadyCallback)
            puts("Warning: Concurrent loadFile() calls are not supported. Cancelling earlier call");

        // Call callFileDataReady to make sure the emscripten linker does not
        // optimize it away, which may happen if the function is called from JavaScript
        // only. Set g_qtFileDataReadyCallback to null to make it a a no-op.
        ::g_FileDataReadyCallback = nullptr;
        ::callFileDataReady(nullptr, 0, nullptr);

        ::g_FileDataReadyCallback = fileDataReady;
        EM_ASM_({
            #if (POINTER_STRINGIFY_DISABLED)
                const accept = UTF8ToString($0);
            #else
                const accept = Pointer_stringify($0);
            #endif

            // Crate file file input which whil display the native file dialog
            var fileElement = document.createElement("input");
            document.body.appendChild(fileElement);
            fileElement.type = "file";
            fileElement.style = "display:none";
            fileElement.accept = accept;
            fileElement.onchange = function(event) {
                const files = event.target.files;

                // Read files
                for (var i = 0; i < files.length; i++) {
                    const file = files[i];
                    var reader = new FileReader();
                    reader.onload = function() {
                        const name = file.name;
                        var contentArray = new Uint8Array(reader.result);
                        const contentSize = reader.result.byteLength;

                        // Copy the file file content to the C++ heap.
                        // Note: this could be simplified by passing the content as an
                        // "array" type to ccall and then let it copy to C++ memory.
                        // However, this built-in solution does not handle files larger
                        // than ~15M (Chrome). Instead, allocate memory manually and
                        // pass a pointer to the C++ side (which will free() it when done).

                        // TODO: consider slice()ing the file to read it picewise and
                        // then assembling it in a QByteArray on the C++ side.

                        const heapPointer = _malloc(contentSize);
                        const heapBytes = new Uint8Array(Module.HEAPU8.buffer, heapPointer, contentSize);
                        heapBytes.set(contentArray);

                        // Null out the first data copy to enable GC
                        reader = null;
                        contentArray = null;

                        // Call the C++ file data ready callback
                        ccall("callFileDataReady", null,
                            ["number", "number", "string"], [heapPointer, contentSize, name]);
                    };
                    reader.readAsArrayBuffer(file);
                }

                // Clean up document
                document.body.removeChild(fileElement);

            }; // onchange callback

            // Trigger file dialog open
            fileElement.click();

        }, accept);
    }

    void saveFile(const char *contentPointer, size_t contentLength, const char *fileNameHint)
    {
        EM_ASM_({
            // Make the file contents and file name hint accessible to Javascript: convert
            // the char * to a JavaScript string and create a subarray view into the C heap.
            const contentPointer = $0;
            const contentLength  = $1;

            #if (POINTER_STRINGIFY_DISABLED)
                const fileNameHint = UTF8ToString($2);
            #else
                const fileNameHint = Pointer_stringify($2);
            #endif

            const fileContent = Module.HEAPU8.subarray(contentPointer, contentPointer + contentLength);

            // Create a hidden download link and click it programatically
            const fileblob = new Blob([fileContent], { type : "application/octet-stream" } );
            var link = document.createElement("a");
            document.body.appendChild(link);
            link.download = fileNameHint;
            link.href = window.URL.createObjectURL(fileblob);
            link.style = "display:none";
            link.click();
            document.body.removeChild(link);

        }, contentPointer, contentLength, fileNameHint);
    }
} // namespace

#undef POINTER_STRINGIFY_DISABLED

#endif


// -----------------------------------------------------------------------------

#if defined(__EMSCRIPTEN__)

/*!
    \brief Read local file via file dialog.
    Call this function to make the browser display an open-file dialog. This function
    returns immediately, and \a fileDataReady is called when the user has selected a file
    and the file contents has been read.
    \a The accept argument specifies which file types to accept, and must follow the
    <input type="file"> html standard formatting, for example ".png, .jpg, .jpeg".
    This function is implemented for WebAssembly only. A nonfunctional cross-
    platform stub is provided so that code that uses it can compile on all platforms.
*/
void ems_utils::localfile::load(const std::string &accept, ems_utils::localfile::file_load_callback_t fileDataReady)
{
    loadFile(accept.c_str(), [=](char *content, size_t content_size, const char *file_name) {

        // Copy file data into byte_array_t and free buffer that was allocated
        // on the JavaScript side. We could have used QByteArray::fromRawData()
        // to avoid the copy here, but that would make memory management awkward.
        byte_array_t file_content(content, (content + content_size));
        free(content);

        // Call user-supplied data ready callback
        fileDataReady(file_content, file_name);
    });
}

/*!
    \brief Write local file via browser download
    Call this function to make the browser start a file download. The file
    will contains the given \a content_bytes, with a suggested \a fileNameHint.
    This function is implemented for WebAssembly only. A nonfunctional cross-
    platform stub is provided so that code that uses it can compile on all platforms.
*/
void ems_utils::localfile::save(const void *content_bytes, std::size_t content_size, const std::string &fileNameHint)
{
    // Convert to C types and save
    saveFile((const char *)content_bytes, content_size, fileNameHint.c_str());
}

#else // !defined(__EMSCPRIPTEN__)

void ems_utils::localfile::load(const std::string &accept, ems_utils::localfile::file_load_callback_t fileDataReady)
{
    (void)accept;
    (void)fileDataReady;
}

void ems_utils::localfile::save(const void *content_bytes, std::size_t content_size, const std::string &fileNameHint)
{
    (void)content_bytes;
    (void)content_size;
    (void)fileNameHint;
}

#endif

void ems_utils::localfile::save(const ems_utils::localfile::byte_array_t &content, const std::string &fileNameHint)
{
    ems_utils::localfile::save(content.data(), content.size(), fileNameHint);
}
