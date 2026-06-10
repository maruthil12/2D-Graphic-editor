#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef USE_CURSES
#include <curses.h>
#else
#include <conio.h>
#endif
#include <math.h>
#define ROWS 25
#define COLS 60
#define BG_CHAR '_'
#define DRAW_CHAR '*'
#define CURSOR_CHAR '@'
#define MAX_OBJECTS 100

typedef enum{
    LINE=1,
    RECTANGLE,
    CIRCLE,
    TRIANGLE
}ShapeType;

typedef struct{
    int r0,c0,r1,c1;
}LineParams;

typedef struct{
    int r0,c0,r1,c1;
}RectParams;

typedef struct{
    int cr,cc,radius;
}CircleParams;

typedef struct{
    int r0,c0,r1,c1,r2,c2;
}TriangleParams;

typedef union{
    LineParams line;
    RectParams rect;
    CircleParams circle;
    TriangleParams triangle;
}ShapeParams;
typedef struct{
    int id;
    ShapeType type;
    ShapeParams params;
}Object;
char canvas[ROWS][COLS];
Object objects[MAX_OBJECTS];
int object_count=0;
int next_id=1;
int canvas_visible=0;
static int absoluteValue(int x){
    if(x<0)
    {
    return -x;
    }
    else
    {
    return x;
    }
}
static int getSign(int x){
    if(x>0)
        return 1;
    if(x<0)
        return -1;
    return 0;
}
static void plot(int r,int c){
    if(r>=0&&r<ROWS&&c>=0&&c<COLS)
        canvas[r][c]=DRAW_CHAR;
}

static void redraw_objects(void);
static const char *shape_name(ShapeType t);
static void show_shape_menu(void);
static void show_canvas_with_cursor(int cursor_r,int cursor_c,int fixed_count,const int fixed_r[],const int fixed_c[],char fixed_char);
static int pick_point(int *pr,int *pc,const char *prompt,int fixed_count,const int fixed_r[],const int fixed_c[],char fixed_char);
static void wait_for_key(void);

#ifdef USE_CURSES
static void init_ui(void){
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
}

static void shutdown_ui(void){
    endwin();
}

static void show_status(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int row = ROWS + 13;
    move(row, 0);
    clrtoeol();
    vw_printw(stdscr, fmt, ap);
    va_end(ap);
    refresh();
}

static void clear_screen(void){
    clear();
}

typedef enum{
    KEY_NONE,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ENTER,
    KEY_QUIT
}InputKey;

static InputKey read_input_key(void){
    int ch = getch();
    switch(ch){
        case KEY_UP: return KEY_UP;
        case KEY_DOWN: return KEY_DOWN;
        case KEY_LEFT: return KEY_LEFT;
        case KEY_RIGHT: return KEY_RIGHT;
        case 10:
        case 13:
            return KEY_ENTER;
        case 'q':
        case 'Q':
            return KEY_QUIT;
        case 'w':
        case 'W':
            return KEY_UP;
        case 's':
        case 'S':
            return KEY_DOWN;
        case 'a':
        case 'A':
            return KEY_LEFT;
        case 'd':
        case 'D':
            return KEY_RIGHT;
    }
    return KEY_NONE;
}

static void show_canvas_with_cursor(int cursor_r,int cursor_c,int fixed_count,const int fixed_r[],const int fixed_c[],char fixed_char){
    int width = COLS + 2;
    mvaddch(0, 0, '+');
    mvaddch(0, width - 1, '+');
    for(int c=1;c<width-1;++c)
        mvaddch(0, c, '-');
    for(int r=1;r<=ROWS;++r){
        mvaddch(r, 0, '|');
        mvaddch(r, width - 1, '|');
    }
    mvaddch(ROWS + 1, 0, '+');
    mvaddch(ROWS + 1, width - 1, '+');
    for(int c=1;c<width-1;++c)
        mvaddch(ROWS + 1, c, '-');

    for(int r=0;r<ROWS;++r){
        for(int c=0;c<COLS;++c){
            char ch = canvas[r][c];
            for(int i=0;i<fixed_count;++i){
                if(r==fixed_r[i] && c==fixed_c[i]){
                    ch = fixed_char;
                    break;
                }
            }
            if(r==cursor_r && c==cursor_c)
                ch = CURSOR_CHAR;
            mvaddch(r + 1, c + 1, ch);
        }
    }
    refresh();
}

static void show_canvas(void){
    int width = COLS + 2;
    mvaddch(0, 0, '+');
    mvaddch(0, width - 1, '+');
    for(int c=1;c<width-1;++c)
        mvaddch(0, c, '-');
    for(int r=1;r<=ROWS;++r){
        mvaddch(r, 0, '|');
        mvaddch(r, width - 1, '|');
        for(int c=0;c<COLS;++c)
            mvaddch(r, c + 1, canvas[r][c]);
    }
    mvaddch(ROWS + 1, 0, '+');
    mvaddch(ROWS + 1, width - 1, '+');
    for(int c=1;c<width-1;++c)
        mvaddch(ROWS + 1, c, '-');
    refresh();
}

static int read_int(const char *prompt,int lo,int hi){
    char buf[32];
    int v;
    while(1){
        mvprintw(ROWS + 11, 0, "  %s [%d..%d]: ", prompt, lo, hi);
        clrtoeol();
        echo();
        nocbreak();
        curs_set(1);
        refresh();
        getnstr(buf, sizeof(buf) - 1);
        noecho();
        cbreak();
        curs_set(0);
        if(sscanf(buf, "%d", &v) == 1 && v >= lo && v <= hi)
            return v;
        mvprintw(ROWS + 12, 0, "    *** Out of range or invalid - try again.");
        clrtoeol();
        refresh();
    }
}

static void show_shape_menu(void){
    mvprintw(ROWS + 4, 0, "Choose a shape to raster:");
    mvprintw(ROWS + 5, 0, "  1) Line");
    mvprintw(ROWS + 6, 0, "  2) Rectangle");
    mvprintw(ROWS + 7, 0, "  3) Circle");
    mvprintw(ROWS + 8, 0, "  4) Triangle");
    refresh();
}

static void list_objects(void){
    int base_row = ROWS + 4;
    if(object_count==0){
        mvprintw(base_row, 0, "No objects in the picture.");
        clrtoeol();
        refresh();
        return;
    }
    mvprintw(base_row, 0, "Objects in the picture:");
    clrtoeol();
    for(int i=0;i<object_count && i<10;++i){
        Object obj = objects[i];
        int row = base_row + 1 + i;
        move(row, 0);
        clrtoeol();
        printw("  id=%d %s ", obj.id, shape_name(obj.type));
        switch(obj.type){
            case LINE:
                printw("from (%d,%d) to (%d,%d)", obj.params.line.r0, obj.params.line.c0, obj.params.line.r1, obj.params.line.c1);
                break;
            case RECTANGLE:
                printw("corner1 (%d,%d) corner2 (%d,%d)", obj.params.rect.r0, obj.params.rect.c0, obj.params.rect.r1, obj.params.rect.c1);
                break;
            case CIRCLE:
                printw("center (%d,%d) radius %d", obj.params.circle.cr, obj.params.circle.cc, obj.params.circle.radius);
                break;
            case TRIANGLE:
                printw("(%d,%d) (%d,%d) (%d,%d)", obj.params.triangle.r0, obj.params.triangle.c0, obj.params.triangle.r1, obj.params.triangle.c1, obj.params.triangle.r2, obj.params.triangle.c2);
                break;
            default:
                break;
        }
    }
    if(object_count > 10){
        mvprintw(base_row + 11, 0, "  ...and %d more objects.", object_count - 10);
        clrtoeol();
    }
    refresh();
}

#else
static void init_ui(void){ }
static void shutdown_ui(void){ }
static void show_status(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    putchar('\n');
    va_end(ap);
}

static void clear_screen(void){
    system("cls");
}

typedef enum{
    KEY_NONE,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ENTER,
    KEY_QUIT
}InputKey;

static InputKey read_input_key(void){
    int ch=_getch();
    if(ch==0||ch==224){
        int arrow=_getch();
        switch(arrow){
            case 72: return KEY_UP;
            case 80: return KEY_DOWN;
            case 75: return KEY_LEFT;
            case 77: return KEY_RIGHT;
        }
    }
    else if(ch==13){
        return KEY_ENTER;
    }
    else if(ch=='q'||ch=='Q'){
        return KEY_QUIT;
    }
    else if(ch=='w'||ch=='W'){
        return KEY_UP;
    }
    else if(ch=='s'||ch=='S'){
        return KEY_DOWN;
    }
    else if(ch=='a'||ch=='A'){
        return KEY_LEFT;
    }
    else if(ch=='d'||ch=='D'){
        return KEY_RIGHT;
    }
    return KEY_NONE;
}

static void show_canvas_with_cursor(int cursor_r,int cursor_c,int fixed_count,const int fixed_r[],const int fixed_c[],char fixed_char){
    int width = COLS + 2;
    putchar('+');
    for(int c=1;c<width-1;++c)
        putchar('-');
    putchar('+');
    putchar('\n');

    for(int r=0;r<ROWS;++r){
        putchar('|');
        for(int c=0;c<COLS;++c){
            char ch = canvas[r][c];
            for(int i=0;i<fixed_count;++i){
                if(r==fixed_r[i] && c==fixed_c[i]){
                    ch = fixed_char;
                    break;
                }
            }
            if(r==cursor_r && c==cursor_c)
                ch = CURSOR_CHAR;
            putchar(ch);
        }
        putchar('|');
        putchar('\n');
    }

    putchar('+');
    for(int c=1;c<width-1;++c)
        putchar('-');
    putchar('+');
    putchar('\n');
}

static void show_canvas(void){
    int width = COLS + 2;
    putchar('+');
    for(int c=1;c<width-1;++c)
        putchar('-');
    putchar('+');
    putchar('\n');
    for(int r=0;r<ROWS;++r){
        putchar('|');
        for(int c=0;c<COLS;++c)
            putchar(canvas[r][c]);
        putchar('|');
        putchar('\n');
    }
    putchar('+');
    for(int c=1;c<width-1;++c)
        putchar('-');
    putchar('+');
    putchar('\n');
}

static int read_int(const char *prompt,int lo,int hi){
    int v;
    for(;;){
        printf("  %s [%d..%d]: ",prompt,lo,hi);
        if(scanf("%d",&v)==1&&v>=lo&&v<=hi)
            return v;
        printf("    *** Out of range or invalid - try again.\n");
        while(getchar()!='\n');
    }
}

static void show_shape_menu(void){
    printf("Choose a shape to raster:\n");
    printf("  1) Line\n");
    printf("  2) Rectangle\n");
    printf("  3) Circle\n");
    printf("  4) Triangle\n");
}

static void list_objects(void){
    if(object_count==0){
        printf("No objects in the picture.\n");
        return;
    }
    printf("Objects in the picture:\n");
    for(int i=0;i<object_count;++i){
        Object obj=objects[i];
        printf("  id=%d %s ",obj.id,shape_name(obj.type));
        switch(obj.type){
            case LINE:
                printf("from (%d,%d) to (%d,%d)",obj.params.line.r0,obj.params.line.c0,obj.params.line.r1,obj.params.line.c1);
                break;
            case RECTANGLE:
                printf("corner1 (%d,%d) corner2 (%d,%d)",obj.params.rect.r0,obj.params.rect.c0,obj.params.rect.r1,obj.params.rect.c1);
                break;
            case CIRCLE:
                printf("center (%d,%d) radius %d",obj.params.circle.cr,obj.params.circle.cc,obj.params.circle.radius);
                break;
            case TRIANGLE:
                printf("(%d,%d) (%d,%d) (%d,%d)",obj.params.triangle.r0,obj.params.triangle.c0,obj.params.triangle.r1,obj.params.triangle.c1,obj.params.triangle.r2,obj.params.triangle.c2);
                break;
            default:
                break;
        }
        putchar('\n');
    }
}
#endif

static int pick_point(int *pr,int *pc,const char *prompt,int fixed_count,const int fixed_r[],const int fixed_c[],char fixed_char){
    int r=ROWS/2;
    int c=COLS/2;
    for(;;){
        redraw_objects();
#ifdef USE_CURSES
        clear_screen();
        mvprintw(0, 0, "%s", prompt);
        mvprintw(1, 0, "Move the cursor from the canvas center.");
        mvprintw(2, 0, "Use arrow keys or WASD to move, Enter to select, Q to cancel.");
        show_canvas_with_cursor(r,c,fixed_count,fixed_r,fixed_c,fixed_char);
#else
        clear_screen();
        printf("%s\n",prompt);
        printf("Move the cursor from the canvas center.\n");
        printf("Use arrow keys or WASD to move, Enter to select, Q to cancel.\n");
        show_canvas_with_cursor(r,c,fixed_count,fixed_r,fixed_c,fixed_char);
#endif
        InputKey key=read_input_key();
        if(key==KEY_QUIT)
            return 0;
        if(key==KEY_ENTER){
            *pr=r;
            *pc=c;
            return 1;
        }
        if(key==KEY_UP && r>0)
            r--;
        else if(key==KEY_DOWN && r<ROWS-1)
            r++;
        else if(key==KEY_LEFT && c>0)
            c--;
        else if(key==KEY_RIGHT && c<COLS-1)
            c++;
    }
}

static int pick_circle_center_and_radius(Object *obj){
    int cr,cc;
    if(!pick_point(&cr,&cc,"Select circle center.",0,NULL,NULL,0))
        return 0;
    int pr,pc;
    if(!pick_point(&pr,&pc,"Select a point on the circle perimeter.",1,&cr,&cc,'*'))
        return 0;
    int dx=pr-cr;
    int dy=pc-cc;
    int radius=(int)(sqrt((double)(dx*dx+dy*dy))+0.5);
    if(radius<1)
        radius=1;
    (*obj).params.circle.cr=cr;
    (*obj).params.circle.cc=cc;
    (*obj).params.circle.radius=radius;
    return 1;
}

static void wait_for_key(void){
#ifdef USE_CURSES
    getch();
#else
    _getch();
#endif
}

static int pick_two_points(int *r0,int *c0,int *r1,int *c1,const char *first_prompt,const char *second_prompt){
    if(!pick_point(r0,c0,first_prompt,0,NULL,NULL,0))
        return 0;
    if(!pick_point(r1,c1,second_prompt,1,r0,c0,'*'))
        return 0;
    return 1;
}

static const char *shape_name(ShapeType t){
    switch(t){
        case LINE:
            return "Line";
        case RECTANGLE:
            return "Rectangle";
        case CIRCLE:
            return "Circle";
        case TRIANGLE:
            return "Triangle";
        default:
            return "Unknown";
    }
}
static void raster_line(int r0,int c0,int r1,int c1){
    int dx = absoluteValue(c1 - c0);
    int sx = getSign(c1 - c0);
    int dy = absoluteValue(r1 - r0);
    int sy = getSign(r1 - r0);
    int err = dx - dy;

    for(;;){
        plot(r0, c0);
        if(r0 == r1 && c0 == c1)
            break;
        int e2 = err + err;
        if(e2 > -dy){
            err -= dy;
            c0 += sx;
        }
        if(e2 < dx){
            err += dx;
            r0 += sy;
        }
    }
}

static void raster_circle_points(int cr,int cc,int x,int y){
    plot(cr + x, cc + y);
    plot(cr - x, cc + y);
    plot(cr + x, cc - y);
    plot(cr - x, cc - y);
    plot(cr + y, cc + x);
    plot(cr - y, cc + x);
    plot(cr + y, cc - x);
    plot(cr - y, cc - x);
}

static void raster_circle(int cr,int cc,int radius){
    int x = 0;
    int y = radius;
    int d = 1 - radius;

    raster_circle_points(cr, cc, x, y);
    while(y > x){
        if(d < 0){
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
        raster_circle_points(cr, cc, x, y);
    }
}

static void raster_rectangle(int r0,int c0,int r1,int c1){
    raster_line(r0,c0,r0,c1);
    raster_line(r1,c0,r1,c1);
    raster_line(r0,c0,r1,c0);
    raster_line(r0,c1,r1,c1);
}

static void raster_triangle(int r0,int c0,int r1,int c1,int r2,int c2){
    raster_line(r0,c0,r1,c1);
    raster_line(r1,c1,r2,c2);
    raster_line(r2,c2,r0,c0);
}

static void clear_canvas(void){
    for(int r=0;r<ROWS;++r){
        for(int c=0;c<COLS;++c)
            canvas[r][c]=BG_CHAR;
    }
}

static int find_object_index(int id){
    for(int i=0;i<object_count;++i){
        if(objects[i].id==id)
            return i;
    }
    return -1;
}

static void draw_object(Object obj){
    switch(obj.type){
        case LINE:
            raster_line(obj.params.line.r0,
                        obj.params.line.c0,
                        obj.params.line.r1,
                        obj.params.line.c1);
            break;
        case RECTANGLE:{
            int r0=obj.params.rect.r0;
            int c0=obj.params.rect.c0;
            int r1=obj.params.rect.r1;
            int c1=obj.params.rect.c1;
            if(r0>r1){ int t=r0; r0=r1; r1=t; }
            if(c0>c1){ int t=c0; c0=c1; c1=t; }
            raster_rectangle(r0,c0,r1,c1);
            break;
        }
        case CIRCLE:
            raster_circle(obj.params.circle.cr,
                          obj.params.circle.cc,
                          obj.params.circle.radius);
            break;
        case TRIANGLE:
            raster_triangle(obj.params.triangle.r0,
                            obj.params.triangle.c0,
                            obj.params.triangle.r1,
                            obj.params.triangle.c1,
                            obj.params.triangle.r2,
                            obj.params.triangle.c2);
            break;
        default:
            break;
    }
}

static void redraw_objects(void){
    clear_canvas();
    for(int i=0;i<object_count;++i)
        draw_object(objects[i]);
}

static void display_picture(void){
    clear_screen();
    if(canvas_visible)
        show_canvas();
}

static void add_object(void){
    if(object_count>=MAX_OBJECTS){
        show_status("*** Cannot add more than %d objects.",MAX_OBJECTS);
        return;
    }
    show_shape_menu();
    ShapeType type=(ShapeType)read_int("Shape number",1,4);
    Object obj;
    obj.type=type;
    int r0,c0,r1,c1,r2,c2;
    switch(type){
        case LINE:
            if(!pick_two_points(&r0,&c0,&r1,&c1,"Select start point for the line.","Select end point for the line.")){
                show_status("Line creation cancelled.");
                return;
            }
            obj.params.line.r0=r0;
            obj.params.line.c0=c0;
            obj.params.line.r1=r1;
            obj.params.line.c1=c1;
            break;
        case RECTANGLE:
            if(!pick_two_points(&r0,&c0,&r1,&c1,"Select first corner of the rectangle.","Select opposite corner of the rectangle.")){
                show_status("Rectangle creation cancelled.");
                return;
            }
            obj.params.rect.r0=r0;
            obj.params.rect.c0=c0;
            obj.params.rect.r1=r1;
            obj.params.rect.c1=c1;
            break;
        case CIRCLE:
            if(!pick_circle_center_and_radius(&obj)){
                show_status("Circle creation cancelled.");
                return;
            }
            break;
        case TRIANGLE:
            if(!pick_point(&r0,&c0,"Select first vertex of the triangle.",0,NULL,NULL,0)){
                show_status("Triangle creation cancelled.");
                return;
            }
            if(!pick_point(&r1,&c1,"Select second vertex of the triangle.",1,&r0,&c0,'*')){
                show_status("Triangle creation cancelled.");
                return;
            }
            {
                int fixed_r[2] = {r0, r1};
                int fixed_c[2] = {c0, c1};
                if(!pick_point(&r2,&c2,"Select third vertex of the triangle.",2,fixed_r,fixed_c,'*')){
                    show_status("Triangle creation cancelled.");
                    return;
                }
            }
            obj.params.triangle.r0=r0;
            obj.params.triangle.c0=c0;
            obj.params.triangle.r1=r1;
            obj.params.triangle.c1=c1;
            obj.params.triangle.r2=r2;
            obj.params.triangle.c2=c2;
            break;
        default:
            break;
    }
    obj.id = next_id++;
    objects[object_count++] = obj;
    canvas_visible = 1;
    show_status("Added %s with id %d.", shape_name(type), obj.id);
}

static void delete_object(void){
    if(object_count==0){
        show_status("No objects to delete.");
        return;
    }
    list_objects();
    int id=read_int("Object id to delete",1,next_id-1);
    int index=find_object_index(id);
    if(index<0){
        printf("*** No object with id %d.\n",id);
        return;
    }
    for(int i=index;i<object_count-1;++i)
        objects[i]=objects[i+1];
    object_count--;
    canvas_visible = (object_count > 0);
    show_status("Deleted object %d.", id);
}

static void modify_object(void){
    if(object_count==0){
        show_status("No objects to modify.");
        return;
    }
    list_objects();
    int id=read_int("Object id to modify",1,next_id-1);
    int index=find_object_index(id);
    if(index<0){
        printf("*** No object with id %d.\n",id);
        return;
    }
    Object *obj=&objects[index];
    show_status("Modifying %s id=%d",shape_name((*obj).type),(*obj).id);
    int r0,c0,r1,c1,r2,c2;
    switch((*obj).type){
        case LINE:
            if(!pick_two_points(&r0,&c0,&r1,&c1,"Select new start point for the line.","Select new end point for the line.")){
                show_status("Line modification cancelled.");
                return;
            }
            (*obj).params.line.r0=r0;
            (*obj).params.line.c0=c0;
            (*obj).params.line.r1=r1;
            (*obj).params.line.c1=c1;
            break;
        case RECTANGLE:
            if(!pick_two_points(&r0,&c0,&r1,&c1,"Select new first corner of the rectangle.","Select new opposite corner of the rectangle.")){
                show_status("Rectangle modification cancelled.");
                return;
            }
            (*obj).params.rect.r0=r0;
            (*obj).params.rect.c0=c0;
            (*obj).params.rect.r1=r1;
            (*obj).params.rect.c1=c1;
            break;
        case CIRCLE:
            if(!pick_circle_center_and_radius(obj)){
                show_status("Circle modification cancelled.");
                return;
            }
            break;
        case TRIANGLE:
            if(!pick_point(&r0,&c0,"Select new first vertex of the triangle.",0,NULL,NULL,0)){
                show_status("Triangle modification cancelled.");
                return;
            }
            if(!pick_point(&r1,&c1,"Select new second vertex of the triangle.",1,&r0,&c0,'*')){
                show_status("Triangle modification cancelled.");
                return;
            }
            {
                int fixed_r[2] = {r0, r1};
                int fixed_c[2] = {c0, c1};
                if(!pick_point(&r2,&c2,"Select new third vertex of the triangle.",2,fixed_r,fixed_c,'*')){
                    show_status("Triangle modification cancelled.");
                    return;
                }
            }
            (*obj).params.triangle.r0=r0;
            (*obj).params.triangle.c0=c0;
            (*obj).params.triangle.r1=r1;
            (*obj).params.triangle.c1=c1;
            (*obj).params.triangle.r2=r2;
            (*obj).params.triangle.c2=c2;
            break;
        default:
            break;
    }
    show_status("Modified object %d.", id);
    canvas_visible = 1;
}

int main(void){
    int choice;
    init_ui();
    clear_canvas();

    for(;;){
        clear_screen();
        redraw_objects();
#ifdef USE_CURSES
        if(canvas_visible)
            show_canvas();
        mvprintw(ROWS + 4, 0, "2D Graphic Editor Menu:");
        mvprintw(ROWS + 5, 0, "  1) Add object");
        mvprintw(ROWS + 6, 0, "  2) Delete object");
        mvprintw(ROWS + 7, 0, "  3) Modify object");
        mvprintw(ROWS + 8, 0, "  4) List objects");
        mvprintw(ROWS + 9, 0, "  5) Render canvas");
        mvprintw(ROWS + 10, 0, "  6) Quit");
        refresh();
#else
        if(canvas_visible)
            show_canvas();
        if(canvas_visible)
            printf("\n");
        printf("2D Graphic Editor Menu:\n");
        printf("  1) Add object\n");
        printf("  2) Delete object\n");
        printf("  3) Modify object\n");
        printf("  4) List objects\n");
        printf("  5) Render canvas\n");
        printf("  6) Quit\n");
#endif

        choice = read_int("Menu choice", 1, 6);
        switch(choice){
            case 1:
                add_object();
                redraw_objects();
                display_picture();
                show_status("Press any key to continue...");
                wait_for_key();
                break;
            case 2:
                delete_object();
                redraw_objects();
                display_picture();
                show_status("Press any key to continue...");
                wait_for_key();
                break;
            case 3:
                modify_object();
                redraw_objects();
                display_picture();
                show_status("Press any key to continue...");
                wait_for_key();
                break;
            case 4:
                list_objects();
                break;
            case 5:
                redraw_objects();
                display_picture();
                show_status("Press any key to continue...");
                wait_for_key();
                break;
            case 6:
                show_status("Exiting editor.");
                shutdown_ui();
                return 0;
            default:
                break;
        }
    }
}
// Compile with: gcc graphics_editor.c -o graphics_editor -lm