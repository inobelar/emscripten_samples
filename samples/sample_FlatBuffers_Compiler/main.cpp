#include <imgui_app/ImGui_Application.hpp>

#include <imgui.h>

#include <imgui_stdlib.h>

#include <deque>

#include "emscripten_utils/emscripten_localfile.hpp"

// -----------------------------------------------------------------------------
// via: https://stackoverflow.com/a/44326935/

#include <cctype>  // <ctype.h>  for isspace()

/** 
 * Parse out the next non-space word from a string.
 * @note No nullptr protection
 * @param str  [IN]   Pointer to pointer to the string. Nested pointer to string will be changed.
 * @param word [OUT]  Pointer to pointer of next word. To be filled.
 * @return  pointer to string - current cursor. Check it for '\0' to stop calling this function   
 */
static char* splitArgv(char **str, char **word)
{
    constexpr char QUOTE = '\'';
    bool inquotes = false;

    // optimization
    if( **str == 0 )
        return NULL;

    // Skip leading spaces.
    while (**str && isspace(**str)) 
        (*str)++;

    if( **str == '\0')
        return NULL;

    // Phrase in quotes is one arg
    if( **str == QUOTE ){
        (*str)++;
        inquotes = true;
    }

    // Set phrase begining
    *word = *str;

    // Skip all chars if in quotes
    if( inquotes ){
        while( **str && **str!=QUOTE )
            (*str)++;
        //if( **str!= QUOTE )
    }else{
        // Skip non-space characters.
        while( **str && !isspace(**str) )
            (*str)++;
    }
    // Null terminate the phrase and set `str` pointer to next symbol
    if(**str)
        *(*str)++ = '\0';

    return *str;
}


/// To support standart convetion last `argv[argc]` will be set to `NULL`
///\param[IN]  str : Input string. Will be changed - splitted to substrings
///\param[IN]  argc_MAX : Maximum a rgc, in other words size of input array \p argv
///\param[OUT] argc : Number of arguments to be filled
///\param[OUT] argv : Array of c-string pointers to be filled. All of these strings are substrings of \p str
///\return Pointer to the rest of string. Check if for '\0' and know if there is still something to parse. \
///        If result !='\0' then \p argc_MAX is too small to parse all. 
char* parseStrToArgcArgvInsitu( char *str, const int argc_MAX, int *argc, char* argv[] )
{
    *argc = 0;
    while( *argc<argc_MAX-1  &&  splitArgv(&str, &argv[*argc]) ){
        ++(*argc);
        if( *str == '\0' )
            break;
    }
    argv[*argc] = nullptr;
    return str;
};



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

namespace messages {
    
enum class MessageType {
    Info = 0,
    Warning,
    Error
};
    
struct Message
{
    MessageType type;
    std::string text;
    
    Message(MessageType type_, const std::string& text_)
        : type(type_)
        , text(text_)
    {}
};
    
static std::deque<Message> messages;
    
} // namespace messages


static const char *g_program_name = nullptr;

static void Warn(const flatbuffers::FlatCompiler *flatc,
                 const std::string &warn, bool show_exe_name) {
  // (void)flatc;
  // if (show_exe_name) { printf("%s: ", g_program_name); }
  // fprintf(stderr, "\nwarning:\n  %s\n\n", warn.c_str());
 
    messages::messages.emplace_back(messages::MessageType::Warning, warn);
}

static void Error(const flatbuffers::FlatCompiler *flatc,
                  const std::string &err, bool usage, bool show_exe_name) {
  // if (show_exe_name) { printf("%s: ", g_program_name); }
  // if (usage && flatc) {
  //   fprintf(stderr, "%s\n", flatc->GetShortUsageString(g_program_name).c_str());
  // }
  // fprintf(stderr, "\nerror:\n  %s\n\n", err.c_str());
  // exit(1);
    
    if(usage && flatc) {
        messages::messages.emplace_back(messages::MessageType::Info, flatc->GetShortUsageString(g_program_name));
    }
    messages::messages.emplace_back(messages::MessageType::Error, err);
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

        FileData(const std::string& name_, const std::string& content_)
            : name(name_)
            , content(content_)
        {}
    };

    static std::deque<FileData> filesystem;
    
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
    
    bool DirExists(const char* dirname)
    {
        for(const FileData& f : filesystem)
        {
            // TODO: possibly here is must be 'directory' check
            if( strstr(f.name.c_str(), dirname) != nullptr ) // substring found
            {
                return true;
            }
        }
        
        return false;
    }
    
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
    
    bool SaveFile(const char* name, const char* buf, size_t len, bool binary)
    {
        (void) binary; // Unused
        
        for(FileData& f : filesystem)
        {
            if( strcmp(name, f.name.c_str()) == 0 )
            {
                // File found - rewrite it
                f.content = {buf, len};
                return true;
            }
        }
        
        // File not found - create it
        filesystem.emplace_back(name, std::string{buf, len});
        return true;
    }
    
    void EnsureDirExists(const std::string& filepath)
    {
        // NOOP for our simple FS
    }
    
    std::string AbsolutePath(const std::string& filepath)
    {
        return filepath; // Currently not supported
    }

} // namespace fs

int compile(int argc, const char** argv)
{
  const std::string flatbuffers_version(flatbuffers::FLATBUFFERS_VERSION());

  flatbuffers::FlatCompiler::InitParams params;
  params.warn_fn = Warn;
  params.error_fn = Error;

  flatbuffers::FlatCompiler flatc(params);

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{
          "b", "binary", "",
          "Generate wire format binaries for any data definitions" },
      flatbuffers::NewBinaryCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "c", "cpp", "",
                                "Generate C++ headers for tables/structs" },
      flatbuffers::NewCppCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "n", "csharp", "",
                                "Generate C# classes for tables/structs" },
      flatbuffers::NewCSharpCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "d", "dart", "",
                                "Generate Dart classes for tables/structs" },
      flatbuffers::NewDartCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "", "proto", "",
                                "Input is a .proto, translate to .fbs" },
      flatbuffers::NewFBSCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "g", "go", "",
                                "Generate Go files for tables/structs" },
      flatbuffers::NewGoCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "j", "java", "",
                                "Generate Java classes for tables/structs" },
      flatbuffers::NewJavaCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "", "jsonschema", "", "Generate Json schema" },
      flatbuffers::NewJsonSchemaCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "", "kotlin", "",
                                "Generate Kotlin classes for tables/structs" },
      flatbuffers::NewKotlinCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "", "lobster", "",
                                "Generate Lobster files for tables/structs" },
      flatbuffers::NewLobsterCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "l", "lua", "",
                                "Generate Lua files for tables/structs" },
      flatbuffers::NewLuaBfbsGenerator(flatbuffers_version));

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "", "nim", "",
                                "Generate Nim files for tables/structs" },
      flatbuffers::NewNimBfbsGenerator(flatbuffers_version));

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "p", "python", "",
                                "Generate Python files for tables/structs" },
      flatbuffers::NewPythonCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "", "php", "",
                                "Generate PHP files for tables/structs" },
      flatbuffers::NewPhpCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "r", "rust", "",
                                "Generate Rust files for tables/structs" },
      flatbuffers::NewRustCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{
          "t", "json", "", "Generate text output for any data definitions" },
      flatbuffers::NewTextCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "", "swift", "",
                                "Generate Swift files for tables/structs" },
      flatbuffers::NewSwiftCodeGenerator());

  flatc.RegisterCodeGenerator(
      flatbuffers::FlatCOption{ "T", "ts", "",
                                "Generate TypeScript code for tables/structs" },
      flatbuffers::NewTsCodeGenerator());

  // Create the FlatC options by parsing the command line arguments.
  const flatbuffers::FlatCOptions &options =
      flatc.ParseFromCommandLineArguments(argc, argv);

  // Compile with the extracted FlatC options.
  return flatc.Compile(options);
}

// Based on: from https://github.com/google/flatbuffers/blob/master/tests/monster_test.fbs
//   Modification: commented 'include "include_test1.fbs";' for simplicity
static constexpr const char* EXAMPLE_SCHEMA =
R"EOF(// test schema file

//include "include_test1.fbs";

namespace MyGame;

table InParentNamespace {}

namespace MyGame.Example2;

table Monster {}  // Test having same name as below, but in different namespace.

namespace MyGame.Example;

attribute "priority";

/// Composite components of Monster color.
enum Color:ubyte (bit_flags) {
  Red = 0, // color Red = (1u << 0)
  /// \brief color Green
  /// Green is bit_flag with value (1u << 1)
  Green,
  /// \brief color Blue (1u << 3)
  Blue = 3,
}

enum Race:byte {
  None = -1,
  Human = 0,
  Dwarf,
  Elf,
}

enum LongEnum:ulong (bit_flags) {
  LongOne = 1,
  LongTwo = 2,
  // Because this is a bitflag, 40 will be out of range of a 32-bit integer,
  // allowing us to exercise any logic special to big numbers.
  LongBig = 40,
}

union Any { Monster, TestSimpleTableWithEnum, MyGame.Example2.Monster }

union AnyUniqueAliases { M: Monster, TS: TestSimpleTableWithEnum, M2: MyGame.Example2.Monster }
union AnyAmbiguousAliases { M1: Monster, M2: Monster, M3: Monster }

struct Test { a:short; b:byte; }

table TestSimpleTableWithEnum (csharp_partial, private) {
  color: Color = Green;
}

struct Vec3 (force_align: 8) {
  x:float;
  y:float;
  z:float;
  test1:double;
  test2:Color;
  test3:Test;
}

struct Ability {
  id:uint(key);
  distance:uint;
}

struct StructOfStructs {
  a: Ability;
  b: Test;
  c: Ability;
}

struct StructOfStructsOfStructs {
 a: StructOfStructs;
}

table Stat {
  id:string;
  val:long;
  count:ushort (key);
}

table Referrable {
  id:ulong(key, hash:"fnv1a_64");
}

/// an example documentation comment: "monster object"
table Monster {
  pos:Vec3 (id: 0);
  hp:short = 100 (id: 2);
  mana:short = 150 (id: 1);
  name:string (id: 3, key);
  color:Color = Blue (id: 6);
  inventory:[ubyte] (id: 5);
  friendly:bool = false (deprecated, priority: 1, id: 4);
  /// an example documentation comment: this will end up in the generated code
  /// multiline too
  testarrayoftables:[Monster] (id: 11);
  testarrayofstring:[string] (id: 10);
  testarrayofstring2:[string] (id: 28);
  testarrayofbools:[bool] (id: 24);
  testarrayofsortedstruct:[Ability] (id: 29);
  enemy:MyGame.Example.Monster (id:12);  // Test referring by full namespace.
  test:Any (id: 8);
  test4:[Test] (id: 9);
  test5:[Test] (id: 31);
  testnestedflatbuffer:[ubyte] (id:13, nested_flatbuffer: "Monster");
  testempty:Stat (id:14);
  testbool:bool (id:15);
  testhashs32_fnv1:int (id:16, hash:"fnv1_32");
  testhashu32_fnv1:uint (id:17, hash:"fnv1_32");
  testhashs64_fnv1:long (id:18, hash:"fnv1_64");
  testhashu64_fnv1:ulong (id:19, hash:"fnv1_64");
  testhashs32_fnv1a:int (id:20, hash:"fnv1a_32");
  testhashu32_fnv1a:uint (id:21, hash:"fnv1a_32", cpp_type:"Stat");
  testhashs64_fnv1a:long (id:22, hash:"fnv1a_64");
  testhashu64_fnv1a:ulong (id:23, hash:"fnv1a_64");
  testf:float = 3.14159 (id:25);
  testf2:float = 3 (id:26);
  testf3:float (id:27);
  flex:[ubyte] (id:30, flexbuffer);
  vector_of_longs:[long] (id:32);
  vector_of_doubles:[double] (id:33);
  parent_namespace_test:InParentNamespace (id:34);
  vector_of_referrables:[Referrable](id:35);
  single_weak_reference:ulong(id:36, hash:"fnv1a_64", cpp_type:"ReferrableT");
  vector_of_weak_references:[ulong](id:37, hash:"fnv1a_64", cpp_type:"ReferrableT");
  vector_of_strong_referrables:[Referrable](id:38, cpp_ptr_type:"default_ptr_type");                 //was shared_ptr
  co_owning_reference:ulong(id:39, hash:"fnv1a_64", cpp_type:"ReferrableT", cpp_ptr_type:"naked");  //was shared_ptr as well
  vector_of_co_owning_references:[ulong](id:40, hash:"fnv1a_64", cpp_type:"ReferrableT", cpp_ptr_type:"default_ptr_type", cpp_ptr_type_get:".get()");  //was shared_ptr
  non_owning_reference:ulong(id:41, hash:"fnv1a_64", cpp_type:"ReferrableT", cpp_ptr_type:"naked", cpp_ptr_type_get:"");                              //was weak_ptr
  vector_of_non_owning_references:[ulong](id:42, hash:"fnv1a_64", cpp_type:"ReferrableT", cpp_ptr_type:"naked", cpp_ptr_type_get:"");                 //was weak_ptr
  any_unique:AnyUniqueAliases(id:44);
  any_ambiguous:AnyAmbiguousAliases (id:46);
  vector_of_enums:[Color] (id:47);
  signed_enum:Race = None (id:48);
  testrequirednestedflatbuffer:[ubyte] (id:49, nested_flatbuffer: "Monster");
  scalar_key_sorted_tables:[Stat] (id: 50);
  native_inline:Test (id: 51, native_inline);
  // The default value of this enum will be a numeric zero, which isn't a valid
  // enum value.
  long_enum_non_enum_default:LongEnum (id: 52);
  long_enum_normal_default:LongEnum = LongOne (id: 53);
  // Test that default values nan and +/-inf work.
  nan_default:float = nan (id: 54);
  inf_default:float = inf (id: 55);
  positive_inf_default:float = +inf (id: 56);
  infinity_default:float = infinity (id: 57);
  positive_infinity_default:float = +infinity (id: 58);
  negative_inf_default:float = -inf (id: 59);
  negative_infinity_default:float = -infinity (id: 60);
  double_inf_default:double = inf (id: 61);
}

table TypeAliases {
    i8:int8;
    u8:uint8;
    i16:int16;
    u16:uint16;
    i32:int32;
    u32:uint32;
    i64:int64;
    u64:uint64;
    f32:float32;
    f64:float64;
    v8:[int8];
    vf64:[float64];
}

rpc_service MonsterStorage {
  Store(Monster):Stat (streaming: "none");
  Retrieve(Stat):Monster (streaming: "server", idempotent);
  GetMaxHitPoint(Monster):Stat (streaming: "client");
  GetMinMaxHitPoints(Monster):Stat (streaming: "bidi");
}

root_type Monster;

file_identifier "MONS";
file_extension "mon";
)EOF";

void set_next_window_initial_pos_and_size(float pos_x, float pos_y, float size_x, float size_y)
{
    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to this application a little more welcoming, but typically this isn't required.
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + pos_x, main_viewport->WorkPos.y + pos_y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(size_x, size_y), ImGuiCond_FirstUseEver);
}

class App : public ImGui_Application
{
    std::string new_file_name;
    std::string new_file_content;
            
    std::string args;
    
public:

    bool init() override
    {
        if( !ImGui_Application::init() )
        {
            return false;
        }

        set_window_title("FlatBuffers compiler");
        
        flatbuffers::SetFileExistsFunction(fs::FileExists);
        flatbuffers::SetDirExistsFunction(fs::DirExists);
        flatbuffers::SetLoadFileFunction(fs::LoadFile);
        flatbuffers::SetSaveFileFunction(fs::SaveFile);
        flatbuffers::SetEnsureDirExistsFunction(fs::EnsureDirExists);
        flatbuffers::SetAbsolutePathFunction(fs::AbsolutePath);
        
        g_program_name = "flatc_web";
        
        // For example - setup 'Example schema' and example arguments
        fs::filesystem.emplace_back("example.fbs", EXAMPLE_SCHEMA);
        args = "--cpp example.fbs";

        return true;
    }

    virtual ~App()
    {

    }

protected:

    void draw_ui() override
    {
        // Menu Bar
        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("About"))
            {
                ImGui::Text("Dear ImGui version: %s", IMGUI_VERSION);
                ImGui::Text("FlatBuffers version: %s", flatbuffers::FLATBUFFERS_VERSION());

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        set_next_window_initial_pos_and_size(15, 30, 460, 420);
        if(ImGui::Begin("Filesystem"))
        {
            for(auto it = fs::filesystem.begin(); it != fs::filesystem.end(); )
            {
                fs::FileData& f = (*it);

                constexpr ImGuiTreeNodeFlags tree_node_flags =
                    ImGuiTreeNodeFlags_AllowItemOverlap;

                const bool tree_opened = ImGui::TreeNodeEx(&f, tree_node_flags, "%s", f.name.c_str());
                ImGui::SameLine();
                
                ImGui::PushID(&f);
                const bool btn_remove_pressed = ImGui::SmallButton("[x]");
                ImGui::SameLine();
                const bool btn_save_pressed = ImGui::SmallButton("[save]");
                ImGui::PopID();
                
                
                if(tree_opened)
                {
                    ImGui::InputTextWithHint("##name", "filename", &f.name);

                    ImGui::InputTextMultiline("##content", &f.content, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 24));

                    ImGui::TreePop();
                }
                
                if(btn_save_pressed)
                {
                    ems_utils::localfile::save(f.content.data(), f.content.size(), f.name);
                }
                
                if(btn_remove_pressed)
                {
                    it = fs::filesystem.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            ImGui::Separator();
            
            const float half_avail_width = ImGui::GetContentRegionAvail().x / 2.0f;

            if( ImGui::Button("[+]", ImVec2(half_avail_width, ImGui::GetFontSize() * 2.0f)) )
            {
                new_file_name   .clear();
                new_file_content.clear();
                ImGui::OpenPopup("file_add_popup");
            }
            
            ImGui::SameLine();
            
            if( ImGui::Button("[Load]", ImVec2(half_avail_width, ImGui::GetFontSize() * 2.0f)) ) 
            {
                ems_utils::localfile::load(".fbs", [](const std::vector<std::uint8_t>& file_content, const std::string& file_name)
                {
                    fs::filesystem.emplace_back(file_name, std::string((const char*)file_content.data(), file_content.size()) );
                });
            }

            if( ImGui::BeginPopup("file_add_popup") )
            {
                ImGui::InputTextWithHint("##name", "filename", &new_file_name);
                ImGui::InputTextMultiline("##content", &new_file_content, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 24));

                if( ImGui::Button("Add", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 2.0f)) )
                {
                    fs::filesystem.emplace_back(new_file_name, new_file_content);
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }
        ImGui::End();
        
        set_next_window_initial_pos_and_size(480, 30, 775, 420);
        if( ImGui::Begin("Compiler") )
        {
            const bool pressed_args = ImGui::InputText("##args", &args, ImGuiInputTextFlags_EnterReturnsTrue );
            
            ImGui::SameLine();
            
            const bool pressed_compile_button = ImGui::Button("Compile");
            
            if( pressed_args || pressed_compile_button )
            {
                if(!args.empty())
                {
                    // Important: for corrent working, the first argument must be
                    // the same as program name, so we need to emulate it :)
                    const std::string full_args = std::string(g_program_name) + " " + args;
                    
                    constexpr size_t argc_MAX = 128;
                    char* argv[argc_MAX] = {0};
                    int argc = 0;

                    char* rest = parseStrToArgcArgvInsitu((char*)full_args.c_str(), argc_MAX, &argc, argv);
    
                    
                    compile(argc, (const char**)argv);
                }
            }
                
            
            if( ImGui::BeginChild("##log") )
            {
                constexpr ImGuiTableFlags TABLE_FLAGS =
                    ImGuiTableFlags_BordersInnerV  |
                    ImGuiTableFlags_NoSavedSettings;
                constexpr char TABLE_ID[] = "table_log";

                if( ImGui::BeginTable(TABLE_ID, 3, TABLE_FLAGS) )
                {
                    // Set columns, specially to set 'stretches'
                    ImGui::TableSetupColumn("Type",   ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Text",   ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Delete", ImGuiTableColumnFlags_WidthFixed);
                    // ImGui::TableHeadersRow(); // Commented to not show table header
            
                    for(auto it = messages::messages.begin(); it != messages::messages.end();)
                    {
                        const messages::Message& msg = (*it);
                        
                        char const* msg_type_str;
                        ImVec4      msg_type_color;
                        switch(msg.type) {
                            case messages::MessageType::Info: {
                                msg_type_str = "Info";
                                msg_type_color = {1.0, 1.0f, 1.0f, 1.0f};
                            } break;
                            case messages::MessageType::Warning: {
                                msg_type_str = "Warning";
                                msg_type_color = {1.0, 1.0f, 0.0f, 1.0f};   
                            } break;
                            case messages::MessageType::Error: {
                                msg_type_str = "Error";
                                msg_type_color = {1.0, 0.0f, 0.0f, 1.0f};   
                            } break;
                        }
                        
                        ImGui::TableNextColumn();
                        ImGui::TextColored(msg_type_color, "%s", msg_type_str);
                        
                        ImGui::TableNextColumn();
                        ImGui::TextWrapped("%s", msg.text.c_str());
                        
                        ImGui::TableNextColumn();
                        ImGui::PushID(&msg);
                        const bool btn_pressed = ImGui::Button("[x]");
                        ImGui::PopID();
                        
                        if( btn_pressed )
                        {
                            it = messages::messages.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }
                        
                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();
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
