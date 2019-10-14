#define STB_DEFINE 
#include "stb/stb.h"

#define STB_C_LEXER_IMPLEMENTATION 
#include "stb/stb_c_lexer.h"

#define STB_CONNECTED_COMPONENTS_IMPLEMENTATION
#define STBCC_GRID_COUNT_X_LOG2    10
#define STBCC_GRID_COUNT_Y_LOG2    10
#include "stb/stb_connected_components.h"

#define STB_DIVIDE_IMPLEMENTATION
// #define C_INTEGER_DIVISION_TRUNCATES  // see Note 1
// #define C_INTEGER_DIVISION_FLOORS     // see Note 2
#include "stb/stb_divide.h"

#define STB_DS_IMPLEMENTATION
#include "stb/stb_ds.h"

#define STB_DXT_IMPLEMENTATION
#include "stb/stb_dxt.h"

#include "stb/stb_easy_font.h"

#define STB_HERRINGBONE_WANG_TILE_IMPLEMENTATION
#include "stb/stb_herringbone_wang_tile.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define STB_INCLUDE_IMPLEMENTATION
#include "stb/stb_include.h"

#include "stb/stb_leakcheck.h"

#define STB_PERLIN_IMPLEMENTATION
#include "stb/stb_perlin.h"

#include "stb/stb_rect_pack.h"

#include "stb/stb_sprintf.h"

#include "stb/stb_textedit.h"

// 源码有语法错误，暂时不编译了:)
// #define STB_TILEMAP_EDITOR_IMPLEMENTATION
// void STBTE_DRAW_RECT(int x0, int y0, int x1, int y1, unsigned int color);
// void STBTE_DRAW_TILE(int x0, int y0, unsigned short id, int highlight, float *data);
// #include "stb/stb_tilemap_editor.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#define STB_VOXEL_RENDER_IMPLEMENTATION
#define STBVOX_CONFIG_MODE 0
#include "stb/stb_voxel_render.h"

#include "stb/stretchy_buffer.h"
