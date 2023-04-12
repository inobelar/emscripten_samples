#include <imgui_app/ImGui_Application.hpp>

#include <imgui.h>

// -----------------------------------------------------------------------------
#include <cstdio>
#include <memory>

#include "bfbs_gen_lua.h"
#include "bfbs_gen_nim.h"
#include "flatbuffers/base.h"
#include "flatbuffers/code_generator.h"
#include "flatbuffers/flatc.h"
#include "flatbuffers/util.h"
#include "idl_gen_binary.h"
#include "idl_gen_cpp.h"
#include "idl_gen_csharp.h"
#include "idl_gen_dart.h"
#include "idl_gen_fbs.h"
#include "idl_gen_go.h"
#include "idl_gen_java.h"
#include "idl_gen_json_schema.h"
#include "idl_gen_kotlin.h"
#include "idl_gen_lobster.h"
#include "idl_gen_php.h"
#include "idl_gen_python.h"
#include "idl_gen_rust.h"
#include "idl_gen_swift.h"
#include "idl_gen_text.h"
#include "idl_gen_ts.h"
// -----------------------------------------------------------------------------

static const char *g_program_name = nullptr;

static void Warn(const flatbuffers::FlatCompiler *flatc,
                 const std::string &warn, bool show_exe_name) {
  (void)flatc;
  if (show_exe_name) { printf("%s: ", g_program_name); }
  fprintf(stderr, "\nwarning:\n  %s\n\n", warn.c_str());
}

static void Error(const flatbuffers::FlatCompiler *flatc,
                  const std::string &err, bool usage, bool show_exe_name) {
  if (show_exe_name) { printf("%s: ", g_program_name); }
  if (usage && flatc) {
    fprintf(stderr, "%s\n", flatc->GetShortUsageString(g_program_name).c_str());
  }
  fprintf(stderr, "\nerror:\n  %s\n\n", err.c_str());
  exit(1);
}

namespace flatbuffers
{
    void LogCompilerWarn(const std::string &warn)
    {
        Warn(static_cast<const flatbuffers::FlatCompiler *>(nullptr), warn, true);
    }
    
    void LogCompilerError(const std::string &err)
    {
        Error(static_cast<const flatbuffers::FlatCompiler *>(nullptr), err, false, true);
    }
}  // namespace flatbuffers

// -----------------------------------------------------------------------------

namespace fs
{
    struct FileData
    {
        std::string name;
        std::string content;
    };

    static std::vector<FileData> filesystem;
    
    // Signature the same as flatbuffers::LoadFileFunction
    bool LoadFile(const char* filename, bool binary, std::string *dest)
    {
        (void) binary; // Unused
        
        for(const FileData& f : filesystem)
        {
            // TODO: possibly here is must be 'directory' check
            if( strcmp(filename, f.name.c_str()) == 0 )
            {
                (*dest) = f.content;
                return true;
            }
        }
        
        return false;
    }
    
    // Signature the same as flatbuffers::FileExistsFunction
    bool FileExists(const char* filename)
    {
        for(const FileData& f : filesystem)
        {
            // TODO: possibly here is must be 'directory' check
            if( strcmp(filename, f.name.c_str()) == 0 )
            {
                return true;
            }
        }
        
        return false;
    }
};

class App : public ImGui_Application
{

public:

    bool init() override
    {
        if( !ImGui_Application::init() )
        {
            return false;
        }

        set_window_title("FlatBuffers compiler");
        
        flatbuffers::SetLoadFileFunction(fs::LoadFile);
        flatbuffers::SetFileExistsFunction(fs::FileExists);

        return true;
    }

    virtual ~App()
    {

    }

protected:

    void draw_ui() override
    {
        if(ImGui::Begin("Filesystem"))
        {
            for(const fs::FileData& f : fs::filesystem)
            {
                
            }
        }
        ImGui::End();
    }
};

int main()
{
    App app;
    if(!app.init())
        return 1;

    app.run_main_loop();

    return 0;
}
