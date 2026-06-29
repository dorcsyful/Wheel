/* Redirect to glad so imgui_impl_opengl3.cpp finds GLES2 declarations
 * on Windows, where there is no system GLES2 SDK. */
#include "../glad.h"