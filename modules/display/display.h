#ifndef DISPLAY_H
#define DISPLAY_H

#include "modulewidget.h"

class Display : public ModuleWidget
{
    Q_OBJECT

public:
    explicit Display(FrameProxyInterface *frame);
};

#endif // DISPLAY_H
