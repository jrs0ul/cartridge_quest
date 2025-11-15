#ifndef _TOUCH_DATA
#define _TOUCH_DATA

#include <vector>

struct TouchData {
    std::vector<Vector3D> up;
    std::vector<Vector3D> move;
    std::vector<Vector3D> down;
    std::vector<Vector3D> oldDown;
    
    bool allfingersup;
    
};



#endif //_TOUCH_DATA

