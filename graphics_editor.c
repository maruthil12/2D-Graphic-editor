#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ROWS 25
#define COLS 60
#define BG_CHAR '_'
#define DRAW_CHAR '*'
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
    int dr=absoluteValue(r1-r0),sr=getSign(r1-r0);
    int dc=absoluteValue(c1-c0),sc=getSign(c1-c0);
    int err=dr-dc;
    for(;;){
        plot(r0,c0);
        if(r0==r1&&c0==c1)
        break;
        int e2=2*err;
        if(e2>-dc){
        err-=dc;
        r0+=sr;
        }
        if(e2<dr){
        err+=dr;
        c0+=sc;
        }
    }
}

static void raster_circle_points(int cr,int cc,int x,int y){
    if(x==0){
        plot(cr,cc+y);
        plot(cr,cc-y);
        plot(cr+y,cc);
        plot(cr-y,cc);
    }
    else if(x==y){
        plot(cr+x,cc+y);
        plot(cr-x,cc+y);
        plot(cr+x,cc-y);
        plot(cr-x,cc-y);
    }
    else{
        plot(cr+x,cc+y);plot(cr-x,cc+y);
        plot(cr+x,cc-y);plot(cr-x,cc-y);
        plot(cr+y,cc+x);plot(cr-y,cc+x);
        plot(cr+y,cc-x);plot(cr-y,cc-x);
    }
}

static void raster_circle(int cr,int cc,int radius){
    int x=0,y=radius,d=1-radius;
    raster_circle_points(cr,cc,x,y);
    while(x<=y){
        x++;
        if(d<0){
            d=d+4*x+6;
        }
        else{
            d=d+4*(x-y)+10;
            y--;
        }
        raster_circle_points(cr,cc,x,y);
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

static void show_canvas(void){
    for(int r=0;r<ROWS;++r){
        for(int c=0;c<COLS;++c)
            putchar(canvas[r][c]);
        putchar('\n');
    }
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

static void add_object(void){
    if(object_count>=MAX_OBJECTS){
        printf("*** Cannot add more than %d objects.\n",MAX_OBJECTS);
        return;
    }
    ShapeType type=(ShapeType)read_int("Shape number",1,4);
    Object obj;
    obj.id=next_id++;
    obj.type=type;
    switch(type){
        case LINE:
            obj.params.line.r0=read_int("Start row",0,ROWS-1);
            obj.params.line.c0=read_int("Start column",0,COLS-1);
            obj.params.line.r1=read_int("End row",0,ROWS-1);
            obj.params.line.c1=read_int("End column",0,COLS-1);
            break;
        case RECTANGLE:
            obj.params.rect.r0=read_int("First corner row",0,ROWS-1);
            obj.params.rect.c0=read_int("First corner column",0,COLS-1);
            obj.params.rect.r1=read_int("Second corner row",0,ROWS-1);
            obj.params.rect.c1=read_int("Second corner column",0,COLS-1);
            break;
        case CIRCLE:
            obj.params.circle.cr=read_int("Center row",0,ROWS-1);
            obj.params.circle.cc=read_int("Center column",0,COLS-1);
            obj.params.circle.radius=read_int("Radius",1,ROWS<COLS?ROWS:COLS);
            break;
        case TRIANGLE:
            obj.params.triangle.r0=read_int("First vertex row",0,ROWS-1);
            obj.params.triangle.c0=read_int("First vertex column",0,COLS-1);
            obj.params.triangle.r1=read_int("Second vertex row",0,ROWS-1);
            obj.params.triangle.c1=read_int("Second vertex column",0,COLS-1);
            obj.params.triangle.r2=read_int("Third vertex row",0,ROWS-1);
            obj.params.triangle.c2=read_int("Third vertex column",0,COLS-1);
            break;
        default:
            break;
    }
    objects[object_count++]=obj;
    printf("Added %s with id %d.\n",shape_name(type),obj.id);
}

static void delete_object(void){
    if(object_count==0){
        printf("No objects to delete.\n");
        return;
    }
    int id=read_int("Object id to delete",1,next_id-1);
    int index=find_object_index(id);
    if(index<0){
        printf("*** No object with id %d.\n",id);
        return;
    }
    for(int i=index;i<object_count-1;++i)
        objects[i]=objects[i+1];
    object_count--;
    printf("Deleted object %d.\n",id);
}

int main(void){
    int choice;
    clear_canvas();

    for(;;){
        printf("\n2D Graphic Editor Menu:\n");
        printf("  1) Add object\n");
        printf("  2) Delete object\n");
        printf("  3) List objects\n");
        printf("  4) Render canvas\n");
        printf("  5) Quit\n");

        choice=read_int("Menu choice",1,5);
        switch(choice){
            case 1:
                add_object();
                redraw_objects();
                show_canvas();
                break;
            case 2:
                delete_object();
                redraw_objects();
                show_canvas();
                break;
            case 3:
                list_objects();
                break;
            case 4:
                redraw_objects();
                show_canvas();
                break;
            case 5:
                printf("Exiting editor.\n");
                return 0;
            default:
                break;
        }
    }
}
