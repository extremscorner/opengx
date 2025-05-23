/*****************************************************************************
Copyright (c) 2011  David Guillen Fandos (david@davidgf.net)
All rights reserved.

Attention! Contains pieces of code from others such as Mesa and GRRLib

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of copyright holders nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#ifndef OPENGX_H
#define OPENGX_H

#include <GL/gl.h>
#include <ogc/gu.h>
#include <ogc/gx.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ogx_initialize(void);
void *ogx_get_proc_address(const char *proc);

/* Enable or disable double buffering. This is actually a choice entirely made
 * by the integration library, but opengx must be informed about it in order to
 * select the right value for the glReadBuffer() and glDrawBuffer() functions.
 *
 * If this is not called, opengx assumes double buffering is on.
 */
int ogx_enable_double_buffering(int double_buffering);

/* The display integration library (SDL, GLUT, etc.) should call this function
 * before copying the EFB to the XFB (and optionally, drawing a cursor). The
 * opengx library might need to restore the EFB (in case it was configured into
 * some special rendering mode); if this function returns a negative number,
 * then the integration library should not proceed with the swap buffers
 * operation. This will typically happen if the GL rendering mode has been set
 * to something other than GL_RENDER.
 */
int ogx_prepare_swap_buffers(void);

/* This function can be called to register an optimized converter for the
 * texture data (used in glTex*Image* functions).
 *
 * The "format" and "internal_format" parameter corresponds to the homonymous
 * parameters passed to the glTexImage2D() function, and the "converter"
 * parameter must be one of the ogx_fast_conv_* variables declared below.
 *
 * The following fast converters are already enabled by default:
 * - ogx_fast_conv_RGB_RGB565;
 * - ogx_fast_conv_RGBA_RGBA8;
 * - ogx_fast_conv_Intensity_I8;
 */
void ogx_register_tex_conversion(GLenum format, GLenum internal_format,
                                 uintptr_t converter);

extern uintptr_t ogx_fast_conv_RGBA_I8;
extern uintptr_t ogx_fast_conv_RGBA_A8;
extern uintptr_t ogx_fast_conv_RGBA_IA8;
extern uintptr_t ogx_fast_conv_RGBA_RGB565;
extern uintptr_t ogx_fast_conv_RGBA_RGBA8;
extern uintptr_t ogx_fast_conv_RGB_I8;
extern uintptr_t ogx_fast_conv_RGB_IA8;
extern uintptr_t ogx_fast_conv_RGB_RGB565;
extern uintptr_t ogx_fast_conv_RGB_RGBA8;
extern uintptr_t ogx_fast_conv_LA_I8;
extern uintptr_t ogx_fast_conv_LA_A8;
extern uintptr_t ogx_fast_conv_LA_IA8;
extern uintptr_t ogx_fast_conv_Intensity_I8;
extern uintptr_t ogx_fast_conv_Alpha_A8;

typedef enum {
    OGX_STENCIL_NONE = 0,
    /* Don't worry about Z buffer being updated even if a fragment fails the
     * stencil test. */
    OGX_STENCIL_DIRTY_Z = 1 << 0,
    OGX_STENCIL_8BIT = 1 << 1,
} OgxStencilFlags;

void ogx_stencil_create(OgxStencilFlags flags);

/* Support for GLSL emulation */

typedef struct {
    /* *_first: number of the first available resource
     * *_end: number of the first *not* available resource
     *
     * The number of available resources is X_end - X_first. Each member
     * specifies the number starting from zero, so that in order to get the ID
     * of the desired resource, you need to add the base ID of the resource:
     * for example, to get the actual stage number, you'd have to do
     *
     *     stage = number + GX_TEVSTAGE0
     *
     * and, for matrix types,
     *
     *     texmtx = number * 3 + GX_TEXMTX0
     *
     * Fields are named according to libogc's constants, to minimize confusion.
     */
    uint8_t tevstage_first;
    uint8_t tevstage_end;
    uint8_t kcolor_first;
    uint8_t kcolor_end;
    uint8_t tevreg_first;
    uint8_t tevreg_end;
    uint8_t texcoord_first;
    uint8_t texcoord_end;
    uint8_t pnmtx_first;
    uint8_t pnmtx_end;
    uint8_t dttmtx_first;
    uint8_t dttmtx_end;
    uint8_t texmtx_first;
    uint8_t texmtx_end;
    uint8_t texmap_first;
    uint8_t texmap_end;
    /* We could add the VTXFMT here too, if we decided to reserve them for
     * specific goals; for the time being, we only use GX_VTXFMT0 and set it up
     * from scratch every time. */
} OgxGpuResources;

extern OgxGpuResources *ogx_gpu_resources;

typedef struct _OgxDrawMode {
    uint8_t mode;
    bool loop;
} OgxDrawMode;

typedef struct _OgxDrawData {
    OgxDrawMode gxmode;
    GLsizei count;

    /* for drawing arrays: */
    GLint first;

    /* for drawing elements: */
    GLenum type;
    const GLvoid *indices;
} OgxDrawData;

typedef struct {
    bool (*compile_shader)(GLuint shader);
    void (*shader_source)(GLuint shader, GLsizei count,
                          const GLchar *const*string, const GLint *length);
    GLenum (*link_program)(GLuint program);
} OgxProgramProcessor;
void ogx_shader_register_program_processor(const OgxProgramProcessor *processor);

typedef enum {
    OGX_VARIABLE_ALLOCATE = 1 << 16,
    OGX_VARIABLE_ARRAY = 1 << 17, /* TODO: read an additional argument
                                     specifying the size */
    OGX_VARIABLE_VERTEX_PTR = 1 << 18,
} OgxVariableFlags;

uint32_t ogx_shader_get_source_hash(GLuint shader);

GXTexObj *ogx_shader_get_texobj(int texture_unit);

/* These can be called from the compile_shader callback.
 * In the variable argument list you should pass a pair (name, type),
 * for example:
 *
 * ogx_shader_add_uniforms(shader_id, 2,
 *                         "MVP", GL_FLOAT_MAT4,
 *                         "vPos", GL_FLOAT_VEC3);
 *
 * OpenGX will not reallocate the names, the client must ensure that the memory
 * won't get deallocated.
 * */
void ogx_shader_add_uniforms(GLuint shader, int count, ...);
/* Similar to ogx_shader_add_uniforms, but the shader needs to specify the GX
 * type of the destination attribute:
 *
 * ogx_shader_add_attributes(shader_id, 2,
 *                           "vPos", GL_FLOAT_VEC3, GX_VA_POS,
 *                           "color", GL_FLOAT_VEC4, GX_VA_CLR0);
 */
void ogx_shader_add_attributes(GLuint shader, int count, ...);

/* These can be called from the link_program callback */
typedef void (*OgxCleanupCb)(void *data);
typedef void (*OgxSetupDrawCb)(GLuint program, const OgxDrawData *draw_data,
                               void *user_data);
typedef void (*OgxSetupMatricesCb)(GLuint program, void *user_data);
typedef void (*OgxDrawDoneCb)(GLuint program, void *user_data);
void ogx_shader_program_set_user_data(GLuint program,
                                      void *data, OgxCleanupCb cleanup);
void ogx_shader_program_set_setup_matrices_cb(GLuint program,
                                              OgxSetupMatricesCb callback);
void ogx_shader_program_set_setup_draw_cb(GLuint program,
                                          OgxSetupDrawCb callback);
void ogx_shader_program_set_draw_done_cb(GLuint program,
                                         OgxDrawDoneCb callback);

/* These can be called from the setup_draw callback */
void ogx_shader_setup_attribute_array(int index, uint8_t gx_attr,
                                      const OgxDrawData *draw_data);
void *ogx_shader_get_data(GLuint shader);

void ogx_shader_set_projection_gx(const Mtx44 matrix);
static inline void ogx_shader_set_projection_gl(const GLfloat *matrix)
{
    Mtx44 m;
    for (int i = 0; i < 16; i++) {
        m[i % 4][i / 4] = matrix[i];
    }
    ogx_shader_set_projection_gx(m);
}

static inline void ogx_matrix_gl_to_mtx(const GLfloat *matrix, Mtx m)
{
    for (int i = 0; i < 12; i++) {
        int row = i / 4;
        int col = i % 4;
        m[row][col] = matrix[col * 4 + row];
    }
}

void ogx_shader_set_modelview_gx(const Mtx matrix);
static inline void ogx_shader_set_modelview_gl(const GLfloat *matrix)
{
    Mtx m;
    ogx_matrix_gl_to_mtx(matrix, m);
    ogx_shader_set_modelview_gx(m);
}

/* Many OpenGL 2.0+ apps pass a uniform with the model-view-projection matrix
 * to the vertex shader. This function decomposes it into MV and projection
 * matrices, and uploads them separately to GX. */
void ogx_shader_set_mvp_gl(const GLfloat *matrix);

#ifdef __cplusplus
} // extern C
#endif

#endif /* OPENGX_H */
