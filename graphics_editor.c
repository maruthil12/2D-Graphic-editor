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
        printf("  %s [%d‥%d]: ",prompt,lo,hi);
        if(scanf("%d",&v)==1&&v>=lo&&v<=hi)
            return v;
        printf("    *** Out of range or invalid — try again.\n");
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

int main(void){
    int sample;
    int choice;
    int abs_value=absoluteValue(-7);
    int sign_neg=getSign(-7);
    int sign_zero=getSign(0);
    int sign_pos=getSign(13);

    for(int r=0;r<ROWS;++r)
    {
        for(int c=0;c<COLS;++c)
        {
            canvas[r][c]=BG_CHAR;
        }
    }

    printf("absoluteValue(-7) = %d\n",abs_value);
    printf("getSign(-7) = %d, getSign(0) = %d, getSign(13) = %d\n",
           sign_neg,sign_zero,sign_pos);

    printf("Choose a shape to draw:\n");
    printf("  1) %s\n",shape_name(LINE));
    printf("  2) %s\n",shape_name(RECTANGLE));
    printf("  3) %s\n",shape_name(CIRCLE));
    printf("  4) %s\n",shape_name(TRIANGLE));

    choice=read_int("Shape number",1,4);
    printf("Selected shape: %s\n",shape_name(choice));

    plot(-1,-1);
    raster_line(ROWS,COLS,ROWS+1,COLS+1);
    raster_rectangle(ROWS,COLS,ROWS+2,COLS+2);
    raster_circle(ROWS+10,COLS+10,3);
    raster_circle_points(ROWS+10,COLS+10,1,2);
    raster_triangle(ROWS,COLS,ROWS+1,COLS+1,ROWS+2,COLS+2);

    switch(choice){
        case LINE:
            raster_line(2,2,20,40);
            break;
        case RECTANGLE:
            raster_rectangle(4,8,15,45);
            break;
        case CIRCLE:
            raster_circle(12,30,8);
            break;
        case TRIANGLE:
            /* draw a symmetric triangle centered horizontally */
            raster_triangle(5, 30, 20, 10, 20, 50);
            break;
        default:
            break;
    }

    printf("Canvas after drawing the selected shape:\n");

    for(int r=0;r<ROWS;++r){
        for(int c=0;c<COLS;++c)
            putchar(canvas[r][c]);
        putchar('\n');
    }

    sample=read_int("Enter a sample value",1,10);
    printf("You entered: %d\n",sample);
    return 0;
}
