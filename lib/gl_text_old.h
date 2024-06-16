#ifndef __GL_TEXT_H__
#define __GL_TEXT_H__

#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdint.h>
#include <stdlib.h>

#ifdef _WIN32
#define FONT_FILE_NAME "C:/Windows/Fonts/Calibri.ttf"
#else
#define FONT_FILE_NAME "/usr/share/fonts/truetype/noto/NotoMono-Regular.ttf"
#endif

#define glTextGetChar(char) glTextChars[char - 32]

typedef struct CharInfo {
    uint32_t texId;
    uint32_t width, height;
    uint32_t bitmap_left, bitmap_top;
    uint32_t advanceX;
} CharInfo;

const int maxTextHeight = 128;
// All ascii
CharInfo glTextChars[126 - 32];

void glTextInit() {
    FT_Library library; /* handle to library     */
    FT_Face face;       /* handle to face object */

    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        printf("Failed to init FreeType\n");
        return;
    }

    error = FT_New_Face(library,
                        FONT_FILE_NAME,
                        0,
                        &face);
    if (error == FT_Err_Unknown_File_Format) {
        printf("Failed to read font file: Unknown format\n");
        return;
    } else if (error) {
        printf("Failed to read font file\n");
        return;
    }

    error = FT_Set_Pixel_Sizes(face, 0, maxTextHeight);
    if (error) {
        printf("Failed to set char size\n");
        return;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int n = 0; n < 126 - 32; n++) {
        FT_Load_Char(face, n + ' ', FT_LOAD_RENDER);
        FT_GlyphSlot glyph = face->glyph;
        // printf("%dx%d\n", glyph->bitmap.width, glyph->bitmap.rows);
        // generate texture
        uint32_t texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        uint32_t* rgba_data = (uint32_t*)malloc(glyph->bitmap.width * glyph->bitmap.rows * sizeof(uint32_t));
        for (int i = 0; i < glyph->bitmap.width * glyph->bitmap.rows; i++) {
            // if (!glyph->bitmap.buffer[i])
            //     rgba_data[i] = 0xFF0000FF;
            // else
            rgba_data[i] = (glyph->bitmap.buffer[i] << 24) |
                           (glyph->bitmap.buffer[i] << 16) |
                           (glyph->bitmap.buffer[i] << 8) |
                           (glyph->bitmap.buffer[i]);
        }
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            glyph->bitmap.width,
            glyph->bitmap.rows,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            rgba_data);
        free(rgba_data);

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // printf("texture id: %u\n", texture);

        glTextChars[n].width = glyph->bitmap.width;
        glTextChars[n].height = glyph->bitmap.rows;
        glTextChars[n].bitmap_left = glyph->bitmap_left;
        glTextChars[n].bitmap_top = glyph->bitmap_top;
        glTextChars[n].advanceX = glyph->advance.x;
        glTextChars[n].texId = texture;
    }


    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

void glTextDrawChar(GLuint file, float x, float y, float w, float h, float angle) {
    // glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    glPushMatrix();
    // glTranslatef(x, y, 0.0);
    glRotatef(angle, 0.0, 0.0, 1.0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, file);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(x, y + h, 0.0f);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(x, y, 0.0f);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(x + w, y, 0.0f);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(x + w, y + h, 0.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

void glDrawString(char* str, uint32_t strLen, float x, float y, float charHeight) {
    float scale = charHeight / maxTextHeight;
    for (size_t i = 0; i < strLen; i++) {
        CharInfo* ch = &glTextGetChar(str[i]);
        float xpos = x + ch->bitmap_left * scale;
        float ypos = y - ((float)ch->height - ch->bitmap_top) * scale;

        float w = ch->width * scale;
        float h = ch->height * scale;
        // printf("%f, %f, %d, %d\n", xpos, ypos, ch->width, ch->height);
        // printf("%d, %d, %d\n", ch->width, ch->height - ch->bearingY, ch->advance >> 6);
        glTextDrawChar(ch->texId, xpos, ypos, w, h, 0.0);
        x += (ch->advanceX >> 6) * scale;
    }
}

uint32_t glCalculateStringWidth(char* str, uint32_t strLen, float charHeight) {
    float scale = charHeight / maxTextHeight;
    uint32_t width = 0;
    for (size_t i = 0; i < strLen; i++) {
        CharInfo* ch = &glTextGetChar(str[i]);
        width += (ch->advanceX >> 6) * scale;
    }
    return width + 0.5;
}

void glDrawStringCenter(char* str, uint32_t strLen, float x, float y, float charHeight) {
    uint32_t width = glCalculateStringWidth(str, strLen, charHeight);
    glDrawString(str, strLen, x - width / 2, y, charHeight);
}

#endif