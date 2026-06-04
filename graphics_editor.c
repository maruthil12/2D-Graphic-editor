
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ROWS        25
#define COLS        60

#define BG_CHAR    '_'
#define DRAW_CHAR  '*'

#define MAX_OBJECTS 100

typedef enum {
    SHAPE_LINE = 1,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;
typedef struct {
    int r0, c0, r1, c1;         
} LineParams;
typedef struct {
    int r0, c0, r1, c1;          
} RectParams;

typedef struct {
    int cr, cc, radius;           
} CircleParams;
typedef struct {
    int r0, c0, r1, c1, r2, c2;
} TriangleParams;

typedef union {
    LineParams     line;
    RectParams     rect;
    CircleParams   circle;
    TriangleParams triangle;
} ShapeParams;

typedef struct {
    int        id;
    ShapeType  type;
    ShapeParams params;
} Object;
char   canvas[ROWS][COLS];
Object objects[MAX_OBJECTS];
int    object_count = 0;
int    next_id      = 1;

static int iabs(int x){
    return (x < 0) ? -x  :  x; 
}
static int isign(int x)        {
    return (x > 0) ?  1  : (x < 0) ? -1 : 0;
}
static void plot(int r, int c)
{
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
        canvas[r][c] = DRAW_CHAR;
}
static int read_int(const char *prompt, int lo, int hi)
{
    int v;
    for (;;) {
        printf("  %s [%d‥%d]: ", prompt, lo, hi);
        if (scanf("%d", &v) == 1 && v >= lo && v <= hi)
            return v;
        printf("    *** Out of range or invalid — try again.\n");
        while (getchar() != '\n'); 
    }
}
static const char *shape_name(ShapeType t)
{
    switch (t) {
        case SHAPE_LINE:      return "Line";
        case SHAPE_RECTANGLE: return "Rectangle";
        case SHAPE_CIRCLE:    return "Circle";
        case SHAPE_TRIANGLE:  return "Triangle";
        default:              return "Unknown";
    }
}

static void raster_line(int r0, int c0, int r1, int c1)
{
    int dr = iabs(r1 - r0), sr = isign(r1 - r0);
    int dc = iabs(c1 - c0), sc = isign(c1 - c0);
    int err = dr - dc;

    for (;;) {
        plot(r0, c0);
        if (r0 == r1 && c0 == c1) break;
        int e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r0 += sr; }
        if (e2 <  dr) { err += dr; c0 += sc; }
    }
}
static void raster_circle_points(int cr, int cc, int x, int y)
{
    plot(cr + x, cc + y);  plot(cr - x, cc + y);
    plot(cr + x, cc - y);  plot(cr - x, cc - y);
    plot(cr + y, cc + x);  plot(cr - y, cc + x);
    plot(cr + y, cc - x);  plot(cr - y, cc - x);
}
static void raster_circle(int cr, int cc, int radius)
{
    int x = 0, y = radius, d = 3 - 2 * radius;
    raster_circle_points(cr, cc, x, y);
    while (x <= y) {
        x++;
        if (d < 0) d += 4 * x + 6;
        else       { d += 4 * (x - y) + 10; y--; }
        raster_circle_points(cr, cc, x, y);
    }
}
static void raster_rectangle(int r0, int c0, int r1, int c1)
{
    raster_line(r0, c0, r0, c1);  
    raster_line(r1, c0, r1, c1);   
    raster_line(r0, c0, r1, c0);   
    raster_line(r0, c1, r1, c1);   
}
static void raster_triangle(int r0, int c0, int r1, int c1, int r2, int c2)
{
    raster_line(r0, c0, r1, c1);
    raster_line(r1, c1, r2, c2);
    raster_line(r2, c2, r0, c0);
}
