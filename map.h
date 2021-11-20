#ifndef MAP_H
#define MAP_H

#include <vector>
#include <iostream>
#include <unordered_map>

#include "coord.h"
#include "combat.h"
#include "sched.h"

using namespace std;

class Tile {
	public:
		virtual bool passable() = 0;
		virtual char render() = 0;
};

class Wall {
	public:
		bool passable() { return false; }
		char render() { return '#'; }
};

class Floor {
	public:
		bool passable() { return true; }
		char render() { return '.'; }
};

class Actor;
class PolicyConsultWorld;
class ActorAction;

class World {
	public:
		using Grid = Grid<Tile>;
		using R = Grid::R;
		using V = Grid::V;
		using index = Grid::index;
		using posmap = unordered_map<index, unordered_map<index, Actor *>>;

	public:
		Grid grid;
		vector<Actor *> actors;
		R screen;
		V center;
		Sched sched;
		vector<Combat *> combats;
		unordered_map<Creature *, Actor *> actorOfCreature;
		PolicyConsultWorld policy;

		World(R scr, R gsz, V cen): screen(scr), grid(gsz), center(cen) {}
		World(R scr, R gsz): screen(scr), grid(gsz) {}

		void fill(Grid::R rect, Tile with);
		posmap actorPosMap();
		void draw(ostream &os);

		void add(Actor *a) { actors.push_back(a); }
		void tick();
		void schedule(ActorAction *act);
};

class Renderer {
	public:
		virtual char render(Actor *a) = 0;
};

class Actor {
	public:
		using Grid = World::Grid;
		using V = Grid::V;

	public:
		V pos;
		World *world;
		Renderer *renderer;

		Actor(World *w, V p, Renderer *r = NULL): world(w), pos(p), renderer(r) { world.add(this); }

		virtual char render() { return renderer ? renderer->render(this) : '!'; }
		virtual void tick() {}
		virtual void collide(Actor &other) {}
};

class StaticRenderer: public Renderer {
	public:
		char ch;

		StaticRenderer(char c): ch(c) {}

		char render(Actor *a) { return ch; }
};

class CreatureActor;

class ActorAction {
	public:
		virtual void act(Actor *performer, double time) = 0;
};

class ActorActionEvent: public Event {
	public:
		ActorAction *act;
		Actor *performer;
		double when;
		double delay;

		ActorActionEvent(ActorAction *a, Actor *p, double d): act(a), performer(p), delay(d) {}

		void initTimebase(double origin) { when = origin + delay; }
		double nextTime() { return when; }
		void perform(double time) { a->act(performer, time); }
};

class ActorPolicy {
	public:
		virtual ActorAction *select(Actor *performer) = 0;
};

enum class Motion {
	UP, UPRIGHT, RIGHT, DOWNRIGHT, DOWN, DOWNLEFT, LEFT, UPLEFT,
};

Grid::V motionOffset(Motion motion);

class CreatureActor: public Actor {
	public:
		using V = Actor::V;

	public:
		Creature creature;
		ActorPolicy *policy;

		CreatureActor(World *w, V p, Creature c, ActorPolicy *pol, Renderer *r = NULL);

		void tick();
};

class ItemActor: public Actor {
	public:
		using V = Actor::V;

	public:
		Item *item;

		ItemActor(World *w, V p, Item *i, Renderer *r = NULL): Actor(w, p, r), item(i) {}
};

class PolicyConsultWorld: public Policy

#endif
