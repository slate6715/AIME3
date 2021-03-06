#ifndef ACTIONS_H
#define ACTIONS_H

class MUD;
class Action;

int infocom(MUD &engine, Action &act_used);
int gocom(MUD &engine, Action &act_used);
int lookcom(MUD &engine, Action &act_used);
int exitscom(MUD &engine, Action &act_used);
int getcom(MUD &engine, Action &act_used);
int putcom(MUD &engine, Action &act_used);
int dropcom(MUD &engine, Action &act_used);
int inventorycom(MUD &engine, Action &act_used);
int userscom(MUD &engine, Action &act_used);
int saycom(MUD &engine, Action &act_used);
int chatcom(MUD &engine, Action &act_used);
int tellcom(MUD &engine, Action &act_used);
int quitcom(MUD &engine, Action &act_used);
int opencom(MUD &engine, Action &act_used);
int closecom(MUD &engine, Action &act_used);
int statscom(MUD &engine, Action &act_used);
int equipcom(MUD &engine, Action &act_used);
int removecom(MUD &engine, Action &act_used);
int tiecom(MUD &engine, Action &act_used);
int untiecom(MUD &engine, Action &act_used);
int failcom(MUD &engine, Action &act_used);
int gotocom(MUD &engine, Action &act_used);
int killcom(MUD &engine, Action &act_used);
int lightcom(MUD &engine, Action &act_used);
int extinguishcom(MUD &engine, Action &act_used);
int summoncom(MUD &engine, Action &act_used);
int eatcom(MUD &engine, Action &act_used);
int hitcom(MUD &engine, Action &act_used);

#endif // ifndef ACTIONS
