#ifndef EMSCRIPTEN_VERSION_H
#define EMSCRIPTEN_VERSION_H

#if defined(__EMSCRIPTEN__)

// Make sure, that Emscripten's version is presented as 'uint8_t', otherwise
// packing in EMSCRIPTEN_VERSION_CHECK may be wrong
static_assert (__EMSCRIPTEN_major__ <= 255, "Emscripten::version.major is out of range");
static_assert (__EMSCRIPTEN_minor__ <= 255, "Emscripten::version.minor is out of range");
static_assert (__EMSCRIPTEN_tiny__  <= 255, "Emscripten::version.tiny  is out of range");

// Inspired by Qt :: QT_VERSION & QT_VERSION_CHECK from <qglobal.h>

/*
   EMSCRIPTEN_VERSION is (major << 16) + (minor << 8) + tiny.
*/
#define EMSCRIPTEN_VERSION \
    EMSCRIPTEN_VERSION_CHECK(__EMSCRIPTEN_major__, __EMSCRIPTEN_minor__, __EMSCRIPTEN_tiny__)

/*
   can be used like: #if (EMSCRIPTEN_VERSION >= EMSCRIPTEN_VERSION_CHECK(2, 0, 16))
*/
#define EMSCRIPTEN_VERSION_CHECK(major, minor, tiny) \
    ( (major << 16) | (minor << 8) | (tiny) )

// -----------------------------------------------------------------------------

#define __EMSCRIPTEN_VERSION__STR_HELPER(x) #x
#define __EMSCRIPTEN_VERSION__STR(x) __EMSCRIPTEN_VERSION__STR_HELPER(x)

#define EMSCRIPTEN_VERSION_MAJOR_STR __EMSCRIPTEN_VERSION__STR(__EMSCRIPTEN_major__)
#define EMSCRIPTEN_VERSION_MINOR_STR __EMSCRIPTEN_VERSION__STR(__EMSCRIPTEN_minor__)
#define EMSCRIPTEN_VERSION_TINY_STR  __EMSCRIPTEN_VERSION__STR(__EMSCRIPTEN_tiny__)

/*
    can be used like: printf("Emscripten version: %s", EMSCRIPTEN_VERSION_STR)
 */
#define EMSCRIPTEN_VERSION_STR \
    (EMSCRIPTEN_VERSION_MAJOR_STR "." EMSCRIPTEN_VERSION_MINOR_STR "." EMSCRIPTEN_VERSION_TINY_STR)

#endif // defined(__EMSCRIPTEN__)

#endif // EMSCRIPTEN_VERSION_H
