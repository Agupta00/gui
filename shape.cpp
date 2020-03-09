#include <typeinfo>
#include <unordered_map>
#include <cmath>

using namespace std;

#include "shape.h"
#include "graphics.h"
#include "util.h"

static unordered_map<void*,string> fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

static unordered_map<string,void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (void* glut_bitmap_font_, const string& textdata_):
      glut_bitmap_font(glut_bitmap_font_), textdata(textdata_) {
   DEBUGF ('c', this);
}

ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}


polygon::polygon (const vertex_list& vertices_): vertices(vertices_) {
   DEBUGF ('c', this);
}

rectangle::rectangle (GLfloat width, GLfloat height):
    polygon({{width, height}, {width, 0}, {0,0}, {0, height}}){
    DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
}

triangle::triangle (const vertex_list& vertices_): polygon(vertices_) {
   DEBUGF ('c', this);
}

equilateral::equilateral(const GLfloat width): triangle(
    {{0,0}, {width / 2, width}, {width,0}}) {
    DEBUGF ('c', this);
}

diamond::diamond(const GLfloat width, const GLfloat height):
    polygon({{0,0}, {width/2, height/2}, 
            {width, 0}, {width/2, 0-height/2}}) {
    DEBUGF ('c', this);
}

void text::draw (const vertex& center, const rgbcolor& color, int i) const {
   const GLubyte* name = reinterpret_cast<const GLubyte*> (textdata.c_str());
   glColor3ubv (color.ubvec);
   glRasterPos2f (center.xpos, center.ypos);
   glutBitmapString (glut_bitmap_font, name);

   DEBUGF ('d', this << "(" << center << "," << color << ")");
}

void text::draw_border (const vertex& center, const rgbcolor& color) const {
  const GLubyte* name = reinterpret_cast<const GLubyte*> (textdata.c_str());
  int length = glutBitmapLength (glut_bitmap_font, name);
  int height = glutBitmapHeight (glut_bitmap_font);

   glColor3ubv(window::border_color.ubvec);
   glLineWidth(window::border_width);
   glBegin(GL_LINE_LOOP);

   int width= window::border_width;
   length+=width;
   height+=width;

   int padding = 5;
   glVertex2f(center.xpos-padding, center.ypos-padding);
   glVertex2f(center.xpos+length+padding, center.ypos-padding);
   glVertex2f(center.xpos+length+padding, center.ypos+height/2+padding);
   glVertex2f(center.xpos-padding, center.ypos+height/2+padding);  
   glEnd();

}

void ellipse::draw (const vertex& center, const rgbcolor& color, int i) const {

   const GLfloat delta = 2 * M_PI / 64;


   shape::draw_number(center, color, i);
   glBegin(GL_POLYGON);
   glEnable (GL_LINE_SMOOTH);
   glColor3ubv(color.ubvec);
   for (GLfloat point = 0; point < 2 * M_PI; point += delta) {
      glVertex2f (dimension.xpos * cos(point) + center.xpos,
                  dimension.ypos * sin(point) + center.ypos);
   }
   glEnd();
   DEBUGF ('d', this << "(" << center << "," << color << ")");

}

void ellipse::draw_border(const vertex& center, const rgbcolor& color) const {
   
   const GLfloat delta = 2 * M_PI / 64;

   glLineWidth(window::border_width);
   glBegin(GL_LINE_LOOP);
   glEnable(GL_LINE_SMOOTH);
   glColor3ubv(window::border_color.ubvec);


   for (GLfloat point = 0; point < 2 * M_PI; point += delta) {
      glVertex2f (dimension.xpos * cos(point) + center.xpos,
                  dimension.ypos * sin(point) + center.ypos);
   }
   glEnd();
}

void polygon::draw (const vertex& center, const rgbcolor& color, int i) const {

  //center of polygon
  GLfloat x;
  GLfloat y;
  for(auto vertex: vertices) {
    x+=vertex.xpos;
    y+=vertex.ypos;
  }
  vertex cm{center.xpos + x/(vertices.size()),center.ypos + y/(vertices.size())};


   shape::draw_number(cm, color, i);
   glBegin (GL_POLYGON);
   glColor3ubv (color.ubvec);
   for(auto vertex: vertices) {
       glVertex2f (vertex.xpos + center.xpos,
                   vertex.ypos + center.ypos);
   }
   glEnd();
   DEBUGF ('d', this << "(" << center << "," << color << ")");
}

void polygon::draw_border (const vertex& center, const rgbcolor& color) const {
  glLineWidth(window::border_width);
  glBegin(GL_LINE_LOOP);
  glEnable(GL_LINE_SMOOTH);
  glColor3ubv(window::border_color.ubvec);

  for(auto vertex: vertices) {
       glVertex2f (vertex.xpos + center.xpos,
                   vertex.ypos + center.ypos);
   }
   glEnd();
}

void shape::draw_number(const vertex& center, const rgbcolor& color, int i) {
  const GLubyte* num = reinterpret_cast<const GLubyte*> (to_string(i).c_str());


  rgbcolor inverted = rgbcolor(255.0-color.ubvec[0],255.0-color.ubvec[1],255.0-color.ubvec[2]);
  glColor3ubv (inverted.ubvec);
  glRasterPos2f (center.xpos, center.ypos);
  glutBitmapString (GLUT_BITMAP_HELVETICA_18, num);
}
void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}

