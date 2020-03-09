#include <memory>
#include <string>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "debug.h"
#include "interp.h"
#include "shape.h"

#include "util.h"

static unordered_map<string,void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

unordered_map<string,interpreter::interpreterfn>
interpreter::interp_map {
   {"define" , &interpreter::do_define },
   {"draw"   , &interpreter::do_draw   },
   {"border" , &interpreter::do_border },
   {"moveby" , &interpreter::do_moveby },
};

unordered_map<string,interpreter::factoryfn>
interpreter::factory_map {
   {"text"     , &interpreter::make_text     },
   {"ellipse"  , &interpreter::make_ellipse  },
   {"circle"   , &interpreter::make_circle   },
   {"polygon"  , &interpreter::make_polygon  },
   {"rectangle", &interpreter::make_rectangle},
   {"square"   , &interpreter::make_square   },
   {"triangle"   , &interpreter::make_triangle },
   {"diamond"   , &interpreter::make_diamond },
   {"equilateral", &interpreter::make_equilateral }
};

interpreter::shape_map interpreter::objmap;

interpreter::~interpreter() {
   for (const auto& itor: objmap) {
      cout << "objmap[" << itor.first << "] = "
           << *itor.second << endl;
   }
}

void interpreter::interpret (const parameters& params) {
   DEBUGF ('i', params);
   param begin = params.cbegin();
   string command = *begin;
   auto itor = interp_map.find (command);
   if (itor == interp_map.end()) throw runtime_error ("syntax error");
   interpreterfn func = itor->second;
   func (++begin, params.cend());
}

void interpreter::do_define (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string name = *begin;
   objmap.emplace (name, make_shape (++begin, end));
}


void interpreter::do_draw (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 4) throw runtime_error ("syntax error");
   string name = begin[1];
   shape_map::const_iterator itor = objmap.find (name);
   if (itor == objmap.end()) {
      throw runtime_error (name + ": no such shape");
   }
   rgbcolor color {begin[0]};
   vertex where {from_string<GLfloat> (begin[2]),
                 from_string<GLfloat> (begin[3])};
   window::push_back (object (itor->second, where, color));
}

void interpreter::do_border(param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 2) throw runtime_error ("syntax error");
   window::border_color = rgbcolor(begin[0]);
   int width = stoi(begin[1]);
   if(width<=0) throw runtime_error ("border: must be bigger than 0");
   window::border_width = width;

}

void interpreter::do_moveby (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 1) throw runtime_error ("syntax error");
   window::move_by = stoi(begin[0]);
}

shape_ptr interpreter::make_shape (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string type = *begin++;
   auto itor = factory_map.find(type);
   if (itor == factory_map.end()) {
      throw runtime_error (type + ": no such shape");
   }
   factoryfn func = itor->second;
   return func (begin, end);
}

shape_ptr interpreter::make_text (param begin, param end) {
   DEBUGF ('f', range (begin, end));

   interpreter::check_error(begin, end);
   auto fontIt = fontcode.find((begin[0]));
   if(fontIt==fontcode.end()) throw runtime_error ("invalid font: " + begin[0]);
   begin++;
   string name ="";
   while(begin!=end){
      name+=*(begin++);
      if(begin!=end) name +=" ";
   }

   return make_shared<text> (fontIt->second, name);
}

shape_ptr interpreter::make_ellipse (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   interpreter::check_error(begin, end, 2);
   return make_shared<ellipse> (GLfloat(stof(begin[0])), 
                                GLfloat(stof(begin[1])));
}

shape_ptr interpreter::make_circle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   interpreter::check_error(begin, end, 1);
   return make_shared<circle> (GLfloat(stof(begin[0])));
}

shape_ptr interpreter::make_polygon (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   vertex_list list;
   interpreter::check_error(begin, end);

   
   while(begin!=end){
      list.push_back({GLfloat(stof(begin[0])), 
         GLfloat(stof(begin[1]))});
      //increament by 2
      begin++;
      begin++;
   }
   return make_shared<polygon> (list);
}

shape_ptr interpreter::make_rectangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   interpreter::check_error(begin, end, 2);
   return make_shared<rectangle> (GLfloat(stof(begin[0])), 
                                  GLfloat(stof(begin[1])));
}

shape_ptr interpreter::make_square (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   interpreter::check_error(begin, end, 1);
   return make_shared<square> (GLfloat(stof(begin[0])));
}

shape_ptr interpreter::make_triangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   interpreter::check_error(begin, end, 6);
   vertex_list list;

   while(begin!=end){
      list.push_back({GLfloat(stof(begin[0])), 
         GLfloat(stof(begin[1]))});
      //increament by 2
      begin++;
      begin++;
   }
   return make_shared<triangle> (list);
}

shape_ptr interpreter::make_diamond (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   DEBUGF ('f', range (end, end));
   interpreter::check_error(begin, end, 2);
   return make_shared<diamond> (GLfloat(stof(begin[0])),
                                GLfloat(stof(begin[1])));
}

shape_ptr interpreter::make_equilateral (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   return make_shared<equilateral> (GLfloat(stof(begin[0])));
}

void interpreter::check_error(param begin, param end, int size){
   //size=-1 means no specified size required then even points, ie polygon
   if(begin == end) throw runtime_error ("usage: number of args");
   int count =end - begin;
   if (size==-1){
      if(count%2!=0) throw runtime_error ("usage: number of args");
   } 
   else if(count!=size) throw runtime_error 
      ("usage: number of args");
}

