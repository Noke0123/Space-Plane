
#define STB_IMAGE_IMPLEMENTATION
#include <windows.h>
#include <GL/glut.h>
#include "tutorial4.h"
#include "texture.h"
#include "3dsloader.h"
#include<cmath>
#include <array>
#include <cstdlib> // For rand()
#include "stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include<ctime>
#include<sstream>
#include<iostream>
using namespace std;
#include <vector>

float lastFrameTime = 0.0f;
time_t count_down;
clock_t my_clock;

 
int screen_width = 640;
int screen_height = 480;

int score=0;
int game_time = 40;
int eat_dog = 0;
int eat_rock = 0;
bool game = true;
float obj_size = 50;

//skybox

#define MAX_VERTICES 2000
#define MAX_POLYGONS 2000

typedef struct {
    float x, y, z;
} vertex;

typedef struct {
    int a, b, c, d;
} polygon;

typedef struct {
    float u, v;
} mapcoord;

typedef struct {
    vertex vertex[MAX_VERTICES];
    polygon polygon[MAX_POLYGONS];
    mapcoord mapcoord[MAX_VERTICES];
    int id_texture;
} sky_type, * sky_type_ptr;


float skybox_size = 5000;

sky_type skybox = {
    {
        {-skybox_size, -skybox_size, skybox_size}, {skybox_size, -skybox_size, skybox_size}, {skybox_size, -skybox_size, -skybox_size}, {-skybox_size, -skybox_size, -skybox_size},
        {-skybox_size, skybox_size, skybox_size}, {skybox_size, skybox_size, skybox_size}, {skybox_size, skybox_size, -skybox_size}, {-skybox_size, skybox_size, -skybox_size}
    },
    {
        {3, 0, 4, 7}, {0, 1, 5, 4}, {4, 5, 6, 7},
        {2, 3, 7, 6}, {1, 2, 6, 5}, {3, 2, 1, 0}
    },
    {
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0},
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0},
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0},
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0},
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0},
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}
    },
    0
};

int id_skybox_up, id_skybox_down, id_skybox_left, id_skybox_right, id_skybox_front, id_skybox_back;



void background()
{

    // 繪製Skybox
    glPushMatrix();


    int textures[] = { id_skybox_front, id_skybox_back, id_skybox_left, id_skybox_right, id_skybox_up, id_skybox_down };

    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);
    glEnable(GL_TEXTURE_2D);

    for (int face = 0; face < 6; ++face) {
        glBindTexture(GL_TEXTURE_2D, textures[face]);
        glBegin(GL_QUADS);
        for (int vertex_index = 0; vertex_index < 4; ++vertex_index) {
            glTexCoord2f(skybox.mapcoord[face * 4 + vertex_index].u,
                skybox.mapcoord[face * 4 + vertex_index].v);
            int polygon_vertex = ((int*)&skybox.polygon[face])[vertex_index];
            glVertex3f(skybox.vertex[polygon_vertex].x,
                skybox.vertex[polygon_vertex].y,
                skybox.vertex[polygon_vertex].z);
        }
        glEnd(); // 關閉單個四邊形的繪製
    }
    glPopAttrib();


    glPopMatrix();
}//skybox



//move_point

GLUquadric* obstacle = gluNewQuadric();


int LoadTexture(const char* filename, int width, int height)
{

    GLuint texture;
    unsigned char* data;
    FILE* file;

    //讀檔案
    file = fopen(filename, "rb");
    if (file == NULL) return 0;

    data = (unsigned char*)malloc(width * height * 3);
    fread(data, width * height * 3, 1, file);
    fclose(file);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


    //線性濾圖
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //生成紋理
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
    free(data); //釋放紋理
    return texture;

}//move_point


float rotation_x = 0;
float rotation_y = 0;
float rotation_z = 0;



//字串
#define MAX_CHAR       128
void drawString(const char* str) {

    static int isFirstCall = 1;
    static GLuint lists;

    if (isFirstCall) { // 如果是第一次調用，執行初始化

        // 爲每一個ASCII字符產生一個顯示列表

        isFirstCall = 0;

        // 申請MAX_CHAR個連續的顯示列表編號

        lists = glGenLists(MAX_CHAR);

        // 把每個字符的繪製命令都裝到對應的顯示列表中

        wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);

    }

    // 調用每個字符對應的顯示列表，繪製每個字符

    for (; *str != '\0'; ++str)
        glCallList(lists + *str);

}

void selectFont(int size, int charset, const char* face) {

    HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0,

        charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,

        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);

    HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);

    DeleteObject(hOldFont);

}





struct Point {
    float x, y, z;
};

Point d1 = { 0,0,0 }, d2 = { 0,0,0 }, d3 = { 0,0,0 }, d4 = { 0,0,0 }, d5 = { 0,0,0 }, d6 = { 0,0,0 }, d7 = { 0,0,0 };


Point normalize(Point look_dir) {
    // 計算向量的長度
    float length = std::sqrt(look_dir.x * look_dir.x + look_dir.y * look_dir.y + look_dir.z * look_dir.z);

    // 如果長度為零，避免除零錯誤，返回原向量
    if (length == 0.0f) {
        return look_dir;
    }

    // 將每個分量除以長度
    return { look_dir.x / length, look_dir.y / length, look_dir.z / length };
}



// 計算貝茲曲線的點
Point cubicBezier(float t, const Point& d1, const Point& d2, const Point& d3, const Point& d4) {
    float u = 1 - t; // u = 1-t
    float u2 = u * u;
    float u3 = u2 * u;
    float t2 = t * t;
    float t3 = t2 * t;

    Point temp;
    temp.x = u3 * d1.x + 3 * u2 * t * d2.x + 3 * u * t2 * d3.x + t3 * d4.x;
    temp.y = u3 * d1.y + 3 * u2 * t * d2.y + 3 * u * t2 * d3.y + t3 * d4.y;
    temp.z = u3 * d1.z + 3 * u2 * t * d2.z + 3 * u * t2 * d3.z + t3 * d4.z;

    return temp;
}

// 計算船的最終位置
float boat_world_x = 0;
float boat_world_y = 0;
float boat_world_z = 0;

float speed_z = 0, speed_y = 0, speed_x = 0;  //WASD控制的方向力度






float M_PI = 3.1415926;










float camera_target_x = 0, camera_target_y = 0, camera_target_z = 0;
float camera_follow_speed = 0.5; // 平滑跟隨速度

void smooth_follow(float target_x, float target_y, float target_z,
    float* camera_x, float* camera_y, float* camera_z,
    float speed) {
    *camera_x += (target_x - *camera_x) * speed;
    *camera_y += (target_y - *camera_y) * speed;
    *camera_z += (target_z - *camera_z) * speed;
}


// Flag for rendering as lines or filled polygons
int filling = 1; //0=OFF 1=ON

//Now the object is generic, the cube has annoyed us a little bit, or not?
obj_type object;


obj_type object2;

obj_type object3;








int LoadPNG(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4); // Force 4 channels (RGBA)

    if (!data) {
        printf("Failed to load PNG: %s\n", filename);
        return -1;
    }

    num_texture++; // Increment texture counter

    glBindTexture(GL_TEXTURE_2D, num_texture); // Bind texture ID

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data); // Free image memory

    return num_texture; // Return OpenGL texture ID
}







/**********************************************************
 *
 * SUBROUTINE resize(int,int)
 *
 * This routine must be called everytime we resize our window.
 *
 *********************************************************/

void resize(int width, int height)
{
    screen_width = width; // We obtain the new screen width values and store it
    screen_height = height; // Height value

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // We clear both the color and the depth buffer so to draw the next frame
    glViewport(0, 0, screen_width, screen_height); // Viewport transformation

    glMatrixMode(GL_PROJECTION); // Projection transformation
    glLoadIdentity(); // We initialize the projection matrix as identity
    gluPerspective(45.0f, (GLfloat)screen_width / (GLfloat)screen_height, 10.0f, 10000.0f);

    glutPostRedisplay(); // This command redraw the scene (it calls the same routine of glutDisplayFunc)
}


float speed_limit = 100;

float speed_a=3;


void keyboard(unsigned char key, int x, int y)
{
    if (game)
    {
        switch (key)
        {

        case 'W': case 'w':


            if (speed_z > speed_limit)speed_z = speed_limit;

            speed_z += speed_a;
            break;


        case 'S': case 's':

            speed_z -= speed_a;
            if (speed_z < -speed_limit)speed_z = -speed_limit;
            break;

        case 'A': case 'a':


            if (speed_x < -speed_limit)speed_x = -speed_limit;
            speed_x += speed_a;
            break;

        case 'D': case 'd':
            if (speed_x > speed_limit)speed_x = speed_limit;



            speed_x -= speed_a;
            break;

        case 'Z': case 'z':
            if (speed_y > speed_limit)speed_y = speed_limit;



            speed_y += speed_a;
            break;

        case 'X': case 'x':
            if (speed_y < -speed_limit)speed_y = -speed_limit;

            speed_y -= speed_a;
            break;


        }


    }

   
}

void build_boat()
{
    glPushMatrix();
    int l_index;
    glBindTexture(GL_TEXTURE_2D, object.id_texture); // We set the active texture 
   
    

    glRotatef(180, 0, 0, 1);
    glRotatef(180, 0, 1, 0);
    glRotatef(90, 1, 0, 0);

    glScalef(0.1f, 0.1f, 0.1f);
    glBegin(GL_TRIANGLES); // glBegin and glEnd delimit the vertices that define a primitive (in our case triangles)
    for (l_index = 0; l_index < object.polygons_qty; l_index++)
    {
        //----------------- FIRST VERTEX -----------------
        // Texture coordinates of the first vertex
        glTexCoord2f(object.mapcoord[object.polygon[l_index].a].u,
            object.mapcoord[object.polygon[l_index].a].v);
        // Coordinates of the first vertex
        glVertex3f(object.vertex[object.polygon[l_index].a].x,
            object.vertex[object.polygon[l_index].a].y,
            object.vertex[object.polygon[l_index].a].z); //Vertex definition

        //----------------- SECOND VERTEX -----------------
        // Texture coordinates of the second vertex
        glTexCoord2f(object.mapcoord[object.polygon[l_index].b].u,
            object.mapcoord[object.polygon[l_index].b].v);
        // Coordinates of the second vertex
        glVertex3f(object.vertex[object.polygon[l_index].b].x,
            object.vertex[object.polygon[l_index].b].y,
            object.vertex[object.polygon[l_index].b].z);

        //----------------- THIRD VERTEX -----------------
        // Texture coordinates of the third vertex
        glTexCoord2f(object.mapcoord[object.polygon[l_index].c].u,
            object.mapcoord[object.polygon[l_index].c].v);
        // Coordinates of the Third vertex
        glVertex3f(object.vertex[object.polygon[l_index].c].x,
            object.vertex[object.polygon[l_index].c].y,
            object.vertex[object.polygon[l_index].c].z);
    }
    glEnd();

    glPopMatrix();
}



void build_dog()
{
    glPushMatrix();
    int l_index;

    

    glBindTexture(GL_TEXTURE_2D, object2.id_texture); // We set the active texture 

    glRotatef(180, 0, 1, 0); // Rotate the tower
    glRotatef(-90, 1, 0, 0); // Rotate the tower
    
    glScalef(0.1,0.1, 0.1); // Scale the tower
    glBegin(GL_TRIANGLES); // glBegin and glEnd delimit the vertices that define a primitive (in our case triangles)

    for (l_index = 0; l_index < object2.polygons_qty; l_index++)
    {
        //----------------- FIRST VERTEX -----------------
        // Texture coordinates of the first vertex
        glTexCoord2f(object2.mapcoord[object2.polygon[l_index].a].u,
            object2.mapcoord[object2.polygon[l_index].a].v);
        // Coordinates of the first vertex
        glVertex3f(object2.vertex[object2.polygon[l_index].a].x,
            object2.vertex[object2.polygon[l_index].a].y,
            object2.vertex[object2.polygon[l_index].a].z); //Vertex definition

        //----------------- SECOND VERTEX -----------------
        // Texture coordinates of the second vertex
        glTexCoord2f(object2.mapcoord[object2.polygon[l_index].b].u,
            object2.mapcoord[object2.polygon[l_index].b].v);
        // Coordinates of the second vertex
        glVertex3f(object2.vertex[object2.polygon[l_index].b].x,
            object2.vertex[object2.polygon[l_index].b].y,
            object2.vertex[object2.polygon[l_index].b].z);

        //----------------- THIRD VERTEX -----------------
        // Texture coordinates of the third vertex
        glTexCoord2f(object2.mapcoord[object2.polygon[l_index].c].u,
            object2.mapcoord[object2.polygon[l_index].c].v);
        // Coordinates of the Third vertex
        glVertex3f(object2.vertex[object2.polygon[l_index].c].x,
            object2.vertex[object2.polygon[l_index].c].y,
            object2.vertex[object2.polygon[l_index].c].z);
    }
    glEnd();
    glPopMatrix();
}




void build_rock()
{
    glPushMatrix();
    int l_index;
    glBindTexture(GL_TEXTURE_2D, object3.id_texture); // We set the active texture 


    glRotatef(90, 0, 1, 0);
    glScalef(5, 5, 2.5); // Scale the tower
    glBegin(GL_TRIANGLES); // glBegin and glEnd delimit the vertices that define a primitive (in our case triangles)

    for (l_index = 0; l_index < object3.polygons_qty; l_index++)
    {
        //----------------- FIRST VERTEX -----------------
        // Texture coordinates of the first vertex
        glTexCoord2f(object3.mapcoord[object3.polygon[l_index].a].u,
            object3.mapcoord[object3.polygon[l_index].a].v);
        // Coordinates of the first vertex
        glVertex3f(object3.vertex[object3.polygon[l_index].a].x,
            object3.vertex[object3.polygon[l_index].a].y,
            object3.vertex[object3.polygon[l_index].a].z); //Vertex definition

        //----------------- SECOND VERTEX -----------------
        // Texture coordinates of the second vertex
        glTexCoord2f(object3.mapcoord[object3.polygon[l_index].b].u,
            object3.mapcoord[object3.polygon[l_index].b].v);
        // Coordinates of the second vertex
        glVertex3f(object3.vertex[object3.polygon[l_index].b].x,
            object3.vertex[object3.polygon[l_index].b].y,
            object3.vertex[object3.polygon[l_index].b].z);

        //----------------- THIRD VERTEX -----------------
        // Texture coordinates of the third vertex
        glTexCoord2f(object3.mapcoord[object3.polygon[l_index].c].u,
            object3.mapcoord[object3.polygon[l_index].c].v);
        // Coordinates of the Third vertex
        glVertex3f(object3.vertex[object3.polygon[l_index].c].x,
            object3.vertex[object3.polygon[l_index].c].y,
            object3.vertex[object3.polygon[l_index].c].z);
    }
    glEnd();
    glPopMatrix();
}



struct obs
{
    float x, y, z;

    float speed_x, speed_y, speed_z;


    int object_num;



};


std::vector<obs> objects;

int object_speed_limit = 1;

void generateRandomObject() {

    obs temp;
    float distance;
    do {
        temp.x = boat_world_x + (rand() % 600 - 300); // 隨機生成範圍 [-150, 150]
        temp.y = boat_world_y + (rand() % 300 - 150);
        temp.z = boat_world_z + (rand() % 1000 - 300);

        // 計算與船的距離
        distance = sqrt(pow(temp.x - boat_world_x, 2) +
            pow(temp.y - boat_world_y, 2) +
            pow(temp.z - boat_world_z, 2));

    } while (distance < 400 || distance > 1200); // 檢查距離是否符合條件

    temp.object_num = rand() % 2;

    temp.speed_x =  (rand() % 3-1); // 隨機生成範圍 [-15, 15]
    temp.speed_y =  (rand() % 3-1);
    temp.speed_z =  (rand() % 3-1);

    objects.push_back(temp);
}

void objectMove()
{
    for (auto& obj : objects) {
        obj.x += obj.speed_x;
        obj.y += obj.speed_y;
        obj.z += obj.speed_z;
    }
}

// 移除與船距離超過10500的物件
void removeDistantObjects() {
    objects.erase(
        std::remove_if(objects.begin(), objects.end(), [](const obs& obj) {
            float distance = sqrt(pow(obj.x - boat_world_x, 2) +
            pow(obj.y - boat_world_y, 2) +
                pow(obj.z - boat_world_z, 2));
    return distance > 10500;
            }),
        objects.end());
}

void objectShow()
{
    for (auto& obj : objects) {
        glPushMatrix();
        glTranslatef(obj.x, obj.y, obj.z);
        glScalef(obj_size/10, obj_size/10, obj_size/10);
        if (obj.object_num == 0)
        {
            build_dog();
        }
        else
        {
            build_rock();
        }
        glPopMatrix();
    }
}
    
void collision()
{

    for (auto it = objects.begin(); it != objects.end(); ) {
        float distance = std::sqrt(std::pow(it->x - boat_world_x, 2) + std::pow(it->y - boat_world_y, 2)+ pow(it->z - boat_world_z, 2));
        if (distance < 75) {
            /*
            speed_x = boat_world_x - it->x;
            speed_y = boat_world_y - it->y;
            speed_z = boat_world_z - it->z;

            d1.x = boat_world_x + speed_x;
            d1.y = boat_world_y + speed_y;
            d1.z = boat_world_z + speed_z;

            d2.x = boat_world_x + speed_x;
            d2.y = boat_world_y + speed_y;
            d2.z = boat_world_z + speed_z;

            d3.x = boat_world_x + speed_x;
            d3.y = boat_world_y + speed_y;
            d3.z = boat_world_z + speed_z;

            d4.x = boat_world_x + speed_x;
            d4.y = boat_world_y + speed_y;
            d4.z = boat_world_z + speed_z;
            */
            if (it->object_num == 0)
            {
                eat_dog++;
            }
            else
            {
                eat_rock++;
            }
            it = objects.erase(it); // 移除後返回新的有效迭代器
            //吃狗加分 吃石頭減分
            
        }
        else {

            if (it->x > 5000)
            {
                it->x = 4990;
                it->speed_x *= -1;

            }

            if (it->x < -5000)
            {
                it->x = -4990;
                it->speed_x *= -1;

            }

            if (it->y > 5000)
            {
                it->y = 4990;
                it->speed_y *= -1;

            }

            if (it->y < -5000)
            {
                it->y = -4990;
                it->speed_y *= -1;

            }

            if (it->z > 5000)
            {
                it->z = 4990;
                it->speed_z *= -1;

            }

            if (it->z < -5000)
            {
                it->z = -4990;
                it->speed_z *= -1;

            }



            ++it; // 移動到下一個元素

        }
    }



    //飛船碰撞skybox
    if (boat_world_x > 5000)
    {
        boat_world_x = 4990;
        speed_x *=-1;
        d1.x = boat_world_x + speed_x;
        d1.y = boat_world_y + speed_y;
        d1.z = boat_world_z + speed_z;

        d2.x = boat_world_x + speed_x;
        d2.y = boat_world_y + speed_y;
        d2.z = boat_world_z + speed_z;

        d3.x = boat_world_x + speed_x;
        d3.y = boat_world_y + speed_y;
        d3.z = boat_world_z + speed_z;

        d4.x = boat_world_x + speed_x;
        d4.y = boat_world_y + speed_y;
        d4.z = boat_world_z + speed_z;
    }

    if (boat_world_x < -5000)
    {
        boat_world_x = -4990;
        speed_x *= -1;
        d1.x = boat_world_x + speed_x;
        d1.y = boat_world_y + speed_y;
        d1.z = boat_world_z + speed_z;

        d2.x = boat_world_x + speed_x;
        d2.y = boat_world_y + speed_y;
        d2.z = boat_world_z + speed_z;

        d3.x = boat_world_x + speed_x;
        d3.y = boat_world_y + speed_y;
        d3.z = boat_world_z + speed_z;

        d4.x = boat_world_x + speed_x;
        d4.y = boat_world_y + speed_y;
        d4.z = boat_world_z + speed_z;
    }
    //x
    //y

    if (boat_world_y > 5000)
    {
        boat_world_y = 4990;
        speed_y *= -1;
        d1.x = boat_world_x + speed_x;
        d1.y = boat_world_y + speed_y;
        d1.z = boat_world_z + speed_z;

        d2.x = boat_world_x + speed_x;
        d2.y = boat_world_y + speed_y;
        d2.z = boat_world_z + speed_z;

        d3.x = boat_world_x + speed_x;
        d3.y = boat_world_y + speed_y;
        d3.z = boat_world_z + speed_z;

        d4.x = boat_world_x + speed_x;
        d4.y = boat_world_y + speed_y;
        d4.z = boat_world_z + speed_z;
    }
    if (boat_world_y < -5000)
    {
        boat_world_y = -4990;
        speed_y *= -1;
        d1.x = boat_world_x + speed_x;
        d1.y = boat_world_y + speed_y;
        d1.z = boat_world_z + speed_z;

        d2.x = boat_world_x + speed_x;
        d2.y = boat_world_y + speed_y;
        d2.z = boat_world_z + speed_z;

        d3.x = boat_world_x + speed_x;
        d3.y = boat_world_y + speed_y;
        d3.z = boat_world_z + speed_z;

        d4.x = boat_world_x + speed_x;
        d4.y = boat_world_y + speed_y;
        d4.z = boat_world_z + speed_z;
    }

    //y
    //z
    if (boat_world_z > 5000)
    {
        boat_world_z = 4990;
        speed_z *= -1;
        d1.x = boat_world_x + speed_x;
        d1.y = boat_world_y + speed_y;
        d1.z = boat_world_z + speed_z;

        d2.x = boat_world_x + speed_x;
        d2.y = boat_world_y + speed_y;
        d2.z = boat_world_z + speed_z;

        d3.x = boat_world_x + speed_x;
        d3.y = boat_world_y + speed_y;
        d3.z = boat_world_z + speed_z;

        d4.x = boat_world_x + speed_x;
        d4.y = boat_world_y + speed_y;
        d4.z = boat_world_z + speed_z;
    }
    if (boat_world_z < -5000)
    {
        boat_world_z = -4990;
        speed_z *= -1;
        d1.x = boat_world_x + speed_x;
        d1.y = boat_world_y + speed_y;
        d1.z = boat_world_z + speed_z;

        d2.x = boat_world_x + speed_x;
        d2.y = boat_world_y + speed_y;
        d2.z = boat_world_z + speed_z;

        d3.x = boat_world_x + speed_x;
        d3.y = boat_world_y + speed_y;
        d3.z = boat_world_z + speed_z;

        d4.x = boat_world_x + speed_x;
        d4.y = boat_world_y + speed_y;
        d4.z = boat_world_z + speed_z;
    }//z

    //飛船碰撞skybox




}



// 控制變數
float t = 0.0f;

const float speed = 0.01f; // 更新速度

// 使用布林標誌來確保每個項目只觸發一次
bool triggered_33 = false;
bool triggered_66 = false;
bool triggered_100 = false;


void show_move_point(Point point,int texture)
{
    float distance;
    distance = sqrt(pow(point.x - boat_world_x, 2) +
        pow(point.y - boat_world_y, 2) +
        pow(point.z - boat_world_z, 2));


    //物件飛船碰撞
    if (distance > 10)
    {
        glPushMatrix();
        glBindTexture(GL_TEXTURE_2D, texture); // We set the active texture 


        glTranslatef(point.x, point.y, point.z);
        gluSphere(obstacle, 5, 20, 20);
        glPopMatrix();
    }

    
}

Point move_point = {0,0,0};

void boatMove() {
    t += speed; // Update time

    // Update d5 when t > 0.33f
    if (!triggered_33 && t > 0.33f) {
        d5 = { d4.x + speed_x, d4.y + speed_y, d4.z + speed_z };
        triggered_33 = true;
    }

    // Update d6 when t > 0.66f
    if (!triggered_66 && t > 0.66f) {
        d6 = { d5.x + speed_x, d5.y + speed_y, d5.z + speed_z };
        triggered_66 = true;
    }

    // Update move_point and display relevant points
    if (triggered_66) {
        move_point = { d6.x + speed_x, d6.y + speed_y, d6.z + speed_z };
        show_move_point(d4, 9);
        show_move_point(d5, 9);
        show_move_point(d6, 9);
        show_move_point(move_point, 10);
    }
    else if (triggered_33) {
        move_point = { d5.x + speed_x, d5.y + speed_y, d5.z + speed_z };
        show_move_point(d3, 9);
        show_move_point(d4, 9);
        show_move_point(d5, 9);
        show_move_point(move_point, 10);
    }
    else {
        show_move_point(d2, 9);
        show_move_point(d3, 9);
        show_move_point(d4, 9);
        move_point = { d4.x + speed_x, d4.y + speed_y, d4.z + speed_z };
        show_move_point(move_point, 10);
    }

    // Reset and update coordinates when t > 1.0f
    if (!triggered_100 && t > 1.0f) {
        d7 = { d6.x + speed_x, d6.y + speed_y, d6.z + speed_z };

        // Shift coordinates
        d1 = d4;
        d2 = d5;
        d3 = d6;
        d4 = d7;

        // Reset time and triggers
        t = 0.0f;
        triggered_33 = triggered_66 = triggered_100 = false;
    }
}







void display(void)
{
    int l_index;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // This clear the background color to dark blue
    glMatrixMode(GL_MODELVIEW); // Modeling transformation
    
    background();



    // 迴圈計算曲線上的點

    

    if (game) //遊戲數值運算  不包含顯示畫面
    {
        
        if (clock() - my_clock >  1000)
        {
            generateRandomObject();
            my_clock = clock();
        }
        
      
        Point temp = cubicBezier(t, d1, d2, d3, d4);

        boat_world_x = temp.x;
        boat_world_y = temp.y;
        boat_world_z = temp.z;

        boatMove();


        objectMove();
        removeDistantObjects();
        collision();


    }


    Point look_dir = { move_point.x - boat_world_x , move_point.y - boat_world_y, move_point.z - boat_world_z };
    Point nor_look_dir = normalize(look_dir);


    glTranslatef(boat_world_x+10, boat_world_y+10, boat_world_z  -1);

    selectFont(48, ANSI_CHARSET, "Comic Sans MS");

    //glClear(GL_COLOR_BUFFER_BIT);
   // glColor3f(1.0f, 0.0f, 0.0f);
    glRasterPos2f(-3.0f, 7.0f);
    glBindTexture(GL_TEXTURE_2D, 10);
    stringstream ss;
    stringstream sss;
    stringstream ssss;
    string temp, dog, rock,sc;
    string s = "Time:";

    if (game)
    {


        if (count_down - time(NULL) == 0)
        {
            game = false;
        }


        ss << (count_down - time(NULL));
        ss >> temp;
        s += temp + "s";

        drawString(s.c_str());


    }
    else
    {
       
        speed_x = 0;
        speed_y = 0;
        speed_z = 0;


        selectFont(130, ANSI_CHARSET, "Comic Sans MS");

        score = eat_dog - eat_rock;
        glRasterPos2f(-3.0f, 0.0f);
        ss << eat_dog;
        ss >> dog;
        sss << eat_rock;
        sss >> rock;
        ssss << score;
        ssss >> sc;
        s = "Dog:" + dog + " Rock:" + rock + " Score:"+sc;
        drawString(s.c_str());



    }


    //結束遊戲  依然繪製畫面

    glLoadIdentity(); // Initialize the model matrix as identity

    smooth_follow(boat_world_x, boat_world_y, boat_world_z,
        &camera_target_x, &camera_target_y, &camera_target_z, camera_follow_speed);


    
    // 設置 gluLookAt
    gluLookAt(
        camera_target_x- speed_x, camera_target_y - speed_y+10, camera_target_z- speed_z-50,     // 相機位置
        boat_world_x+speed_x, boat_world_y+speed_y, boat_world_z+speed_z, // 目標（船的位置）
        0, 1, 0                          // 向上的方向
    );

    objectShow();

    // 在世界座標系下繪製船
    glPushMatrix();
    glTranslatef(boat_world_x, boat_world_y, boat_world_z);
    if (nor_look_dir.z < 0)
    {
        glRotatef(180, 0, 1, 0);
        glRotatef(nor_look_dir.x * -90, 0, 1, 0);
        glRotatef(nor_look_dir.y * -90, 1, 0, 0);
    }
    else
    {
        glRotatef(nor_look_dir.x * 90, 0, 1, 0);
        glRotatef(nor_look_dir.y * -90, 1, 0, 0);
    }
    
    
    build_boat();
    glPopMatrix();


    

    



    glFlush(); // This force the execution of OpenGL commands
    glutSwapBuffers(); // In double buffered mode we invert the positions of the visible buffer and the writing buffer
}


int texture = 0;



void init(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0); // This clear the background color to black
    glShadeModel(GL_SMOOTH); // Type of shading for the polygons

    // Viewport transformation
    glViewport(0, 0, screen_width, screen_height);

    // Projection transformation
    glMatrixMode(GL_PROJECTION); // Specifies which matrix stack is the target for matrix operations 
    glLoadIdentity(); // We initialize the projection matrix as identity
    gluPerspective(45.0f, (GLfloat)screen_width / (GLfloat)screen_height, 10.0f, 10000.0f); // We define the "viewing volume"

    glEnable(GL_DEPTH_TEST); // We enable the depth test (also called z buffer)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Polygon rasterization mode (polygon filled)

    glEnable(GL_TEXTURE_2D); // This Enable the Texture mapping

    Load3DS(&object, "spaceship.3ds");
    Load3DS(&object2, "dog.3ds");
    Load3DS(&object3, "rock.3ds");



    object.id_texture = LoadBitmap("spaceshiptexture.bmp"); // The Function LoadBitmap() return the current texture ID
    object2.id_texture = LoadPNG("dog.png");
    object3.id_texture = LoadPNG("rock.png");
    // If the last function returns -1 it means the file was not found so we exit from the program
    if (object.id_texture == -1)
    {
        MessageBox(NULL, "Image file: spaceshiptexture.bmp not found", "Zetadeck", MB_OK | MB_ICONERROR);
        exit(0);
    }
    if (object2.id_texture == -1)
    {
        MessageBox(NULL, "Image file: dog.bmp not found", "Zetadeck", MB_OK | MB_ICONERROR);
        exit(0);
    }

    if (object3.id_texture == -1)
    {
        MessageBox(NULL, "Image file: rock.bmp not found", "Zetadeck", MB_OK | MB_ICONERROR);
        exit(0);
    }

    my_clock = clock();


    id_skybox_up = LoadBitmap("skybox_up.bmp");
    id_skybox_down = LoadBitmap("skybox_down.bmp");
    id_skybox_left = LoadBitmap("skybox_left.bmp");
    id_skybox_right = LoadBitmap("skybox_right.bmp");

    id_skybox_front = LoadBitmap("skybox_back.bmp");

    id_skybox_back = LoadBitmap("skybox_back.bmp");

    if (id_skybox_up == -1 || id_skybox_down == -1 || id_skybox_left == -1 ||
        id_skybox_right == -1 || id_skybox_front == -1 || id_skybox_back == -1) {
        MessageBox(NULL, "Skybox texture file not found", "Error", MB_OK | MB_ICONERROR);
        exit(0);
    }


    texture = LoadTexture("move_point.bmp", 328, 154);
    texture = LoadTexture("move_point_new.bmp", 800, 450);
    gluQuadricTexture(obstacle, GL_TRUE);



    srand(time(NULL));
    t = clock();
    count_down = time(NULL) + game_time;
}



void update(int value) {

    //generateRandomObject(boat_world_x, boat_world_y, boat_world_z);

    glutPostRedisplay(); // 觸發重新繪製
    glutTimerFunc(4, update, 0); // 每 16 毫秒調用一次更新（約 60 FPS）
}


int main(int argc, char** argv)
{
    // We use the GLUT utility to initialize the window, to handle the input and to interact with the windows system
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screen_width, screen_height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("www.spacesimulator.net - 3d engine tutorials: Tutorial 4");
    glutDisplayFunc(display);
    glutTimerFunc(16, update, 0); // 開始計時器（16 毫秒）

    for (int i = 0; i < 15; i++)
    {
        generateRandomObject();
    }
    obs temp;
    temp.x = 0;
    temp.y = 0;
    temp.z = 100;
    temp.speed_x = 0;
    temp.speed_y = 0;
    temp.speed_z = 0;
    temp.object_num = 0;
    objects.push_back(temp);
    

    obs temp1;
    temp1.x = 60;
    temp1.y = 0;
    temp1.z = 100;
    temp1.speed_x = 0;
    temp1.speed_y = 0;
    temp1.speed_z = 0;
    temp1.object_num = 1;
    objects.push_back(temp1);


   

    glutReshapeFunc(resize);
    glutKeyboardFunc(keyboard);

    init();
    glutMainLoop();

    return(0);
}
