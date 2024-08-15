#include <AvailabilityMacros.h>
#include "fastfetch.h"
#include "opengl.h"

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h> // This brings in CGL, not GL

#if MAC_OS_X_VERSION_MIN_REQUIRED < 1070
#define kCGLPFAOpenGLProfile 99
#define kCGLOGLPVersion_3_2_Core 0x3200
#endif

void ffOpenGLHandleResult(FFOpenGLResult* result, __typeof__(&glGetString) ffglGetString);

static const char* cglHandleContext(FFOpenGLResult* result, CGLContextObj context)
{
    if(CGLSetCurrentContext(context) != kCGLNoError)
        return "CGLSetCurrentContext() failed";

    ffOpenGLHandleResult(result, &glGetString);

    GLint major, minor;
    CGLGetVersion(&major, &minor);
    ffStrbufSetF(&result->library, "CGL %d.%d", major, minor);

    return NULL;
}

static const char* cglHandlePixelFormat(FFOpenGLResult* result, CGLPixelFormatObj pixelFormat)
{
    CGLContextObj context;

    if(CGLCreateContext(pixelFormat, NULL, &context) != kCGLNoError)
        return "CGLCreateContext() failed";

    const char* error = cglHandleContext(result, context);
    CGLDestroyContext(context);
    return error;
}

const char* ffDetectOpenGL(FF_MAYBE_UNUSED FFOpenGLOptions* options, FFOpenGLResult* result)
{
    CGLPixelFormatObj pixelFormat;
    CGLPixelFormatAttribute attrs[] = {
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
        kCGLPFAAccelerated,
        0
    };

    GLint num;
    if (CGLChoosePixelFormat(attrs, &pixelFormat, &num) != kCGLNoError)
        return "CGLChoosePixelFormat() failed";

    const char* error = cglHandlePixelFormat(result, pixelFormat);
    CGLDestroyPixelFormat(pixelFormat);
    return error;
}
