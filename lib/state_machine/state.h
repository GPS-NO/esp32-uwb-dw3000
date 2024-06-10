#ifndef STATE_H
#define STATE_H

class State {
public:
  virtual void onEnter() = 0;
  virtual void onUpdate() = 0;
  virtual void onExit() = 0;
  virtual ~State(){};
};

#endif