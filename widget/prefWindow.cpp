#include "widget.h"

int main()
{
    NSInitialize();

    NSHContainer speed(300, 50);
    NSLabel speedLabel("Speed");
    NSTextField speedVal(80);

    speed.add(&speedLabel);
    speed.add(&speedVal);
    speed.reallocate();

    NSHContainer toggles(300, 50);
    NSToggleButton anim("Anim"), query("Query"), round("Rounded");
    
    toggles.add(&anim);
    toggles.add(&query);
    toggles.add(&round);
    toggles.reallocate();

    NSHContainer reply(300, 100);
    NSButton ok("  OK  "), cancel("Cancel");
    
    reply.add(&ok);
    reply.add(&cancel);
    reply.reallocate();

    NSVContainer main(300, 200);
    
    main.add(&speed);
    main.add(&toggles);
    main.add(&reply);
    main.reallocate();

    NSFrame frame;
    
    frame.container(&main);

    frame.map();

    NSMainLoop();

    return 0;
}
